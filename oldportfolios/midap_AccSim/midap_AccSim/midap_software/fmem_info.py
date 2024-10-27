from collections import deque

from acc_utils.errors import CompilerError
from config import cfg


class FMEMInfo(object):
    def __init__(self):
        self.reset()

    def __del__(self):
        del self.reserved_banks, self.available_queue, self.mapping_info

    def reset(self):
        self.reserved_banks = set([])
        self.available_queue = deque([i for i in range(cfg.MIDAP.FMEM.NUM)])
        self.mapping_info = []

    def discard_data_by_layer(self, name, reverse_order=False):
        discard_list = []
        for idx, data in enumerate(self.mapping_info):
            n = data[0]
            if n == name:
                discard_list.append(idx)

        # discard the data in the order used by CIMs. (available_queue)
        for idx in (reversed(discard_list) if reverse_order else discard_list):
            _, bank, _ = self.mapping_info[idx]
            if reverse_order:
                self.available_queue.appendleft(bank)
            else:
                self.available_queue.append(bank)
            self.reserved_banks.discard(bank)

        for idx in reversed(discard_list):
            del self.mapping_info[idx]

    def discard_data(self, bank):
        discard_idx = -1
        for idx, data in enumerate(self.mapping_info):
            b = data[1]
            if b == bank:
                discard_idx = idx
                break

        if bank not in self.reserved_banks:
            self.available_queue.append(bank)
            del self.mapping_info[discard_idx]

    def _pop_available_bank(self):
        if not self.available_queue:
            return None
        bank = self.available_queue.popleft()
        return bank

    def get_num_available_bank(self):
        return len(self.available_queue)

    def get_num_unreserved_bank(self):
        return cfg.MIDAP.FMEM.NUM - len(self.reserved_banks)

    def save_data_to_empty_bank(self, layer, data):
        name = layer.name

        # FIXME check that data is already in fmem.
        for n, b, d in self.mapping_info:
            if name == n and d == data:
                return None

        bank = self._pop_available_bank()
        if bank is not None:
            self._save_data(name, bank, data)
        return bank

    def _save_data(self, name, bank, data):
        self.mapping_info.append((name, bank, data))

        if len(self.mapping_info) > cfg.MIDAP.FMEM.NUM:
            raise CompilerError("Use more banks than exists: " +
                                str(self.mapping_info) + self.__repr__())

    def reverse_mapping(self, name):
        mapping_list = []
        discard_list = []
        for idx, data in enumerate(self.mapping_info):
            n = data[0] if data else None
            if n == name:
                mapping_list.append(data)
                discard_list.append(idx)

        for idx in reversed(discard_list):
            del self.mapping_info[idx]
        self.mapping_info = self.mapping_info + list(reversed(mapping_list))

    def reserve_input_banks(self, mapping, input_stationary):
        if mapping and input_stationary >= 0:
            banks = [v[0] for v in mapping[:input_stationary]]
            self.reserved_banks = set(banks)

    def reserve_output_banks(self, next_layer, output_stationary):
        # FIXME
        # set output bank once per path(or branch)
        for data in self.mapping_info:
            n = data[0] if data else None
            if n == next_layer.name:
                return

        fragments = next_layer.get_output_fragments(cfg.MIDAP.FMEM.NUM - 1)
        for f in fragments[:output_stationary]:
            bank = self.save_data_to_empty_bank(next_layer, f)
            if bank is None:
                raise CompilerError(
                    "There is no bank to save output of " + next_layer.name + " " + self.__repr__())
            self.reserved_banks.add(bank)

    def get_fmem_mapping_info(self, name):
        mapping = []
        for data in self.mapping_info:
            n = data[0] if data else None
            if n == name:
                mapping.append([data[1], data[2], False])
        return mapping

    def __repr__(self):
        usage = ['O'] * cfg.MIDAP.FMEM.NUM
        for data in self.mapping_info:
            usage[data[1]] = data[0]

        return "FMEM Available Bank {}".format(usage)
