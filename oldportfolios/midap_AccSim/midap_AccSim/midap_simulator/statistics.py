from __future__ import absolute_import, division, print_function, unicode_literals

import time

from acc_utils.attrdict import AttrDict
from acc_utils.errors import *
from acc_utils.model_utils import *
from config import cfg
from generic_op import *

__S = None


def create_branch():
    branch = AttrDict()
    branch.SIM_TIME = time.time()
    branch.CLOCK = 0
    branch.WRITE = AttrDict()
    branch.WRITE.DRAM = 0
    branch.WRITE.FMEM = 0
    branch.WRITE.WMEM = 0
    branch.RUN = AttrDict()
    branch.RUN.REGISTER_RESET = 0  # Write 0 cnt
    branch.RUN.REGISTER_SHIFT = 0  # Count
    branch.RUN.ALU = 0  # Activation is skipped
    branch.READ = AttrDict()
    branch.READ.DRAM = 0
    branch.READ.FMEM = 0
    branch.READ.WMEM = 0
    branch.MACs = 0
    branch.MEM_LATENCY = 0
    branch.WRITE_LATENCY = 0
    branch.WMEM_READ_DELAY = 0
    branch.FMEM_READ_DELAY = 0
    branch.FMEM_WRITE_DELAY = 0
    return branch


__DESCRIPTION = create_branch()

__DESCRIPTION.SIM_TIME = "Elapsed time"
__DESCRIPTION.CLOCK = "Simulated Cycle"
__DESCRIPTION.WRITE.DRAM = "# of DRAM write"
__DESCRIPTION.WRITE.FMEM = "# of FMEM write"
__DESCRIPTION.WRITE.WMEM = "# of WMEM write"
__DESCRIPTION.RUN.REGISTER_RESET = "# of reset register operations"
__DESCRIPTION.RUN.REGISTER_SHIFT = "# of shift register operations"
__DESCRIPTION.RUN.ALU = "# of Multiplications at MIDAP alu"
__DESCRIPTION.READ.DRAM = "# of DRAM read"
__DESCRIPTION.READ.FMEM = "# of FMEM read"
__DESCRIPTION.READ.WMEM = "# of WMEM read"
__DESCRIPTION.MACs = "MACs"
__DESCRIPTION.MEM_LATENCY = "LATENCY due to the memory delay"
__DESCRIPTION.WRITE_LATENCY = "LATENCY due to the write operation bottleneck"


def stats_str(name, op_type, stats, write=False, run=False, read=True, etc=False):
    default_str = "{}\t{}\t{}\t{}".format(
        name, op_type, stats.CLOCK, stats.MACs)
    read_str = "\t{}\t{}\t{}".format(
        stats.READ.DRAM, stats.READ.FMEM, stats.READ.WMEM)
    write_str = "\t{}\t{}\t{}".format(
        stats.WRITE.DRAM, stats.WRITE.FMEM, stats.WRITE.WMEM)
    run_str = "{}\t{}\t{}".format(
        stats.RUN.REGISTER_RESET, stats.RUN.REGISTER_SHIFT, stats.RUN.ALU)
    etc_str = "{}\t{}\t{}".format(
        stats.MEM_LATENCY, stats.WRITE_LATENCY, stats.SIM_TIME)
    print_str = default_str
    if read:
        print_str += read_str
    if write:
        print_str += write_str
    if run:
        print_str += run_str
    if etc:
        print_str += etc_str
    print_str += "\n"
    return print_str


def description(*args, **kwargs):
    global __DESCRIPTION
    return stats_str("LAYER", "TYPE", __DESCRIPTION, *args, **kwargs)


def init():
    global __S
    if __S is not None:
        del __S

    __S = AttrDict()
    __S.GLOBAL = create_branch()
    __S.LOCAL = create_branch()
    __S.LAYERS = dict()
    return __S


