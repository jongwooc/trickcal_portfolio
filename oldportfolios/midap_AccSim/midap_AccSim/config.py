from __future__ import absolute_import, division, print_function, unicode_literals

from acc_utils.attrdict import AttrDict

__C = AttrDict()

cfg = __C

__C.FUNCTIONAL_SIMULATION = True

__C.SYSTEM = AttrDict()

__C.ACTIVATE_DEBUG_MODE = False

__C.LOGGER = "MIDAP Logger"

# SYSTEM CONFIG : BANDWIDTH, DATASIZE(VIRTUAL), NETWORK

__C.SYSTEM.BANDWIDTH = 32  # GB ( * 10^9 byte) / s
__C.SYSTEM.DATA_SIZE = 1  # byte

__C.SYSTEM.ROOT = "/home/kangdongh/workspace/SmartCampus/AccSim"
__C.SYSTEM.NETWORK = "examples/inceptionV3/predict_net.pb"
__C.SYSTEM.WEIGHTS = "examples/inceptionV3/init_net.pb"
__C.SYSTEM.INPUT = "imgs/dog.jpg"
__C.SYSTEM.INPUT_SHAPE = (299, 299, 3)
__C.SYSTEM.FREQUENCY = 1000  # MHZ

# FIXME __C.SYSTEM.DRAM.PAGE_SIZE? other all DRAM related configurations..
__C.SYSTEM.DRAM_PAGE_SIZE = 4 * 1024  # byte

# LATENCY CONFIG : HOW MUCH CYCLE SPENT IN ..

__C.LATENCY = AttrDict()

__C.LATENCY.DRAM_READ = 100
__C.LATENCY.INCLUDE_DRAM_WRITE = True
__C.LATENCY.FMEM_RESET = 1

# ENERGY CONFIG

__C.ENERGY = AttrDict()

__C.ENERGY.DRAM_READ = 100
__C.ENERGY.DRAM_WRITE = 100

__C.ENERGY.SRAM_READ = 4
__C.ENERGY.SRAM_WRITE = 4

__C.ENERGY.REGISTER_READ = 1
__C.ENERGY.REGISTER_WRITE = 1


# TARGET ACCELERLATOR CONFIG
# MIDAP

__C.MODEL = AttrDict()

__C.MODEL.REDUCTION_LOGIC = True

__C.MODEL.USE_TILING = False
__C.MODEL.TILING_METHOD = None
__C.MODEL.TILING_OBJECTIVE = None

__C.MODEL.ALLOW_ABSTRACT_DATA = True
# True -> Memory mapping: to be supported in v1.3.0
# False -> Post module(subop): v1.2.0


__C.MIDAP = AttrDict()

__C.MIDAP.EFFICENT_LOGIC = True
# Skip ineffective computation without delay

__C.MIDAP.SYSTEM_WIDTH = 64

__C.MIDAP.FMEM = AttrDict()
__C.MIDAP.FMEM.NUM_ENTRIES = 256 * 1024  # # of Entries , PER ONE BANK
__C.MIDAP.FMEM.NUM = 4
__C.MIDAP.REDUCTION = AttrDict()
__C.MIDAP.REDUCTION.NUM_ENTRIES = 2048
# __C.MIDAP.FMEM.ALIGNMENT = 4 # Due to the Shift-Register limitation - maximum division - ex: 64, 4 --> 64 // 4 = 16 is a alignment channel size

__C.MIDAP.WMEM = AttrDict()
__C.MIDAP.WMEM.NUM_ENTRIES = 32 * 1024  # # of Entries , PER ONE BANK
__C.MIDAP.WMEM.NUM = 16
__C.MIDAP.BMMEM = AttrDict()
__C.MIDAP.BMMEM.NUM_ENTRIES = 4 * 1024  # Entries
__C.MIDAP.BMMEM.META_OFFSET = 2048
# MIDAP CONTROL SEQUENCE GENERATOR STRATEGY
# It is a scheduling(mapping) problem!!
# There must be an optimal strategy, but it is hard to find without complex algorithm
# e. g. GA, ILP ....

__C.MIDAP.CONTROL_STRATEGY = AttrDict()

__C.MIDAP.CONTROL_STRATEGY.FIRST_LAYER = 'GEMM'

__C.MIDAP.CONTROL_STRATEGY.FMEM = 'INPUT_STATIONARY'
# INPUT_STATIONARY, OUTPUT_STATIONARY, GREEDY(will be supported)

# __C.MIDAP.CONTROL_STRATEGY.FILTER_LOAD = 'MAXIMUM'

__C.MIDAP.CONTROL_STRATEGY.FILTER_LOAD = 'ONE'
# ONE, MAXIMUM, COMPILER_DRIVEN

__C.MIDAP.CONTROL_STRATEGY.LAYER_COMPILER = 'MIN_DRAM_ACCESS'
# __C.MIDAP.CONTROL_STRATEGY.LAYER_COMPILER = 'HIDE_DRAM_LATENCY'

# __C.MIDAP.BUS_POLICY = 'FIFO'
__C.MIDAP.BUS_POLICY = 'WMEM_FIRST'

# TODO: OTHER MIDAP CONFIGURATION OPTIONS WILL BE ADDED
