#-*- coding: utf-8 -*-
from parse import *

import fully
import conv
import pool
import reshape

def read_problem(options):
    f = open(options.problem, 'r')
    prob = {}
    while True:
        line = f.readline()
        if not line:
            break
        line = ' '.join(line.split())
        num_classes = parse("classes ={:d}", line)
        if (num_classes != None):
            prob['num_classes'] = num_classes[0]

        train_images = parse("train ={:^}", line)
        if (train_images != None):
            prob['train_images'] = train_images[0]

        test_images = parse("test ={:^}", line)
        if (test_images != None):
            prob['test_images'] = test_images[0]

        train_labels = parse("train_labels ={:^}", line)
        if (train_labels != None):
            prob['train_labels'] = train_labels[0]

        test_labels = parse("test_labels ={:^}", line)
        if (test_labels != None):
            prob['test_labels'] = test_labels[0]

        top = parse("top ={:d}", line)
        if (top != None):
            prob['top'] = top[0]

        height = parse("height ={:d}", line)
        if (height != None):
            prob['height'] = height[0]

        width = parse("width ={:d}", line)
        if (width != None):
            prob['width'] = width[0]

        channels = parse("channels ={:d}", line)
        if (channels != None):
            prob['channels'] = channels[0]

        dataset = parse("dataset ={:^}", line)
        if (dataset != None):
            prob['dataset'] = dataset[0]
    
    f.close()

    return prob

def parse_net(f, network):
    while True:
        sline = f.readline()
        if (sline == "\n" or sline == ""):
            break
        sline = ' '.join(sline.split())
        
        iters = parse("iter ={:d}", sline)
        if (iters != None):
            network["iter"] = iters[0]

        test_iter = parse("test_iter ={:d}", sline)
        if (test_iter != None):
            network["test_iter"] = test_iter[0]
        
        batch = parse("batch ={:d}", sline)
        if (batch != None):
            network["batch"] = batch[0]
        
        momentum = parse("momentum ={:f}", sline)
        if (momentum != None):
            network["momentum"] = momentum[0]
        
        decay = parse("decay ={:f}", sline)
        if (decay != None):
            network["decay"] = decay[0]
        
        learning_rate = parse("learning_rate ={:f}", sline)
        if (learning_rate != None):
            network["learning_rate"] = learning_rate[0]
        
        gamma = parse("gamma ={:f}", sline)
        if (gamma != None):
            network["gamma"] = gamma[0]
        
        power = parse("power ={:f}", sline)
        if (power != None):
            network["power"] = power[0]
        
        display = parse("display ={:d}", sline)
        if (display != None):
            network["display"] = display[0]

def parse_conv(f, name2layer, input_array, wino):
    while True:
        sline = f.readline()
        if (sline == "\n" or sline == ""):
            break
        sline = ' '.join(sline.split())
        
        _name = parse("name ={:^}", sline)
        if (_name != None):
            name = _name[0]
            
        _filters = parse("filters ={:d}", sline)
        if (_filters != None):
            filters = _filters[0]
            
        _size = parse("size ={:d}", sline)
        if (_size != None):
            size = _size[0]
            
        _stride = parse("stride ={:d}", sline)
        if (_stride != None):
            stride = _stride[0]
            
        activation = "none"
        _activation = parse("activation ={:^}", sline)
        if (_activation != None):
            activation = _activation[0]
            
        _bottom_name = parse("bottom ={:^}", sline)
        if (_bottom_name != None):
            bottom_name = _bottom_name[0]
           

    in_array = [0] * 3

    if (bottom_name.lower() == "input" or bottom_name.lower() == "data"):
        bottom = None
        in_array[0] = input_array[0]
        in_array[1] = input_array[1]
        in_array[2] = input_array[2]
    else:
        bottom = name2layer[bottom_name]
        in_array[0] = bottom.out_array[0]
        in_array[1] = bottom.out_array[1]
        in_array[2] = bottom.out_array[2]

    out_array = [0] * 3

    out_array[0] = (in_array[0] - size) / stride + 1 
    out_array[1] = (in_array[1] - size) / stride + 1 
    out_array[2] = filters
    
    if (activation == "none"):
        activation_array = [False, "relu"]
    elif (activation == "relu"):
        activation_array = [True, "relu"]
    elif (activation == "leaky_relu"):
        activation_array = [True, "leaky_relu"]
    
    tmp_conv = conv.conv(name, in_array, out_array, size, stride, activation_array, bottom, wino)
    name2layer[name] = tmp_conv

    return tmp_conv