def update(layer_name, print_stats=True):
    global __S
    for attr in __S.GLOBAL:
        if attr == 'SIM_TIME':
            continue
        if isinstance(__S.GLOBAL[attr], dict):
            for key in __S.GLOBAL[attr]:
                __S.GLOBAL[attr][key] += __S.LOCAL[attr][key]
        else:
            __S.GLOBAL[attr] += __S.LOCAL[attr]

    __S.LOCAL.SIM_TIME = time.time() - __S.LOCAL.SIM_TIME
    ## PRINT STATS ##
    stat = __S.LOCAL
    if print_stats:
        print(" LAYER {} - Elapsed Time(s): {}, Simulated cycles: {}, MACs: {}.".format(
            layer_name,
            stat.SIM_TIME,
            stat.CLOCK,
            stat.MACs)
        )

    __S.LAYERS[layer_name] = __S.LOCAL
    __S.LOCAL = create_branch()


def end_simulation():
    global __S
    __S.GLOBAL.SIM_TIME = time.time() - __S.GLOBAL.SIM_TIME
    stat = __S.GLOBAL
    print(""" SIMULATION FINISHED. SUMMARY:
            SIMULATION TIME(s): {}
            SIMULATED CYCLE(cylce): {}
            ALU WORKS: {}
            MACs: {}
            DRAM_READ, WRITE: {}, {}
            FMEM_READ, WRITE: {}, {}
            WMEM_READ, WRITE: {}, {}
            """.format(stat.SIM_TIME,
                       stat.CLOCK,
                       stat.RUN.ALU,
                       stat.MACs,
                       stat.READ.DRAM, stat.WRITE.DRAM,
                       stat.READ.FMEM, stat.WRITE.FMEM,
                       stat.READ.WMEM, stat.WRITE.WMEM)
          )


def dram_read(cnt=1):
    global __S
    __S.LOCAL.READ.DRAM += cnt


def fmem_read(cnt=1):
    global __S
    __S.LOCAL.READ.FMEM += cnt


def wmem_read(cnt=1):
    global __S
    __S.LOCAL.READ.WMEM += cnt


def run_register_shift(cnt=1):
    global __S
    __S.LOCAL.RUN.REGISTER_SHIFT += cnt


def run_register_reset(cnt):
    global __S
    __S.LOCAL.RUN.REGISTER_RESET += cnt


def run_alu(cnt):
    global __S
    __S.LOCAL.RUN.ALU += cnt


def dram_write(cnt=1):
    global __S
    __S.LOCAL.WRITE.DRAM += cnt


def fmem_write(cnt=1):
    global __S
    __S.LOCAL.WRITE.FMEM += cnt


def wmem_write(cnt=1):
    global __S
    __S.LOCAL.WRITE.WMEM += cnt


def total_cycle():
    global __S
    return __S.LOCAL.CLOCK + __S.GLOBAL.CLOCK


def current_cycle():
    global __S
    return __S.LOCAL.CLOCK


def increase_cycle(t=1):
    global __S
    __S.LOCAL.CLOCK += t


def memory_latency():
    global __S
    return __S.LOCAL.MEM_LATENCY


def wait_memory(t=1):
    global __S
    __S.LOCAL.MEM_LATENCY += t
    __S.LOCAL.CLOCK += t


def wait_dram2fmem(t=1):
    global __S
    __S.LOCAL.FMEM_READ_DELAY += t


def wait_dram2wmem(t=1):
    global __S
    __S.LOCAL.WMEM_READ_DELAY += t


def wait_fmem2dram(t=1):
    global __S
    __S.LOCAL.FMEM_WRITE_DELAY += t


def wait_writing(t=1):
    global __S
    __S.LOCAL.WRITE_LATENCY += t
    __S.LOCAL.CLOCK += t


def set_macs(size):
    global __S
    __S.LOCAL.MACs = size


def print_simulate_result(result_file, path_info, read=True, write=False, run=False, etc=False):
    global __S
    statistics = __S
    stats_kwargs = {
        "read": read,
        "write": write,
        "run": run,
        "etc": etc
    }
    with open(result_file, 'w') as f:
        whole_stats = statistics.GLOBAL
        f.write(description(**stats_kwargs))
        for layer in path_info:
            if layer.name in statistics.LAYERS:
                f.write(stats_str(layer.name, layer.main_op.type,
                                  statistics.LAYERS[layer.name], **stats_kwargs))
        f.write(stats_str("TOTAL", "-", whole_stats, **stats_kwargs))


