import logging
from functools import reduce

import numpy as np

from config import cfg
from generic_op import ConcatOp, ConvPoolOpBase
from logger import init_logger

from .layer_block import BlockBuilder, LayerBlock
from .tile import BlockTileBuilder


class PartitionAlgo(object):
    def __init__(self):
        pass

    def partition(self, model):
        raise NotImplementedError


class SingleBlockPartition(PartitionAlgo):
    def __init__(self):
        self.block_organizer = BlockBuilder()

    def partition(self, model):
        input_blob  = model.init_layer
        input_layer = [model[x] for x in input_blob]

        path_list       = self.block_organizer.make_block_path(input_layer)
        tiling_pyramids = [BlockPyramid([v]) for v in path_list if isinstance(v, LayerBlock)]

        return tiling_pyramids


class MultiBlockPartition(PartitionAlgo):
    def __init__(self):
        self.block_organizer = BlockBuilder()

    def partition(self, model):
        raise NotImplementedError


class Pyramid(object):
    builder = None

    def __init__(self, layers):
        self.layers = layers
        self.name   = layers[0].name

        self.fmem_bank_size = cfg.MIDAP.FMEM.NUM_ENTRIES
        self.num_fmem       = cfg.MIDAP.FMEM.NUM

        source = self.layers[0].source if isinstance(self.layers[0], LayerBlock) else self.layers[0]
        shapes = self._shapes = np.array([l.get_output_shape() for l in source.input])
        self._in_chan = np.sum(shapes[:, 2]) if isinstance(source, ConcatOp) else shapes[0][2]
        self._orig_size = shapes[0][0]
        self._overhead = self._calc_overhead()

        self.logger = init_logger(self.name, logging.INFO)

    @property
    def in_chan(self):
        return self._in_chan

    @property
    def orig_size(self):
        return self._orig_size

    @property
    def overhead(self):
        return self._overhead

    @property
    def tiles(self):
        return self._tiles

    def _make_tile(self, tiles):
        shapes = self._shapes

        tile_size    = tiles[0][1] - tiles[0][0]
        shapes[:, 0] = tile_size
        general_tile = self.builder.make_tile(self.layers, shapes)

        rest_tile = None
        if tile_size != (tiles[-1][1] - tiles[-1][0]):
            shapes[:, 0] = (tiles[-1][1] - tiles[-1][0])
            rest_tile = self.builder.make_tile(self.layers, shapes)

        return general_tile, rest_tile

    # TODO make one function and take cycle and dram as arguments
    def _get_dram_access_each_tile(self):
        self.logger.debug("[ General Case ({}) ]".format(self.tiles[0].tile_size))
        general_dram = self.tiles[0].calc_dram_access()
        if self.tiles[1]:
            self.logger.debug("[ Rest Case ({}) ]".format(self.tiles[1].tile_size))
        rest_dram = self.tiles[1].calc_dram_access() if self.tiles[1] else 0
        return general_dram, rest_dram

    def calc_dram_access(self, tile_size):
        tiles       = self._make_tile_fragments(tile_size)
        self._tiles = self._make_tile(tiles)
        dram_access = self._get_dram_access_each_tile()

        return self._calc_dram_access_in_pyramid(tiles, dram_access)

    def _calc_dram_access_in_pyramid(self, tiles, dram_access):
        in_shape        = np.append(self._shapes[0][:2], self.in_chan)
        in_feature_dram = reduce(lambda x, y: x + np.prod(in_shape[1:]) * (y[1] - y[0]), [0] + tiles)
        # XXX all banks except one bank to save output
        feature_dram    = max(in_feature_dram - ((self.fmem_bank_size * (self.num_fmem - 1)) if len(tiles) == 1 else 0), 0)
        weight_dram     = 0

        tile_size = tiles[0][1] - tiles[0][0]
        for t in tiles:
            tile_w = t[1] - t[0]
            feature_dram += (dram_access[0][0] if tile_w == tile_size else dram_access[1][0])
            weight_dram  += (dram_access[0][1] if tile_w == tile_size else dram_access[1][1])
        return feature_dram, weight_dram

    # TODO make one function and take cycle and dram as arguments
    def _get_cycle_each_tile(self):
        self.logger.debug("[ General Case ({}) ]".format(self.tiles[0].tile_size))
        general_cycle = self.tiles[0].calc_cycle()
        if self.tiles[1]:
            self.logger.debug("[ Rest Case ({}) ]".format(self.tiles[1].tile_size))
        rest_cycle = self.tiles[1].calc_cycle() if self.tiles[1] else 0
        return general_cycle, rest_cycle

    def calc_cycle(self, tile_size):
        tiles       = self._make_tile_fragments(tile_size)
        self._tiles = self._make_tile(tiles)
        cycles      = self._get_cycle_each_tile()

        return self._calc_cycle_in_pyramid(tiles, cycles)

    def _calc_cycle_in_pyramid(self, tiles, cycles):
        dram_delay = computation_cycle = 0
        tile_size = tiles[0][1] - tiles[0][0]
        for t in tiles:
            tile_w = t[1] - t[0]
            dram_delay += (cycles[0][0] if tile_w == tile_size else cycles[1][0])
            computation_cycle  += (cycles[0][1] if tile_w == tile_size else cycles[1][1])
        return dram_delay, computation_cycle

    def _make_tile_fragments(self, tile_size):
        # TODO this code can be make fast by calculating statically.
        tiles = [(0, tile_size)]
        while tiles[-1][1] < self.orig_size:
            prev_w = tiles[-1][1]
            start  = prev_w - self.overhead
            tiles.append((start, min(start + tile_size, self.orig_size)))
        return tiles

    def _simulate_each_tile(self):
        self.logger.debug("[ General Case ({}) ]".format(self.tiles[0].tile_size))
        general_delay, general_cycle, general_dram = self.tiles[0].get_cycle_from_simulation()
        if self.tiles[1]:
            self.logger.debug("[ Rest Case ({}) ]".format(self.tiles[1].tile_size))
        rest_delay, rest_cycle, rest_dram = self.tiles[1].get_cycle_from_simulation() if self.tiles[1] else (0, 0, 0)
        return (general_delay, rest_delay), (general_cycle, rest_cycle), (general_dram, rest_dram)

    def _calc_all_simulation(self, tiles, delays, cycles, dram):
        tile_size = tiles[0][1] - tiles[0][0]
        tot_delay = tot_cycle = tot_dram = 0
        for t in tiles:
            tile_w = t[1] - t[0]
            tot_delay += (delays[0] if tile_w == tile_size else delays[1])
            tot_cycle += (cycles[0] if tile_w == tile_size else cycles[1])
            tot_dram  += (dram[0] if tile_w == tile_size else dram[1])
        return tot_delay, tot_cycle, tot_dram

    def simulate(self, tile_size):
        tiles       = self._make_tile_fragments(tile_size)
        self._tiles = self._make_tile(tiles)
        results     = self._simulate_each_tile()
        del self._tiles
        return self._calc_all_simulation(tiles, *results)


class BlockPyramid(Pyramid):
    builder = BlockTileBuilder

    @staticmethod
    def _calc_layer_overhead(layer):
        op = layer.main_op
        overhead = 0
        if isinstance(op, ConvPoolOpBase) and op.k_w > 1:
            overhead = op.k_w - op.stride
        return overhead

    def _calc_overhead(self):
        block  = self.layers[0]
        source = block.source

        max_overhead = 0
        init_overhead = BlockPyramid._calc_layer_overhead(source)

        for p in block.path_list:
            overhead = init_overhead
            for l in p:
                overhead += BlockPyramid._calc_layer_overhead(l)
            max_overhead = max(overhead, max_overhead)
        return max_overhead
