import itertools
import logging
from collections import OrderedDict
from functools import reduce

import numpy as np

from config import cfg
from generic_op import ConvPoolOpBase, GenericConvertor
from logger import init_logger
from midap_simulator import midap_manager
from models.model_builder import ModelBuilder

from .layer_block import LayerBlock
from .midap_model import MidapModel
from .subnet_compiler import SubNetCompiler


class TileBuilder(object):
    @staticmethod
    def _make_conv(mb, layer, op, tensor):
        kernel = (op.k_w, op.k_h)
        pad    = (op.pad_t, op.pad_b, op.pad_l, op.pad_r)
        in_c   = layer.get_input_shape()[2]
        out_c  = layer.get_output_shape()[2]
        tensor = mb.Conv(tensor, in_c, out_c, kernel, op.stride, pad)
        return tensor, out_c

    @staticmethod
    def _make_pool(mb, layer, op, tensor):
        kernel = (op.k_w, op.k_h)
        pad    = (op.pad_t, op.pad_l)
        in_c   = layer.get_input_shape()[2]
        tensor = mb.Pool(tensor, op.type, kernel, op.stride, pad)
        return tensor, in_c

    @staticmethod
    def _make_model(mb):
        odict = mb.get_operator_dict()
        convertor = GenericConvertor()
        convertor.operator_dict = odict
        convertor.post_process()
        model = MidapModel()
        model.from_generic_op_dict(odict)
        return model

    @staticmethod
    def _build_layer(mb, tensors, layer):
        op = layer.main_op
        tensor = tensors[0]
        if op.type == 'Conv':
            tensor, in_c = TileBuilder._make_conv(mb, layer, op, tensor)
        elif 'Pool' in op.type:
            tensor, in_c = TileBuilder._make_pool(mb, layer, op, tensor)
        elif op.type == 'Sum':
            tensor = mb.Sum(*tensors)
        elif op.type == 'Concat':
            tensor = mb.Concat(tensors)
        else:
            raise NotImplementedError("Name: {} Type: {}".format(layer.name, op.type))
        return [tensor]

    @staticmethod
    def _add_input(mb, shapes):
        in_tensors = []
        for idx, shape in enumerate(shapes):
            shape = (1, shape[2], shape[1], shape[0])
            in_tensors.append(mb.set_input_tensor(name='input' + str(idx), tensor_shape=shape))
        return in_tensors

    @staticmethod
    def make_tile(layers, shapes):
        raise NotImplementedError


class BlockTileBuilder(TileBuilder):
    counter = itertools.count(1)

    @staticmethod
    def _add_block(mb, block, tensors=None):
        first_tensors = BlockTileBuilder._build_layer(mb, tensors, block.source)

        outputs = []
        for p in block.path_list:
            tensors = first_tensors
            for l in p:
                tensors = BlockTileBuilder._build_layer(mb, tensors, l)
            outputs.append(tensors[0])

        tensors = BlockTileBuilder._build_layer(mb, outputs, block.sink)

        return tensors

    @staticmethod
    def _build_model(layers, shapes):
        mb = ModelBuilder("Tile_{}".format(next(BlockTileBuilder.counter)))

        tensors = BlockTileBuilder._add_input(mb, shapes)

        for l in layers:
            if isinstance(l, LayerBlock):
                tensors = BlockTileBuilder._add_block(mb, l, tensors)
            else:
                raise NotImplementedError

        return BlockTileBuilder._make_model(mb)

    @staticmethod
    def make_tile(layers, shapes):
        model = BlockTileBuilder._build_model(layers, shapes)
        return BlockTile(model, shapes)


