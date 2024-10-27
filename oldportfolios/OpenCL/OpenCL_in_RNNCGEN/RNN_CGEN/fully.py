import layer


class fully(layer.Layer):

    def __init__(self, name, in_array, out_array, activation_array, prev_layer):
        layer.Layer.__init__(self, name, in_array, out_array)

        self.out_array = out_array
        self.funcs = "void " + name + "_forward(float *pfWeight, float *pfBias, float *pfPrevData);\nvoid " + \
            name + "_backprop(float *pfWeight, float *pfPrevData, float *pPrevDelta);\nvoid " + \
            name + "_update(float *pfWeight, float *pfBias);\nfloat *get_" + \
            name + "_delta();\nfloat *get_" + name + "_data();\n"
        self.__make_gemm_funcs()
        self.__make_buffer()
        self.prev_layer = prev_layer

        self.use_activation = activation_array[0]
        self.act_func = activation_array[1]
        self.grad_func = activation_array[1] + "_grad"

    def __make_gemm_funcs(self):
        self.gemm_funcs = "void " + self.name + "_gemm_nn(float *A, float *B, float *C);\nvoid " + \
            self.name + "_gemm_nt(float *A, float *B, float *C);\nvoid " + \
            self.name + "_gemm_tn(float *A, float *B, float *C);\n"

    def __make_buffer(self):
        buf_idx = 0
        self.buffer = "static float buffer[" + str(
            self.out_size * self.in_size + self.out_size) + " + BATCH * " + str(2 * self.out_size) + "];\n"
        self.buffer = self.buffer + \
            "static float *pfWeightUpdate = &buffer[" + \
            str(buf_idx) + "];\n"
        buf_idx = buf_idx + (self.out_size * self.in_size)
        self.buffer = self.buffer + \
            "static float *pfBiasUpdate = &buffer[" + \
            str(buf_idx) + "];\n"
        buf_idx = buf_idx + self.out_size
        self.buffer = self.buffer + \
            "static float *pfDelta = &buffer[" + \
            str(buf_idx) + "];\n"
        self.buffer = self.buffer + \
            "static float *pfData = &buffer[" + \
            str(buf_idx) + " + BATCH * " + str(self.out_size) + "];"


def make_gemm(network):
    # Make c files
    for layer in network["layers"]:
        if isinstance(layer, fully):
            gemm_f = open("./templates/gemm_nn.tc", 'r')
            gen_f = open("./generated/" + layer.name + "_gemm_nn.c", 'w')
            while True:
                line = gemm_f.readline()
                if not line:
                    break

                line = line.replace("%(name)", layer.name)

                line = line.replace("%(M)", str(network["batch"]))
                line = line.replace("%(N)", str(layer.in_size))
                line = line.replace("%(K)", str(layer.out_size))

                line = line.replace("%(lda)", str(layer.out_size))
                line = line.replace("%(ldb)", str(layer.in_size))
                line = line.replace("%(ldc)", str(layer.in_size))

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

                line = line.replace("%(M)", str(network["batch"]))
                line = line.replace("%(N)", str(layer.out_size))
                line = line.replace("%(K)", str(layer.in_size))

                line = line.replace("%(lda)", str(layer.in_size))
                line = line.replace("%(ldb)", str(layer.in_size))
                line = line.replace("%(ldc)", str(layer.out_size))

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

                line = line.replace("%(M)", str(layer.out_size))
                line = line.replace("%(N)", str(layer.in_size))
                line = line.replace("%(K)", str(network["batch"]))

                line = line.replace("%(lda)", str(layer.out_size))
                line = line.replace("%(ldb)", str(layer.in_size))
                line = line.replace("%(ldc)", str(layer.in_size))

                gen_f.write(line)
            gen_f.close()
            gemm_f.close()

def make_fully_etc(network, layer):
    temp_f = open("./templates/_fully.th", 'r')
    gen_f = open("./generated/_" + layer.name + ".h", 'w')
    while True:
        line = temp_f.readline()
        if not line:
            break

        line = line.replace("%(layer_buffer)", layer.buffer)
        line = line.replace("%(name)", layer.name)
        line = line.replace("%(num_output)", str(layer.out_size))
        line = line.replace("%(num_input)", str(layer.in_size))

        line = line.replace("%(batch)", str(network["batch"]))
        line = line.replace("%(num_weight)",
                            str(layer.out_size * layer.in_size))
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

def make_fully(network):
    # Make c files
    funcs = ""
    for layer in network["layers"]:
        if isinstance(layer, fully):
            make_fully_etc(network, layer)
            temp_f = open("./templates/fully.tc", 'r')
            gen_f = open("./generated/" + layer.name + ".c", 'w')
            while True:
                line = temp_f.readline()
                if not line:
                    break

                line = line.replace("%(layer_buffer)", layer.buffer)
                line = line.replace("%(name)", layer.name)
                line = line.replace("%(num_output)", str(layer.out_size))
                line = line.replace("%(num_input)", str(layer.in_size))

                line = line.replace("%(batch)", str(network["batch"]))
                line = line.replace("%(num_weight)",
                                    str(layer.out_size * layer.in_size))
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
        "./generated/fully.h", 'w')
    temp_f = open("./templates/fully.th", 'r')
    while True:
        line = temp_f.readline()
        if not line:
            break
        line = line.replace("%(dec_funcs)", funcs)
        gen_header_f.write(line)

    temp_f.close()
    gen_header_f.close()
