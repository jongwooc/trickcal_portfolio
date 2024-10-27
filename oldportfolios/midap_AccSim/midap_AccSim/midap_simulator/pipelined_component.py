from __future__ import absolute_import, division, print_function, unicode_literals


class PipelinedComponent(object):
    def __init__(self):
        self.name = 'pipelined_component'
        self.output_buf = None  # should be overided
        self.output_info = None

    def setup(self):
        self.output_info = None

    def work(self, input_buf, input_info):
        # Should be overrided or overloaded
        pass
