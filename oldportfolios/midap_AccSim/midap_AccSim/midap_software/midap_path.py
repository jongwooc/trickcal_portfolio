from config import cfg

# TODO Do we need to make MidapPath class?


def get_path(prev_layer, current_layer, current_path, cost):
    layer = current_layer
    if prev_layer not in layer.input:
        raise ValueError("Unknown path - prev_layer : {}, layer {} - layer.inputs : {}".format(
            prev_layer.name, layer.name, [l.name for l in layer.input]))

    def make_path(p, f, c, o):
        return {'path': p, 'furcation': f, 'cost': c, 'output': o}

    current_path.append(layer)
    if len(layer.next) == 0:
        # print("End route: no next - layer inputs: {}, prev_layer = {}".format([l.name for l in layer.input], prev_layer.name))
        return make_path(current_path, False, cost, True)

    if len(layer.input) > 1:
        # print("End route - layer inputs: {}, prev_layer = {}".format([l.name for l in layer.input], prev_layer.name))
        return make_path(current_path, False, cost, False)
    else:
        cost.append(layer.require_fmem)
        if len(layer.next) > 1:
            extra_cost = layer.require_fmem
            for next_layer in layer.next:
                p = get_path(layer, next_layer, [], [])
                new_cost = [c + extra_cost for c in p['cost']]
                cost += new_cost
            return make_path(current_path, True, cost, False)
        return get_path(current_layer, layer.next[0], current_path, cost)


def classify_paths(paths):  # will be deprecated
    out_paths = []
    inner_paths = []
    unifying_layer = None
    for path, last_layer in paths:
        if last_layer is None:
            out_paths.append(path)
        else:
            if unifying_layer is None:
                unifying_layer = last_layer
            else:
                if unifying_layer != last_layer:
                    #self.error_log = paths
                    raise ValueError("Diversed branches are not unified: {}, error_log: {}".format(
                        (unifying_layer.name, last_layer.name), paths))
            inner_paths.append(path)

    # out_paths.sort(key=(lambda x: (max(x), len(x))))
    # inner_paths.sort(key=(lambda x: (max(x), len(x))))
    out_paths.sort(key=(
        lambda x: 100 * sum([max(0, l.require_fmem - cfg.MIDAP.FMEM.NUM + 1) for l in x[:-1]]) + len(x)))
    inner_paths.sort(key=(
        lambda x: 100 * sum([max(0, l.require_fmem - cfg.MIDAP.FMEM.NUM + 1) for l in x[:-1]]) + len(x)))
    # print([p['cost'] for p in inner_paths])
    # for path in inner_paths:
    #     if path['path'][-1] != inner_paths[0]['path'][-1] and not path['furcation']:
    #         raise ValueError("Not yet supported network! - recursively diversed graph")
    return out_paths, inner_paths, unifying_layer
