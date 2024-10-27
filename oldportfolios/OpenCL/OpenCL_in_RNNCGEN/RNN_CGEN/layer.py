class Layer:

    def __init__(self, name, in_array, out_array):
        self.name = name
        self.in_array = in_array[:]
        self.in_size = in_array[0] * in_array[1] * in_array[2]
        self.out_array = out_array[:]
        self.out_size = out_array[0] * out_array[1] * out_array[2]

    def __make_funcs(self):
        return

    def __make_buffer(self):
        return
