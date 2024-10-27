import math
import sys

import numpy as np

from config import cfg

from .layer_block import LayerBlock
from .midap_layer import MidapLayer


class Objective(object):
    def __init__(self, pyramid, weight=1):  # if weight is negative value, then maximize.
        if weight != 1 and weight != -1:
            raise ValueError

        self._min_value = weight * sys.maxsize
        if cfg.MODEL.TILING_OBJECTIVE == "dram_access":
            self._fit_function = pyramid.calc_dram_access
        elif cfg.MODEL.TILING_OBJECTIVE == "cycle":
            self._fit_function = pyramid.calc_cycle
        else:
            raise ValueError

        self._tile_size  = 0
        self._breakdown  = None

    def compare_and_set(self, pyramid, tile_size):
        total_dram_access = self._fit_function(tile_size)
        if self._min_value >= np.sum(total_dram_access):
            self._min_value = np.sum(total_dram_access)
            self._breakdown = total_dram_access
            self._tile_size = tile_size

    def get_objectives(self):
        return self._tile_size, self._breakdown


class TileSearchAlgo(object):
    @staticmethod
    def _calc_tile_size(width):
        raise NotImplementedError

    @classmethod
    def search(cls, pyramid):
        objective = Objective(pyramid)
        x_orig_size = pyramid.orig_size
        for tile_size in cls._calc_tile_size(x_orig_size):
            objective.compare_and_set(pyramid, tile_size)

        return objective.get_objectives()


class ManualAlgo(TileSearchAlgo):
    idx = 0
    tile_size = [90, 128, 128, 13, 64, 64, 64, 13, 32, 32, 32, 32, 32, 9, 16, 16]

    @classmethod
    def search(cls, pyramid):
        objective = Objective(pyramid)
        objective.compare_and_set(pyramid, cls.tile_size[cls.idx])
        cls.idx += 1

        return objective.get_objectives()


class ExponentialSearchAlgo(TileSearchAlgo):
    @staticmethod
    def _calc_tile_size(width):
        max_exp = int(math.log(width, 2))
        for exp in range(2, max_exp):
            tile_size = min(2 ** exp, width)
            yield tile_size


class AllSearchAlgo(TileSearchAlgo):
    @staticmethod
    def _calc_tile_size(width):
        for w in range(4, width + 1):
            yield w


class NoTilingAlgo(TileSearchAlgo):
    @staticmethod
    def _calc_tile_size(width):
        for w in range(width, width + 1):
            yield w


class ZeroAccessInMidLayerAlgo(AllSearchAlgo):
    @classmethod
    def _calc_inner_pyramid_dram_access(cls, pyramid):
        layers = []
        for v in pyramid.layers:
            if isinstance(v, MidapLayer):
                layers.append(v)
            elif isinstance(v, LayerBlock):
                layers.extend(v.path_list)

        in_feature = np.array([v[0] for v in pyramid.tiles[0].feature_per_layer.values()])
        return np.sum(in_feature)

    @classmethod
    def search(cls, pyramid):
        dram_access = sys.maxsize
        min_tile_size = -1
        x_orig_size = pyramid.orig_size
        for tile_size in cls._calc_tile_size(x_orig_size):
            total_dram_access = pyramid.calc_dram_access(tile_size)

            if 0 == cls._calc_inner_pyramid_dram_access(pyramid)\
                    and dram_access >= np.sum(total_dram_access):
                dram_access = np.sum(total_dram_access)
                dram_breakdown = total_dram_access
                min_tile_size = tile_size

        return min_tile_size, dram_breakdown