def diff_static_and_simulate(path_info, static_info):
    global __S
    statistics = __S
    for layer in path_info:
        if isinstance(layer.main_op, ConvOp) or isinstance(layer.main_op, PoolOp):
            stats_layer = statistics.LAYERS[layer.name]
            profile_value = stats_layer['CLOCK'] - stats_layer['MEM_LATENCY']
            static_cycle = static_info[layer.name]['cycle']
            static_in_size = static_info[layer.name]['in_size']
            static_out_size = static_info[layer.name]['out_size']
            print("Layer: {:>16s} {:^12s}\tKern: {:1d}x{:1d}\tSimulated: {:>8d}\tStatic Calc: {:>8d}\tDiff. Rate: {:>.2f}\tDRAM: {:>8d}\tFMEM: {:>8d} ({:>5d}, {:>5d})\tTensor: {:>8d} -> {:>8d}".format(
                layer.name, "(" + layer.main_op.type +
                ")", layer.main_op.k_w, layer.main_op.k_h, profile_value, static_cycle,
                (static_cycle - profile_value) / profile_value * 100, stats_layer['MEM_LATENCY'], stats_layer['WRITE']['FMEM'], stats_layer['FMEM_READ_DELAY'], stats_layer['FMEM_WRITE_DELAY'], static_in_size, static_out_size))
        elif layer.name in statistics.LAYERS:
            stats_layer = statistics.LAYERS[layer.name]
            print("Layer: {:>16s} {:^12s}\tDRAM: {:>8d}\tFMEM: {:>8d} ({:>5d}, {:>5d})".format(
                layer.name, "(" + layer.main_op.type + ")", stats_layer['MEM_LATENCY'], stats_layer['WRITE']['FMEM'], stats_layer['FMEM_READ_DELAY'], stats_layer['FMEM_WRITE_DELAY']))


def get_dram_delay():
    global __S
    return __S.GLOBAL.MEM_LATENCY


