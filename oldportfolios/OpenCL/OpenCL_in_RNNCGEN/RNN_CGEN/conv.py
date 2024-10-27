import layer


class conv(layer.Layer):

    def __init__(self, name, in_array, out_array, size, stride, activation_array, prev_layer, wino):
        layer.Layer.__init__(self, name, in_array, out_array)

        self.out_array = out_array
        self.size = size
        self.wino = wino
        self.__make_funcs()
        self.__make_gemm_funcs()
        self.stride = stride
        self.__make_buffer()

        self.use_activation = activation_array[0]
        self.act_func = activation_array[1]
        self.grad_func = activation_array[1] + "_grad"
        self.workspace_size = self.in_size * size * size
        self.prev_layer = prev_layer

    def __make_gemm_funcs(self):
        self.gemm_funcs = "void " + self.name + "_gemm_nn(float *A, float *B, float *C);\nvoid " + \
            self.name + "_gemm_nt(float *A, float *B, float *C);\nvoid " + \
            self.name + "_gemm_tn(float *A, float *B, float *C);\n"

    def __make_funcs(self):
        if self.size == 3 and self.wino == True:
            self.funcs = "void " + self.name + "_wino_forward(float *pfWeight, float *pfBias, float *pfPrevData);\nvoid " + \
                self.name + "_wino_backprop(float *pfWeight, float *pfPrevData, float *pPrevDelta);\nvoid " + \
                self.name + "_update(float *pfWeight, float *pfBias);\nfloat *get_" + \
                self.name + "_delta();\nfloat *get_" + self.name + "_data();\n"
        else:
            self.funcs = "void " + self.name + "_forward(float *pfWeight, float *pfBias, float *pfPrevData);\nvoid " + \
                self.name + "_backprop(float *pfWeight, float *pfPrevData, float *pPrevDelta);\nvoid " + \
                self.name + "_update(float *pfWeight, float *pfBias);\nfloat *get_" + \
                self.name + "_delta();\nfloat *get_" + self.name + "_data();\n"

    def __make_buffer(self):
        buf_idx = 0
        bias_size = self.out_array[2]
        weight_size = self.in_array[2] * \
            self.size * self.size * self.out_array[2]
        delta_size = self.out_size

        self.buffer = "static float buffer[" + str(
            weight_size + bias_size) + " + BATCH * " + str(self.out_size + delta_size) + "];\n"
        self.buffer = self.buffer + \
            "static float *pfWeightUpdate = &buffer[" + str(buf_idx) + "];\n"
        buf_idx = buf_idx + weight_size
        self.buffer = self.buffer + \
            "static float *pfBiasUpdate = &buffer[" + str(buf_idx) + "];\n"
        buf_idx = buf_idx + bias_size
        self.buffer = self.buffer + \
            "static float *pfDelta = &buffer[" + str(buf_idx) + "];\n"
        self.buffer = self.buffer + \
            "static float *pfData = &buffer[" + \
            str(buf_idx) + " + BATCH * " + str(delta_size) + "];"


def make_gemm(network):
    for layer in network["layers"]:
        if isinstance(layer, conv):
            gemm_f = open("./templates/gemm_nn.tc", 'r')
            gen_f = open("./generated/" + layer.name + "_gemm_nn.c", 'w')
            while True:
                line = gemm_f.readline()
                if not line:
                    break

                line = line.replace("%(name)", layer.name)

                line = line.replace("%(M)", str(layer.out_array[2]))
                line = line.replace("%(N)",
                                    str(layer.out_array[0] * layer.out_array[1]))
                line = line.replace("%(K)",
                                    str(layer.in_array[2] * layer.size * layer.size))

                line = line.replace("%(lda)",
                                    str(layer.in_array[2] * layer.size * layer.size))
                line = line.replace("%(ldb)",
                                    str(layer.out_array[0] * layer.out_array[1]))
                line = line.replace("%(ldc)",
                                    str(layer.out_array[0] * layer.out_array[1]))

                gen_f.write(line)
            gen_f.close()
            gemm_f.close()

            gemm_f = open("./templates/gemm_nt.tc", 'r')
            gen_f = open("./generated/" + layer.name + "_gemm_nt.c", 'w')
            while True:
                line = gemm_f.readline()
                if not line:
                    break

                line = line.replace("%(name)", layer.name)

                line = line.replace("%(M)", str(layer.out_array[2]))
                line = line.replace("%(N)",
                                    str(layer.in_array[2] * layer.size * layer.size))
                line = line.replace("%(K)",
                                    str(layer.out_array[0] * layer.out_array[1]))

                line = line.replace("%(lda)",
                                    str(layer.out_array[0] * layer.out_array[1]))
                line = line.replace("%(ldb)",
                                    str(layer.out_array[0] * layer.out_array[1]))
                line = line.replace("%(ldc)",
                                    str(layer.in_array[2] * layer.size * layer.size))

                gen_f.write(line)
            gen_f.close()
            gemm_f.close()

            gemm_f = open("./templates/gemm_tn.tc", 'r')
            gen_f = open("./generated/" + layer.name + "_gemm_tn.c", 'w')
            while True:
                line = gemm_f.readline()
                if not line:
                    break

                line = line.replace("%(name)", layer.name)

                line = line.replace("%(M)",
                                    str(layer.in_array[2] * layer.size * layer.size))
                line = line.replace("%(N)",
                                    str(layer.out_array[0] * layer.out_array[1]))
                line = line.replace("%(K)", str(layer.out_array[2]))

                line = line.replace("%(lda)",
                                    str(layer.in_array[2] * layer.size * layer.size))
                line = line.replace("%(ldb)",
                                    str(layer.out_array[0] * layer.out_array[1]))
                line = line.replace("%(ldc)",
                                    str(layer.out_array[0] * layer.out_array[1]))
                gen_f.write(line)
            gen_f.close()
            gemm_f.close()
            
