#-*- coding: utf-8 -*-
from __future__ import print_function
from optparse import OptionParser
import sys

import parser
import generate
import fully
import conv
import pool
import reshape


def parse_opt():
    usage = """usage: %prog [options]"""
    parser = OptionParser(usage=usage)
    parser.add_option("-p", "--problem", dest="problem",
                      help="Problem Configuration Path", metavar="FILE")
    parser.add_option("-n", "--network", dest="network",
                      help="Network Configuration Path", metavar="FILE")
    parser.add_option("-c", "--cached", action="store_true", dest="cached", default=False,
                      help="Use Cached Dataset(default = False)")
    parser.add_option("-w", "--winograd", action="store_true", dest="winograd", default=False,
                      help="Use Winograd Convolution in 3x3 conv layers (default = False)")
    parser.add_option("-f", "--fixed_arith", action="store_true", dest="fixed_arith", default=False,
                      help="Use fixed point arithemic (default = False)")

    (options, args) = parser.parse_args()

    if options.problem == None:
        parser.print_help()
        sys.exit(2)

    if options.network == None:
        parser.print_help()
        sys.exit(2)

    return options, args

def make_weight(network):
    import random
    import math
    import array

    f = open("weight.byte", 'w')

    weight_array = array.array('f')
    for layer in network["layers"]:
        if isinstance(layer, fully.fully):
            in_size = layer.in_size
            out_channel = layer.out_array[2]
            out_size = layer.out_size

            # Xavier initializer
            scale = math.sqrt(2. / (in_size + out_size))
            for _ in range(in_size * out_size):
                weight = scale * random.uniform(-1, 1)
                weight_array.append(weight)

            for _ in range(out_channel):
                weight_array.append(0)
        if isinstance(layer, conv.conv):
            kernel_size = layer.size
            in_channel = layer.in_array[2]
            out_channel = layer.out_array[2]

            # Xavier initializer
            scale = math.sqrt(
                2. / (kernel_size * kernel_size * (in_channel + out_channel)))
            for _ in range((kernel_size * kernel_size * in_channel * out_channel)):
                weight = scale * random.uniform(-1, 1)
                weight_array.append(weight)

            for _ in range(out_channel):
                weight_array.append(0)
    weight_array.tofile(f)

    f.close()


# FIXME : Temporally Read data from argument
if __name__ == '__main__':
    options, args = parse_opt()
    problem = parser.read_problem(options)
    network = parser.read_network(options, problem)
    if (options.cached == False):
        if (problem['dataset'] == "mnist"):
            parser.read_mnist(problem)
        elif (problem['dataset'] == "cifar"):
            parser.read_cifar(problem)

    make_weight(network)
    generate.make_c_file(problem, network, options)
