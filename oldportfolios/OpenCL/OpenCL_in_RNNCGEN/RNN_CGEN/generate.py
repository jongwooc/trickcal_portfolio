#-*- coding: utf-8 -*-
from __future__ import print_function
import os
import shutil

import reshape
import fully
import conv
import pool

def make_main(problem, network, options):
    temp_f = open("./templates/main.tc", 'r')
    gen_f = open("./generated/main.c", 'w')

    with_bytes_data = 0
    with_images_data = 0

    forward = ""
    backprop = ""
    update = ""
    weight_pointers = ""
    weight_offset = 0
    name = ""
    layer_data_pointers = ""
    layer_delta_pointers = ""
    last_delta = ""
    last_layer_data = ""
    for layer in network["layers"]:
        prev_name = name
        name = layer.name
        last_delta = layer.name + "_delta"
        if isinstance(layer, fully.fully):
            in_size = layer.in_size
            out_size = layer.out_size

            if layer.prev_layer is None:
                forward = forward + name + "_forward(" + name + "_weight, " + name + \
                    "_bias, pfInput);\n        "
            else:
                forward = forward + name + "_forward(" + name + "_weight, " + name + \
                    "_bias, " + layer.prev_layer.name + "_data);\n        "
            if layer.prev_layer is None:
                backprop = name + \
                    "_backprop(" + name + \
                    "_weight, pfInput, NULL);\n        " + backprop
            else:
                backprop = name + "_backprop(" + name + "_weight, " + prev_name + \
                    "_data, " + prev_name + "_delta);\n        " + backprop
            update = update + name + \
                "_update(" + name + "_weight, " + name + "_bias);\n        "

            weight_pointers = weight_pointers + "float *" + \
                name + "_weight = weight + " + str(weight_offset) + ";\n    "
            weight_offset = weight_offset + in_size * out_size
            weight_pointers = weight_pointers + "float *" + \
                name + "_bias = weight + " + str(weight_offset) + ";\n    "
            weight_offset = weight_offset + out_size

            layer_data_pointers = layer_data_pointers + "float *" + \
                name + "_data = get_" + name + "_data();\n    "
            layer_delta_pointers = layer_delta_pointers + "float *" + \
                name + "_delta = get_" + name + "_delta();\n    "

        if isinstance(layer, conv.conv):
            kernel_size = layer.size

            out_width = layer.out_array[0]
            out_height = layer.out_array[1]
            out_chan = layer.out_array[2]

            if kernel_size == 3 and layer.wino == True:
                if layer.prev_layer is None:
                    forward = forward + name + "_wino_forward(" + name + "_weight, " + name + \
                        "_bias, pfInput);\n        "
                else:
                    forward = forward + name + "_wino_forward(" + name + "_weight, " + name + \
                        "_bias, " + layer.prev_layer.name + "_data);\n        "
                if prev_name == "":
                    backprop = name + \
                        "_wino_backprop(" + name + \
                        "_weight, pfInput, NULL);\n        " + backprop
                else:
                    backprop = name + "_wino_backprop(" + name + "_weight, " + prev_name + \
                        "_data, " + prev_name + "_delta);\n        " + backprop

            else:
                if layer.prev_layer is None:
                    forward = forward + name + "_forward(" + name + "_weight, " + name + \
                        "_bias, pfInput);\n        "
                else:
                    forward = forward + name + "_forward(" + name + "_weight, " + name + \
                        "_bias, " + layer.prev_layer.name + "_data);\n        "
                if layer.prev_layer is None:
                    backprop = name + \
                        "_backprop(" + name + \
                        "_weight, pfInput, NULL);\n        " + backprop
                else:
                    backprop = name + "_backprop(" + name + "_weight, " + prev_name + \
                        "_data, " + prev_name + "_delta);\n        " + backprop

            update = update + name + \
                "_update(" + name + "_weight, " + name + "_bias);\n        "

            weight_pointers = weight_pointers + "float *" + \
                name + "_weight = weight + " + str(weight_offset) + ";\n    "
            weight_offset = weight_offset + kernel_size * kernel_size * out_chan
            weight_pointers = weight_pointers + "float *" + \
                name + "_bias = weight + " + str(weight_offset) + ";\n    "
            weight_offset = weight_offset + out_chan

            layer_data_pointers = layer_data_pointers + "float *" + \
                name + "_data = get_" + name + "_data();\n    "
            layer_delta_pointers = layer_delta_pointers + "float *" + \
                name + "_delta = get_" + name + "_delta();\n    "

        if isinstance(layer, pool.pool):
            if layer.prev_layer is None:
                forward = forward + name + "_forward(pfInput);\n        "
            else:
                forward = forward + name + \
                    "_forward(" + layer.prev_layer.name + "_data);\n        "
            layer_data_pointers = layer_data_pointers + "float *" + \
                name + "_data = get_" + name + "_data();\n    "
            layer_delta_pointers = layer_delta_pointers + "float *" + \
                name + "_delta = get_" + name + "_delta();\n    "

            if layer.prev_layer is None:
                backprop = name + "_backprop(NULL);\n        " + backprop
            else:
                backprop = name + \
                    "_backprop(" + prev_name + "_delta);\n        " + backprop

        if isinstance(layer, reshape.reshape):
            prev_layer_datas = ""
            prev_layer_deltas = ""
            for i in range(len(layer.prev_layers)):
                if layer.prev_layers[i] == "dummy":
                    weight_pointers = "float dummy_data[" + str((layer.out_size - layer.in_size) * network[
                                                                "batch"]) + "] = {0.5, };\n  " + weight_pointers
                    prev_layer_datas = prev_layer_datas + name + \
                        "_input[" + str(i) + "] = " + "dummy_data;\n        "
                else:
                    prev_layer_datas = prev_layer_datas + name + \
                        "_input[" + str(i) + "] = " + \
                        layer.prev_layers[i].name + "_data;\n        "
                    prev_layer_deltas = prev_layer_deltas + name + \
                        "_input[" + str(i) + "] = " + \
                        layer.prev_layers[i].name + "_delta;\n        "

            forward = forward + prev_layer_datas
            forward = forward + name + \
                "_forward(" + name + "_input);\n        "
            layer_data_pointers = layer_data_pointers + "float **" + \
                name + "_input = get_" + name + "_input();\n    "
            layer_data_pointers = layer_data_pointers + "float *" + \
                name + "_data = get_" + name + "_data();\n    "
            layer_delta_pointers = layer_delta_pointers + "float *" + \
                name + "_delta = get_" + name + "_delta();\n    "

            if prev_name == "":
                raise Exception("Reshape layer need prev layer")
            else:
                backprop = name + \
                    "_backprop(" + name + "_input);\n        " + backprop
                backprop = prev_layer_deltas + backprop
        last_layer_data = name + "_data"

    # fixed point arithmetic option
    if (options.fixed_arith == True):
        define_fixed = "#define FIXED_POINT_ARITHMETIC"
    else:
        define_fixed = "//#define FIXED_POINT_ARITHMETIC"

    # which type of dataset will be used
    if (problem['dataset'] == "cifar" or problem['dataset'] == "mnist"):
        with_bytes_data = 1;
    if (problem['dataset'] == "VOC"):
        with_images_data = 1;

    while True:
        line = temp_f.readline()
        if not line:
            break

        line = line.replace("%(with_bytes_data)", str(with_bytes_data))
        line = line.replace("%(with_images_data)", str(with_images_data))
        
        line = line.replace("%(train_images_path)", problem['train_images'])
        
        line = line.replace("%(define_fixed)", define_fixed)
        line = line.replace("%(last_delta)", last_delta)
        line = line.replace("%(last_layer)", last_layer_data)
        line = line.replace("%(num_classes)", str(problem["num_classes"]))
        line = line.replace("%(weight_pointer)", weight_pointers)
        line = line.replace("%(layer_data_pointer)", layer_data_pointers)
        line = line.replace("%(layer_delta_pointer)", layer_delta_pointers)
        line = line.replace("%(train_iter)", str(network["iter"]))
        line = line.replace("%(test_iter)", str(network["test_iter"]))
        line = line.replace("%(display)", str(network["display"]))
        line = line.replace("%(input_size)", str(
            problem["width"] * problem["height"] * problem["channels"]))
        line = line.replace("%(height)", str(problem["height"]))
        line = line.replace("%(width)", str(problem["width"]))
        line = line.replace("%(forward)", forward)
        line = line.replace("%(backward)", backprop)
        line = line.replace("%(update)", update)
        line = line.replace("%(batch)", str(network["batch"]))

        gen_f.write(line)

    gen_f.close()
    temp_f.close()


