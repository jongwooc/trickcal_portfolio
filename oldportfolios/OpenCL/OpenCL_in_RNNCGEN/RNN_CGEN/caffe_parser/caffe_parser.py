import os
import sys
import caffe_pb2

from parse import *

from google.protobuf import text_format

# default accuracy is 1-accuracy.
top = 1
batch_size = 1
classes = 1
height = 1
width = 1
channels = 1
dataset = "mnist"

def dequote(s):
    if (s[0] == s[-1]) and s.startswith(("'", '"')):
        return s[1:-1]
    return s

def parse_solver(filepath):
    global cgen_path, top, batch_size, classes, height, width, channels, dataset
    
    rf = open(filepath, "r")
    
    if not rf:
        raise self.ProcessException("ERROR (" + filepath + ")!")
    
    net = caffe_pb2.NetParameter()
    out_filename = ""

    while True:
        line = rf.readline()
        if not line:
            break
        
        line = ' '.join(line.split())
        
        net_path = parse("net:{:^}", line)
        if (net_path != None):
            net_path[0].replace('"', '')
            path = cgen_path + "/" + dequote(net_path[0])
            out_filename = cgen_path + "/caffe_parser/cfg_output/parsed_cfg.cfg"
            net = _readProtoFile(path, net)
            break
    
    wf = open(out_filename, 'w')
    wf.write("[net]\n")
    
    while True:
        line = rf.readline()
        if not line:
            break
        line = ' '.join(line.split())
        
        momentum = parse("momentum:{:f}", line)
        if (momentum != None):
            wf.write("momentum = " + str(momentum[0]) + "\n")
            continue
        
        decay = parse("weight_decay:{:f}", line)
        if (decay != None):
            wf.write("decay = " + str(decay[0]) + "\n")
            continue

        gamma = parse("gamma:{:f}", line)
        if (gamma != None):
            wf.write("gamma = " + str(gamma[0]) + "\n")
            continue

        learning_rate = parse("base_lr:{:f}", line)
        if (learning_rate != None):
            wf.write("learning_rate = " + str(learning_rate[0]) + "\n")
            continue

        _policy = parse("lr_policy:{:^}", line)
        if (_policy != None):
            policy = dequote(_policy[0])
            wf.write("policy = " + policy + "\n")
            continue

        power = parse("power:{:f}", line)
        if (power != None):
            wf.write("power = " + str(power[0]) + "\n")
            continue

        iters = parse("max_iter:{:d}", line)
        if (iters != None):
            wf.write("iter = " + str(iters[0]) + "\n")
            continue

        test_iter = parse("test_iter:{:d}", line)
        if (test_iter != None):
            wf.write("test_iter = " + str(test_iter[0]) + "\n")
            continue
        
        display = parse("display:{:d}", line)
        if (display != None):
            wf.write("display = " + str(display[0]) + "\n")
            continue
       
        ### things kept in global variables ###
        _dataset = parse("dataset:{:^}", line)
        if (_dataset != None):
            dataset = _dataset[0]
            continue
        
        _height = parse("height:{:d}", line)
        if (_height != None):
            height = _height[0]
            continue
        
        _width = parse("width:{:d}", line)
        if (_width != None):
            width = _width[0]
            continue
        
        _channels = parse("channels:{:d}", line)
        if (_channels != None):
            channels = _channels[0]
            continue
        
        if not (str(line).startswith('#') or len(str(line)) < 1):
            print("[NO SUPPORT] (" + str(line) + ") in solver will be ignored.")

    rf.close()
    wf.close()

    return net, out_filename

def _readProtoFile(filepath, parser_object):
    f = open(filepath, "r")

    if not f:
        raise self.ProcessException("ERROR (" + filepath + ")!")

    text_format.Merge(f.read(), parser_object)
    
    f.close()
    return parser_object