def parse_pool(f, name2layer):
    pool_type = "max"
    while True:
        sline = f.readline()
        if (sline == "\n" or sline == ""):
            break
        sline = ' '.join(sline.split())
        
        _name = parse("name ={:^}", sline)
        if (_name != None):
            name = _name[0]
        
        _pool_type = parse("pool_type ={:^}", sline)
        if (_pool_type != None):
            pool_type = _pool_type[0]
            if (pool_type == "MAX" or pool_type == "max"):
                pool_type = "max"
            elif (pool_type == "AVG" or pool_type == "avg"):
                pool_type = "avg"

        _size = parse("size ={:d}", sline)
        if (_size != None):
            size = _size[0]
        
        _stride = parse("stride ={:d}", sline)
        if (_stride != None):
            stride = _stride[0]
        
        _bottom_name = parse("bottom ={:^}", sline)
        if (_bottom_name != None):
            bottom_name = _bottom_name[0]
        
    bottom = name2layer[bottom_name]

    in_array = [0] * 3

    in_array[0] = bottom.out_array[0]
    in_array[1] = bottom.out_array[1]
    in_array[2] = bottom.out_array[2]
    
    out_array = [0] * 3
    
    out_array[0] = (in_array[0] - size) / stride + 1 
    out_array[1] = (in_array[1] - size) / stride + 1 
    out_array[2] = in_array[2]
    
    tmp_pool = pool.pool(name, in_array, out_array, size, stride, pool_type, bottom)
    name2layer[name] = tmp_pool

    return tmp_pool

def parse_fully(f, name2layer, input_array):
    while True:
        sline = f.readline()
        if (sline == "\n" or sline == ""):
            break
        sline = ' '.join(sline.split())
    
        _name = parse("name ={:^}", sline)
        if (_name != None):
            name = _name[0]
        
        _output = parse("output ={:d}", sline)
        if (_output != None):
            output = _output[0]
        
        activation = "none"
        _activation = parse("activation ={:^}", sline)
        if (_activation != None):
            activation = _activation[0]
        
        _dropout = parse("dropout ={:f}", sline)
        if (_dropout != None):
            dropout = _dropout[0]
        
        _bottom_name = parse("bottom ={:^}", sline)
        if (_bottom_name != None):
            bottom_name = _bottom_name[0]
        

    in_array = [0] * 3

    if (bottom_name == "input"):
        bottom = None
        in_array[0] = input_array[0]
        in_array[1] = input_array[1]
        in_array[2] = input_array[2]
    else:
        bottom = name2layer[bottom_name]
        in_array[0] = bottom.out_array[0]
        in_array[1] = bottom.out_array[1]
        in_array[2] = bottom.out_array[2]

    out_array = [0] * 3
    
    out_array[0] = 1 
    out_array[1] = 1 
    out_array[2] = output
    if (activation == "none"):
        activation_array = [False, "relu"]
    elif (activation == "relu"):
        activation_array = [True, "relu"]
    elif (activation == "leaky_relu"):
        activation_array = [True, "leaky_relu"]
    

    tmp_fully = fully.fully(name, in_array, out_array, activation_array, bottom)
    name2layer[name] = tmp_fully
    
    return tmp_fully

def parse_reshape(f, name2layer, input_array):
    bottoms_name = []
    bottoms = []
    
    while True:
        sline = f.readline()
        if (sline == "\n" or sline == ""):
            break
        sline = ' '.join(sline.split())
    
        _name = parse("name ={:^}", sline)
        if (_name != None):
            name = _name[0]
        
        _output = parse("output ={:d}", sline)
        if (_output != None):
            output = _output[0]
       
        _bottoms_name = parse("bottoms ={:^}", sline)
        if (_bottoms_name != None):
            bottoms_name.append(_bottoms_name[0])

    #calc in_array
    in_array = [0] * 3
    
    in_array[0] = 1
    in_array[1] = 1
    in_array[2] = 0
    for bottom_name in bottoms_name:
        if (bottom_name.lower() == "input" or bottom_name.lower() == "data"):
            tmp_input = layer.layer("input", [1, 1, 1], input_array)
            bottoms.append(tmp_input)
            in_array[2] += (input_array[0] * input_array[1] * input_array[2])
        elif (bottom_name == "dummy"):
            bottoms.append("dummy")
        else:
            bottom = name2layer[bottom_name]
            bottoms.append(bottom)
            in_array[2] += (bottom.out_array[0] * bottom.out_array[1] * bottom.out_array[2])

    out_array = [0] * 3
    
    out_array[0] = 1 
    out_array[1] = 1 
    out_array[2] = output
   
    tmp_reshape = reshape.reshape(name, in_array, out_array, len(bottoms_name), bottoms)
    name2layer[name] = tmp_reshape

    return tmp_reshape