class Tile(object):
    logger = init_logger('Tile', logging.INFO)
    sim_dict = {}

    def __init__(self, model, in_shapes):
        self.model      = model
        self._in_shapes = in_shapes
        self._tile_size = in_shapes[0][0]
        self._tile_shape = np.append(in_shapes[0][:2], np.sum(in_shapes[:, 2]))

        self._dram_access       = None
        self._feature_per_layer = None

    def __del__(self):
        del self.model
        del self._tile_shape

    @property
    def tile_size(self):
        return self._tile_size

    @property
    def tile_shape(self):
        return self._tile_shape

    @property
    def dram_access(self):
        return self._dram_access

    @property
    def feature_per_layer(self):
        return self._feature_per_layer

    def calc_dram_access(self):
        raise NotImplementedError

    def calc_cycle(self):
        raise NotImplementedError

    def get_cycle_from_simulation(self):
        raise NotImplementedError

    class DRAM(object):
        @staticmethod
        def _calc_feature_dram_access(paths):
            feature_access_size = 0
            feature_per_layer = OrderedDict()
            Tile.logger.debug("[ Feature ]")
            for l in paths:
                input_feature_size = [feature_per_layer[prev][1] if prev.main_op.type != 'HEAD' else 0 for prev in l.input]
                feature_per_layer[l] = [input_feature_size[0]]

                out_shape     = l.get_output_shape()
                num_out_banks = len(l.control_info.output_mapping)
                if cfg.MODEL.TILING_METHOD == 'no' or paths[-1] != l:  # if no fusing, write output fragments to on-chip memory
                    reduced_width = l.num_planes_per_fmem * num_out_banks
                    feature_per_layer[l].append((max(out_shape[0] - reduced_width, 0)) * np.prod(out_shape[1:]))
                else:
                    feature_per_layer[l].append(out_shape[0] * np.prod(out_shape[1:]))
                feature_access_size += np.sum(feature_per_layer[l])

                Tile.logger.debug('{:9s} {:8d} {:8d} {:8d}'.format(l.name, *feature_per_layer[l], np.sum(feature_per_layer[l])))
            return feature_access_size, feature_per_layer

        @staticmethod
        def _calc_weight_dram_access(paths):
            weight_access_size = 0
            Tile.logger.debug("[ Weight ]")
            for l in paths:
                op = l.main_op
                if op.type == 'Conv':
                    process_num = reduce(lambda x, y: x + y, [0] + [1 if a[0] == 'PROCESS' else 0 for a in l.control_info.action])
                    weight_access_size += (op.orig_weight_size * process_num)
                    Tile.logger.debug('{:9s} {:3d} {:8d}'.format(l.name, process_num, (op.orig_weight_size * process_num)))
                elif op.type == 'Sum':
                    input_feature_size = [prev.get_output_size() for prev in l.input]
                    weight_access_size += sum(input_feature_size[1:])
                    Tile.logger.debug('{:9s} {:20s} {:8d}'.format(l.name, str(input_feature_size), sum(input_feature_size[1:])))
            return weight_access_size

        @staticmethod
        def _calc_model_dram_access(paths):
            feature_access_size, feature_per_layer = Tile.DRAM._calc_feature_dram_access(paths)
            weight_access_size = Tile.DRAM._calc_weight_dram_access(paths)
            return (feature_access_size, weight_access_size), feature_per_layer


class BlockTile(Tile):
    def _compile_and_get_path(self):
        subnet_compiler = SubNetCompiler()
        # subnet_compiler.force_setup(cfg.MIDAP.WMEM.NUM - 1)
        static_info = subnet_compiler.setup(self.model)
        inputs, paths = subnet_compiler.get_model_control_info()
        del subnet_compiler
        return inputs, paths, static_info

    def calc_dram_access(self):
        _, paths, _ = self._compile_and_get_path()
        self.logger.debug(self.tile_shape)
        self._dram_access, self._feature_per_layer = self.DRAM._calc_model_dram_access(paths[:-1])
        return self.dram_access

    def calc_cycle(self):
        delay, computation_cycle, _ = self.get_cycle_from_simulation()
        return (delay, computation_cycle)

    def id(self, paths):
        names = [(l.name, l.main_op.k_w, l.main_op.k_h, l.main_op.stride) if isinstance(l.main_op, ConvPoolOpBase) else l.name for l in paths]
        return tuple(names + self.tile_shape.tolist())

    def get_cycle_from_simulation(self):
        import midap_simulator.statistics as stats
        inputs, paths, static_info = self._compile_and_get_path()
        if self.id(paths) in BlockTile.sim_dict:
            return BlockTile.sim_dict[self.id(paths)]
        init_layer_list = self.model.init_layer
        simulator = midap_manager.MidapManager()
        _, cycle, dram = simulator.process_network_with_multiple_input(inputs, init_layer_list, paths[:-1])
        dram_delay = stats.get_dram_delay()
        stats.print_result(paths[:-1], static_info, self.tile_shape)
        # import sys
        # print("SHAPE", self.tile_shape, file=sys.stderr)
        BlockTile.sim_dict[self.id(paths)] = (dram_delay, cycle - dram_delay, dram)
        del simulator, inputs, paths, init_layer_list, static_info
        return (dram_delay, cycle - dram_delay, dram)