if __name__ == '__main__':
    if (len(sys.argv) == 1):
        print("please input a caffe solver prototxt file")
        exit()
 
    global cgen_path
    cgen_path = os.environ['CGEN_PATH']

    out_filename = ""
    net = caffe_pb2.NetParameter()

    dirname = cgen_path + "/caffe_parser/pd_output"
    if not os.path.isdir(os.path.join(os.getcwd(), dirname)):
        os.makedirs(os.path.join(os.getcwd(), dirname))
        print "make dir " + os.getcwd()
    
    dirname = cgen_path + "/caffe_parser/cfg_output"
    if not os.path.isdir(os.path.join(os.getcwd(), dirname)):
        os.makedirs(os.path.join(os.getcwd(), dirname))
        print "make dir " + os.getcwd()
    
    out_pd_filename = cgen_path + "/caffe_parser/pd_output/parsed_pd.data"
    pd_f = open(out_pd_filename, 'w')

    net, out_cfg_filename = parse_solver(cgen_path + "/" + sys.argv[1])
    cfg_f = open(out_cfg_filename, 'a+')

    for l in net.layer :
        layer_type = str(l.type).lower()
        if (layer_type == "data"):
            # TRAIN
            if (l.include[0].phase == 0):
                source = l.data_param.source
                train_data = l.data_param.data_source
                train_label = l.data_param.label_source
                if (len(source) > 0):
                    pd_f.write("train = " + source + "\n")
                else:
                    pd_f.write("train = " + train_data + "\n")
                    pd_f.write("train_labels = " + train_label + "\n")
                batch_size = l.data_param.batch_size
            # TEST
            elif (l.include[0].phase == 1):
                source = l.data_param.source
                test_data = l.data_param.data_source
                test_label = l.data_param.label_source
                if (len(source) > 0):
                    pd_f.write("test = " + source + "\n")
                else:
                    pd_f.write("test = " + test_data + "\n")
                    pd_f.write("test_labels = " + test_label + "\n")
                if (batch_size != l.data_param.batch_size):
                    print("[NOTE] test batch size (" + str(l.data_param.batch_size) + ") will be ignored. The training batch size (" + str(batch_size) + ") will be applied.")
        elif (layer_type == "convolution" or layer_type == "conv"):
            cfg_f.write("\n[conv]\n")
            cfg_f.write("name = " + str(l.name) + "\n")
            cfg_f.write("bottom = " + str(l.bottom[0]) + "\n")
            cfg_f.write("filters = " + str(l.convolution_param.num_output) + "\n")
            cfg_f.write("size = " + str(l.convolution_param.kernel_size[0]) + "\n")
            cfg_f.write("stride = " + str(l.convolution_param.stride[0]) + "\n")
        elif (layer_type == "pooling" or layer_type == "pool"):
            cfg_f.write("\n[pool]\n")
            cfg_f.write("name = " + str(l.name) + "\n")
            cfg_f.write("bottom = " + str(l.bottom[0]) + "\n")
            if (l.pooling_param.pool == 0):
                cfg_f.write("pool_type = " + "MAX" + "\n")
            elif (l.pooling_param.pool == 1):
                cfg_f.write("pool_type = " + "AVG" + "\n")
            cfg_f.write("size = " + str(l.pooling_param.kernel_size) + "\n")
            cfg_f.write("stride = " + str(l.pooling_param.stride) + "\n")
        elif (layer_type == "innerproduct" or layer_type == "fully" or layer_type == "connected"):
            cfg_f.write("\n[fully]\n")
            cfg_f.write("name = " + str(l.name) + "\n")
            cfg_f.write("bottom = " + str(l.bottom[0]) + "\n")
            cfg_f.write("output = " + str(l.inner_product_param.num_output) + "\n")
            classes = l.inner_product_param.num_output
        elif (layer_type == "concat" or layer_type == "reshape"):
            cfg_f.write("\n[reshape]\n")
            cfg_f.write("name = " + str(l.name) + "\n")
            for i in range(len(l.bottom)):
                cfg_f.write("bottoms = " + str(l.bottom[i]) + "\n")
            cfg_f.write("output = " + str(l.concat_param.concat_dim) + "\n")
        # assume that activation layers are specified right after the target layer.
        elif (layer_type == "relu" or layer_type == "leaky_relu"):
            cfg_f.write("activation = " + str(layer_type) + "\n")
        elif (layer_type == "input" or layer_type == "softmax"):
            continue; 
        else:
            print("[WARNING] No support for " + layer_type)
   

    cfg_f.seek(0)
    cfg_contents = cfg_f.readlines()
    cfg_contents.insert(1, "batch = " + str(batch_size) + "\n")
    cfg_f.close()

    cfg_f = open(out_cfg_filename, "w")
    cfg_contents = "".join(cfg_contents)
    cfg_f.write(cfg_contents)

    pd_f.write("classes = " + str(classes) + "\n")
    pd_f.write("top = " + str(top) + "\n")
    pd_f.write("dataset = " + str(dataset) + "\n")
    pd_f.write("height = " + str(height) + "\n")
    pd_f.write("width = " + str(width) + "\n")
    pd_f.write("channels = " + str(channels) + "\n")

    cfg_f.close()
    pd_f.close()