def print_result(path_info, static_info, model):
    import math
    global __S
    statistics = __S

    name = []
    dram = []
    fmem_dram = []
    wmem_dram = []
    total_delay = []
    fmem_delay = []
    wmem_delay = []
    cycle = []
    mac = []
    fps = []
    utilization = []
    dram_is = []
    dram_ws = []

    # for conv
    conv_clock = 0
    conv_mac = 0
    conv_delay = 0

    # for fc
    fc_delay = 0

    # for residual
    residual_delay = 0

    for layer in path_info:
        if isinstance(layer.main_op, ConcatOp) or layer.reduction:
            continue
        stats_layer = statistics.LAYERS[layer.name]
        input_size = layer.get_input_size()
        weight_size = 0
        name.append(layer.name)
        dram.append(stats_layer['READ']['DRAM'] + stats_layer['WRITE']['DRAM'])
        wmem_dram.append(stats_layer['WRITE']['WMEM'] *
                         max(cfg.SYSTEM.BANDWIDTH, cfg.MIDAP.SYSTEM_WIDTH))
        fmem_dram.append(dram[-1] - wmem_dram[-1])

        total_delay.append(stats_layer['MEM_LATENCY'])
        fmem_delay.append(stats_layer['FMEM_READ_DELAY'])
        wmem_delay.append(stats_layer['WMEM_READ_DELAY'])

        mac.append(stats_layer['MACs'])
        cycle.append(stats_layer['CLOCK'])
        fps.append(1.0e9 / stats_layer['CLOCK'])
        utilization.append(stats_layer['MACs'] / (stats_layer['CLOCK'] * 1024))
        if isinstance(layer.main_op, ConvOp) and layer.main_op.type != 'FC':
            conv_clock += stats_layer['CLOCK']
            conv_mac += stats_layer['MACs']
            conv_delay += stats_layer['WMEM_READ_DELAY']
            weight_size = layer.main_op.orig_weight_size
        elif isinstance(layer.main_op, ConvOp) and layer.main_op.type == 'FC':
            fc_delay += stats_layer['WMEM_READ_DELAY']
            weight_size = layer.main_op.orig_weight_size
        elif isinstance(layer.main_op, ArithmeticOp):
            residual_delay += stats_layer['WMEM_READ_DELAY']
        dram_is.append(input_size + int(math.ceil(input_size / (cfg.MIDAP.FMEM.NUM_ENTRIES *
                                                                cfg.MIDAP.FMEM.NUM * cfg.SYSTEM.DATA_SIZE))) * weight_size)
        dram_ws.append(weight_size + max(int(math.ceil(weight_size / (cfg.MIDAP.WMEM.NUM_ENTRIES *
                                                                      cfg.MIDAP.WMEM.NUM * cfg.SYSTEM.DATA_SIZE))), 1) * input_size)

    print("{}\tDRAM_Access\tDRAM_Delay\tDRAM_Access(FMEM)\tDRAM_Access(WMEM)\tDRAM_Delay(FMEM)\t\
        DRAM_Delay(WMEM)\tMACs\tCYCLE\tFPS\tUtilization\tDRAM_Access(IS)\tDRAM_Access(WS)\tUtil.(Conv)\tResidual_Delay(WMEM)\t\
        FC_Delay(WMEM)\tConv_Delay(WMEM)\tDRAM_Dealy_Ratio\tDRAM_Delay_Ratio(FMEM)\tDRAM_Delay_Ratio(WMEM)\tDRAM_Delay_Ratio(WMEM, Conv)".format(model))
    for v in zip(name, dram, total_delay, fmem_dram, wmem_dram, fmem_delay, wmem_delay, mac, cycle, fps, utilization, dram_is, dram_ws):
        print("{}\t{:,}\t{:,}\t{:,}\t{:,}\t{:,}\t{:,}\t{:,}\t{:,}\t{:.0f}\t{:.4f}\t{}\t{}".format(*v))

    # dram, fmem_dram, wmem_dram, total_delay, fmem_delay, wmem_delay, mac, cycle, fps, utilization
    # conv util, residual delay, fc delay, conv delay, dram delay ratio, fmem ratio, wmem ratio, wmem ratio (only conv)
    global_stat = __S.GLOBAL
    print("Total\t{:,}\t{:,}\t{:,}\t{:,}\t{:,}\t{:,}\t{:,}\t{:,}\t\
            {:.0f}\t{:.4f}\t{}\t{}\t{:.4f}\t{}\t{}\t\
            {}\t{:.4f}\t{:.4f}\t{:.4f}\t{:.4f}".format(global_stat.READ.DRAM + global_stat.WRITE.DRAM,
                                                       global_stat.MEM_LATENCY,
                                                       global_stat.READ.DRAM + global_stat.WRITE.DRAM - global_stat.WRITE.WMEM *
                                                       max(cfg.SYSTEM.BANDWIDTH,
                                                           cfg.MIDAP.SYSTEM_WIDTH),
                                                       global_stat.WRITE.WMEM *
                                                       max(cfg.SYSTEM.BANDWIDTH,
                                                           cfg.MIDAP.SYSTEM_WIDTH),
                                                       global_stat.FMEM_READ_DELAY, global_stat.WMEM_READ_DELAY,
                                                       global_stat.MACs, global_stat.CLOCK, 1.0e9 / global_stat.CLOCK,
                                                       global_stat.MACs /
                                                       (global_stat.CLOCK * 1024),
                                                       sum(dram_is), sum(
                                                           dram_ws),
                                                       conv_mac /
                                                       (conv_clock * 1024),
                                                       residual_delay, fc_delay, conv_delay,
                                                       global_stat.MEM_LATENCY / global_stat.CLOCK,
                                                       global_stat.FMEM_READ_DELAY / global_stat.CLOCK,
                                                       global_stat.WMEM_READ_DELAY / global_stat.CLOCK,
                                                       conv_delay / global_stat.CLOCK))
