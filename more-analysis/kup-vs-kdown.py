import struct
import sys
import re

def read_n(filename):
    with open(filename, "rb") as f:
        return struct.unpack("<H", f.read(2))[0]

def read_sol_size(filename):
    with open(filename, "r") as f:
        for line in f:
            if line.startswith("Solution size"):
                return int(line.strip().split()[2])

def analyse_instance(instance, runtime_kup, runtime_kdown):
    n0 = read_n(instance[1])
    n1 = read_n(instance[2])
    solsize_kup = read_sol_size(
            "../experiments/gpgnode-results/mcsplain/james-cpp-max/" + instance[0] + ".out")
    solsize_kdown = read_sol_size(
            "../experiments/gpgnode-results/mcsplain/james-cpp-max/" + instance[0] + ".out")
    if solsize_kup != solsize_kdown:
        sys.stderr.write("Solution sizes differ between runs!\n")
        sys.exit(1)
    runtime_ratio = float(runtime_kup) / runtime_kdown
    print instance[0], n0, n1, runtime_kup, runtime_kdown, runtime_ratio, solsize_kup, \
            float(solsize_kup) / n0
    
def print_header():
    print "instance", \
            "n_pattern", \
            "n_target", \
            "runtime_kup", \
            "runtime_kdown", \
            "runtime_ratio", \
            "solsize", \
            "solsize_over_n_pattern"
def go():
    print_header()
    runtimes = {}
    with open("../experiments/gpgnode-results/mcsplain/runtimes.data", "r") as f:
        lines = [line.strip().split() for line in f]
    for tokens in lines[1:]:
        kup_time = int(tokens[7])
        kdown_time = int(tokens[8])
        if kup_time == 0:
            kup_time = 1
        if kdown_time == 0:
            kdown_time = 1
        runtimes[tokens[0]] = {"kup":kup_time, "kdown":kdown_time}

    with open("../experiments/mcsplaininstances.txt", "r") as f:
        instances = [line.strip().split() for line in f]
    for instance in instances:
        runtime = runtimes[instance[0]]
        if runtime["kup"]<1000000 and runtime["kdown"]<1000000:
            analyse_instance(instance, runtime["kup"], runtime["kdown"])

if __name__=="__main__":
    go()
