from __future__ import absolute_import, division, print_function, unicode_literals

from config import cfg
from midap_software.net_divider import NetDivider
from midap_software.static_estimator import estimator
from midap_software.subnet_compiler import SubNetCompiler


class Compiler(dict):
    def __init__(self, *args, **kwargs):
        super(Compiler, self).__init__(*args, **kwargs)
        self.model = None
        self.op_dict = None

        self.network_divider = NetDivider() if cfg.MODEL.USE_TILING else None
        self.subnet_compiler = SubNetCompiler()

    def set_op_dict(self, op_dict):
        self.op_dict = op_dict

    def setup(self, midap_model):
        self.model = midap_model

        # static performance estimator
        estimator.setup(midap_model)
        static_info = estimator.calc_approximate_cycle()

        # TODO divide whole network to small networks for tiling
        if cfg.MODEL.USE_TILING:
            self.network_divider.devide_network(midap_model)

        # compiler setting
        self.subnet_compiler.setup(midap_model)
        return static_info

    def force_setup(self, num_init_banks):
        self.subnet_compiler.force_setup(num_init_banks)

    def get_control_info(self):
        input_tensor_list, path_info = self.subnet_compiler.get_model_control_info()
        return input_tensor_list, path_info
