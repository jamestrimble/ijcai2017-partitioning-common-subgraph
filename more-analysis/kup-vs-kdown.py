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

def analyse_instance(instance, runtime_kup, runtime_kdown, dataset_name):
    n0 = read_n(instance[1])
    n1 = read_n(instance[2])
    solsize_kup = read_sol_size(
            "../experiments/gpgnode-results/{}/james-cpp-max/{}.out".format(
                dataset_name, instance[0]))
    solsize_kdown = read_sol_size(
            "../experiments/gpgnode-results/{}/james-cpp-max-down/{}.out".format(
                dataset_name, instance[0]))
    if solsize_kup != solsize_kdown:
        sys.stderr.write("Solution sizes differ between runs!\n")
        sys.exit(1)
    print instance[0], n0, n1, runtime_kup, runtime_kdown, solsize_kup, \
            float(solsize_kup) / n0
    
def print_header():
    print "instance", \
            "n_pattern", \
            "n_target", \
            "runtime_kup", \
            "runtime_kdown", \
            "solsize", \
            "solsize_over_n_pattern"

def go(dataset_name, kup_time_column, kdown_time_column):
    print_header()
    runtimes = {}
    with open("../experiments/gpgnode-results/{}/runtimes.data".format(dataset_name), "r") as f:
        lines = [line.strip().split() for line in f]
    for tokens in lines[1:]:
        kup_time = int(tokens[kup_time_column])
        kdown_time = int(tokens[kdown_time_column])
        runtimes[tokens[0]] = {"kup":kup_time, "kdown":kdown_time}

    with open("../experiments/{}instances.txt".format(dataset_name), "r") as f:
        instances = [line.strip().split() for line in f]
    for instance in instances:
        runtime = runtimes[instance[0]]
        if runtime["kup"]<1000000 and runtime["kdown"]<1000000:
            analyse_instance(instance, runtime["kup"], runtime["kdown"], dataset_name)

if __name__=="__main__":
    go(sys.argv[1], int(sys.argv[2]), int(sys.argv[3]))