def make_var(network):
    gen_header_f = open(
        "./generated/variable.h", 'w')
    temp_f = open("./templates/variable.th", 'r')
    while True:
        line = temp_f.readline()
        if not line:
            break
        line = line.replace("%(batch)", str(network["batch"]))
        line = line.replace("%(momentum)", str(network["momentum"]))
        line = line.replace("%(decay)", str(network["decay"]))
        line = line.replace("%(learning_rate)", str(network["learning_rate"]))
        line = line.replace("%(gamma)", str(network["gamma"]))
        line = line.replace("%(power)", str(network["power"]))
        gen_header_f.write(line)

    temp_f.close()
    gen_header_f.close()


def make_makefile():
    import glob
    #every generated files
    files = glob.glob("./generated/*.c")
    file_str = ""
    for i in range(len(files)):
        file_str = file_str + " " + files[i]
    #files from darknet
    files = glob.glob("./include/*.c")
    for i in range(len(files)):
        file_str = file_str + " " + files[i]
    gen_header_f = open("./makefile", 'w')
    temp_f = open("./templates/Makefile.t", 'r')
    while True:
        line = temp_f.readline()
        if not line:
            break

        line = line.replace("%(c_files)", file_str)
        gen_header_f.write(line)

    temp_f.close()
    gen_header_f.close()


