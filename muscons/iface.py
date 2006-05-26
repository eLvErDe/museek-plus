import re

def iface_import(fn):
    results = {}
    data = open(fn, 'r').read().split('\n')
    for line in data:
        if line[:13] == 'IFACEMESSAGE(':
            name, code = line[13:-1].split(', ')
            results['@@' + name + '@@'] = code
    return results

def iface_merge(outf, fn, iface_map):
    of = open(outf, 'w')
    data = open(fn, 'r').read().split('\n')
    for line in data:
    	for key in iface_map.keys():
            line = line.replace(key, iface_map[key])
    	of.write(line + '\n')
    of.close()

def IfaceGen(target, source, template):
    iface_merge(target, source, iface_import(template))