def make_conv_etc(network, layer):
    workspace_size = 0
    if layer.size == 3 and layer.wino == True:
        temp_f = open("./templates/_wino.th", 'r')
        workspace_size = max(workspace_size, 10 * layer.workspace_size)
    else:
        temp_f = open("./templates/_conv.th", 'r')
        workspace_size = max(workspace_size, layer.workspace_size)
    gen_f = open("./generated/_" + layer.name + ".h", 'w')
    while True:
        line = temp_f.readline()
        if not line:
            break

        line = line.replace("%(workspace_size)", str(workspace_size))
        line = line.replace("%(layer_buffer)", layer.buffer)
        line = line.replace("%(name)", layer.name)
        line = line.replace("%(num_output)", str(layer.out_size))

        ## About Filter ##
        line = line.replace("%(kernel_size)", str(layer.size))
        line = line.replace("%(stride)", str(layer.stride))

        ## About Output ##
        line = line.replace("%(out_width)", str(layer.out_array[0]))
        line = line.replace("%(out_height)", str(layer.out_array[1]))
        line = line.replace("%(out_channel)", str(layer.out_array[2]))

        ## About Input ##
        line = line.replace("%(in_width)", str(layer.in_array[0]))
        line = line.replace("%(in_height)", str(layer.in_array[1]))
        line = line.replace("%(in_channel)", str(layer.in_array[2]))

        line = line.replace("%(batch)", str(network["batch"]))
        line = line.replace("%(out_area)",
                            str(layer.out_array[0] * layer.out_array[1]))
        line = line.replace("%(kernel_weight)",
                            str(layer.in_array[2] * layer.size * layer.size))
        line = line.replace("%(kernel_total)",
                            str(layer.out_array[2] * layer.in_array[2] * layer.size * layer.size))
        line = line.replace("%(out_total)",
                            str(layer.out_size * network["batch"]))

        ## About Activation ##
        if (layer.use_activation):
            line = line.replace("%(use_activation)", "1")
        else:
            line = line.replace("%(use_activation)", "0")

        line = line.replace("%(act_func)", layer.act_func)
        line = line.replace("%(grad_func)", layer.grad_func)

        if (layer.act_func is "relu"):
            line = line.replace("%(use_relu)", "1")
        else:
            line = line.replace("%(use_relu)", "0")
        if (layer.act_func is "leaky_relu"):
            line = line.replace("%(use_leaky_relu)", "1")
        else:
            line = line.replace("%(use_leaky_relu)", "0")

        gen_f.write(line)
    gen_f.close()
    temp_f.close()

def make_conv(network):
    # Make c files
    funcs = ""
    for layer in network["layers"]:
        if isinstance(layer, conv):
            make_conv_etc(network, layer)
            if layer.size == 3 and layer.wino == True:
                temp_f = open("./templates/wino.tc", 'r')
            else:
                temp_f = open("./templates/conv.tc", 'r')
            gen_f = open("./generated/" + layer.name + ".c", 'w')
            while True:
                line = temp_f.readline()
                if not line:
                    break

                line = line.replace("%(layer_buffer)", layer.buffer)
                line = line.replace("%(name)", layer.name)
                line = line.replace("%(num_output)", str(layer.out_size))

                ## About Filter ##
                line = line.replace("%(kernel_size)", str(layer.size))
                line = line.replace("%(stride)", str(layer.stride))

                ## About Output ##
                line = line.replace("%(out_width)", str(layer.out_array[0]))
                line = line.replace("%(out_height)", str(layer.out_array[1]))
                line = line.replace("%(out_channel)", str(layer.out_array[2]))

                ## About Input ##
                line = line.replace("%(in_width)", str(layer.in_array[0]))
                line = line.replace("%(in_height)", str(layer.in_array[1]))
                line = line.replace("%(in_channel)", str(layer.in_array[2]))

                line = line.replace("%(batch)", str(network["batch"]))
                line = line.replace("%(out_area)",
                                    str(layer.out_array[0] * layer.out_array[1]))
                line = line.replace("%(kernel_weight)",
                                    str(layer.in_array[2] * layer.size * layer.size))
                line = line.replace("%(kernel_total)",
                                    str(layer.out_array[2] * layer.in_array[2] * layer.size * layer.size))
                line = line.replace("%(out_total)",
                                    str(layer.out_size * network["batch"]))

                ## About Activation ##
                if (layer.use_activation):
                    line = line.replace("%(use_activation)", "1")
                else:
                    line = line.replace("%(use_activation)", "0")

                line = line.replace("%(act_func)", layer.act_func)
                line = line.replace("%(grad_func)", layer.grad_func)

                if (layer.act_func is "relu"):
                    line = line.replace("%(use_relu)", "1")
                else:
                    line = line.replace("%(use_relu)", "0")
                if (layer.act_func is "leaky_relu"):
                    line = line.replace("%(use_leaky_relu)", "1")
                else:
                    line = line.replace("%(use_leaky_relu)", "0")

                gen_f.write(line)
            gen_f.close()
            temp_f.close()

            funcs = funcs + layer.funcs

    # Make header
    gen_header_f = open(
        "./generated/conv.h", 'w')
    temp_f = open("./templates/conv.th", 'r')
    while True:
        line = temp_f.readline()
        if not line:
            break
        line = line.replace("%(dec_funcs)", funcs)
        gen_header_f.write(line)

    temp_f.close()
    gen_header_f.close()