def copy_c_files():
    import glob
    from shutil import copy

    for file in glob.glob(r'./templates/*.c'):
        copy(file, "./generated/")
    for file in glob.glob(r'./templates/*.h'):
        copy(file, "./generated/")

def make_gemm_header(network):
    # make header
    gen_header_f = open(
        "./generated/gemm.h", 'w')
    temp_f = open("./templates/gemm.th", 'r')
    funcs = ""

    for layer in network["layers"]:
        if isinstance(layer, conv.conv):
            funcs = funcs + layer.gemm_funcs
        if isinstance(layer, fully.fully):
            funcs = funcs + layer.gemm_funcs

    while True:
        line = temp_f.readline()
        if not line:
            break
        line = line.replace("%(dec_funcs)", funcs)
        gen_header_f.write(line)

def copy_required_files():
    from shutil import copy

    # copy h files from darknet
    copy("./include/image.h", "./generated/")
    copy("./include/utils.h", "./generated/")
    copy("./include/list.h", "./generated/")
    copy("./include/blas.h", "./generated/")
    copy("./include/box.h", "./generated/")
    copy("./include/darknet.h", "./generated/")
    copy("./include/stb_image.h", "./generated/")
    copy("./include/stb_image_write.h", "./generated/")

    # copy fixed_point.h
    copy("./include/fixed_point.h", "./generated/")

def make_c_file(problem, network, options):
    if (os.path.exists("./generated")):
        shutil.rmtree("./generated")
    os.mkdir("./generated", 0755)

    make_var(network)

    conv.make_conv(network)
    conv.make_gemm(network)

    pool.make_pool(network)

    fully.make_fully(network)
    fully.make_gemm(network)

    make_gemm_header(network)
    copy_required_files()

    reshape.make_reshape(network)
    make_main(problem, network, options)
    copy_c_files()

    make_makefile()
