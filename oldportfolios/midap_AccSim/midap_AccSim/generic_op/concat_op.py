from acc_utils.errors import _assert

from .operator_base import OperatorBase


class ConcatOp(OperatorBase):
    def __init__(self, op_type='Concat', channel_info=None, **kwargs):
        super(ConcatOp, self).__init__(op_type=op_type, **kwargs)
        _assert(isinstance(channel_info, list),
                'channel_info must be given as a channel size list')
        self.size_info = [sum(channel_info[:i])
                          for i in range(len(channel_info))]

    def __repr__(self):
        return super(ConcatOp, self).__repr__() + "offset information: {}\n".format(self.size_info)