def read_network(options, problem):
    f = open(options.network, 'r')
    network = {}
    layer_arr = []
    name2layer = {}
    name2layer["input"] = None

    input_array = []

    input_array.append(problem["height"])
    input_array.append(problem["width"])
    input_array.append(problem["channels"])
    #print(input_array)

    cnt = 0

    while True:
        line = f.readline()
        line = ' '.join(line.split())
        if not line:
            break
       
        section = parse("[{}]", line)
        if (section != None):
            if (section[0] == "net"):
                parse_net(f, network)
            elif (section[0] == "conv"):
                if (options.winograd == True):
                    tmp_conv = parse_conv(f, name2layer, input_array, True)
                elif (options.winograd == False):
                    tmp_conv = parse_conv(f, name2layer, input_array, False)
                layer_arr.append(tmp_conv)
            elif (section[0] == "pool"):
                tmp_pool = parse_pool(f, name2layer)
                layer_arr.append(tmp_pool)
            elif (section[0] == "fully" or section[0] == "connected"):
                tmp_fully = parse_fully(f, name2layer, input_array)
                layer_arr.append(tmp_fully)
            elif (section[0] == "reshape"):
                tmp_reshape = parse_reshape(f, name2layer, input_array)
                layer_arr.append(tmp_reshape)

    f.close()

    network["layers"] = layer_arr

    return network

def read_mnist(problem):
    import struct
    import ctypes
    import array

    # Make Train
    images = open(problem["train_images"], 'r')
    labels = open(problem["train_labels"], 'r')

    out_train = open("train_image.byte", 'w')
    out_train_label = open("train_label.byte", 'w')
    images.read(4)
    labels.read(4)
    train_num = struct.unpack('>I', images.read(4))[0]
    if (train_num != struct.unpack('>I', labels.read(4))[0]):
        raise Exception("# of Images != # of Labels")
    images.read(4)
    images.read(4)

    label_array = array.array('f')
    img_array = array.array('f')
    for t in range(train_num):
        for j in range(problem["height"]):
            for i in range(problem["width"]):
                num = images.read(1)
                num = float(struct.unpack('>B', num)[0])
                img_array.append(num / 255.0)

        num = labels.read(1)
        num = float(struct.unpack('>B', num)[0])
        label_array.append(num)
    img_array.tofile(out_train)
    label_array.tofile(out_train_label)

    out_train_label.close()
    out_train.close()
    labels.close()
    images.close()

    # Make Test
    images = open(problem["test_images"], 'r')
    labels = open(problem["test_labels"], 'r')

    out_test = open("test_image.byte", 'w')
    out_test_label = open("test_label.byte", 'w')
    images.read(4)
    labels.read(4)
    test_num = struct.unpack('>I', images.read(4))[0]
    if (test_num != struct.unpack('>I', labels.read(4))[0]):
        raise Exception("# of Images != # of Labels")
    images.read(4)
    images.read(4)

    label_array = array.array('f')
    img_array = array.array('f')
    for t in range(test_num):
        for j in range(problem["height"]):
            for i in range(problem["width"]):
                num = images.read(1)
                num = float(struct.unpack('>B', num)[0])
                img_array.append(num / 255.0)

        num = labels.read(1)
        num = float(struct.unpack('>B', num)[0])
        label_array.append(num)
    
    img_array.tofile(out_test)
    label_array.tofile(out_test_label)

    out_test_label.close()
    out_test.close()
    labels.close()
    images.close()

def unpickle(file):
    import cPickle
    with open(file, 'rb') as fo:
        dict = cPickle.load(fo)
    return dict

def read_cifar(problem):
    import array

    # Make Train
    data_bin = unpickle(problem["train_images"])

    data = data_bin['data']
    if "labels" in data_bin.keys():
        labels = data_bin['labels']
    else:
        labels = data_bin['fine_labels']
    
    out_train = open("train_image.byte", 'w')
    out_train_label = open("train_label.byte", 'w')
    train_num = len(data)
    if (train_num != len(labels)):
        raise Exception("# of Images != # of Labels")

    label_array = array.array('f')
    img_array = array.array('f')
    for t in range(train_num):
        for c in range(problem["channels"]):
            for j in range(problem["height"]):
                for i in range(problem["width"]):
                    num = float(data[t][c * problem["height"] * problem["width"] + j * problem["width"] + i])
                    img_array.append(num / 255.0)
        num = float(labels[t])
        label_array.append(num)
        
    img_array.tofile(out_train)
    label_array.tofile(out_train_label)

    out_train_label.close()
    out_train.close()

    # Make Test
    data_bin = unpickle(problem["test_images"])
    data = data_bin['data']
    if "labels" in data_bin.keys():
        labels = data_bin['labels']
    else:
        labels = data_bin['fine_labels']
    
    out_test = open("test_image.byte", 'w')
    out_test_label = open("test_label.byte", 'w')
    test_num = len(data)
    if (test_num != len(labels)):
        raise Exception("# of Images != # of Labels")
    
    label_array = array.array('f')
    img_array = array.array('f')
    for t in range(test_num):
        for c in range(problem["channels"]):
            for j in range(problem["height"]):
                for i in range(problem["width"]):
                    num = float(data[t][c * problem["height"] * problem["width"] + j * problem["width"] + i])
                    img_array.append(num / 255.0)

        num = float(labels[t])
        label_array.append(num)

    img_array.tofile(out_test)
    label_array.tofile(out_test_label)

    out_test_label.close()
    out_test.close()
