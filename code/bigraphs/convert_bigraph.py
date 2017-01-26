import sys

class Graph(object):
    def __init__(self, adjmat, labels):
        self.adjmat = adjmat
        self.labels = labels

    def n(self):
        return len(self.adjmat)

    def __repr__(self):
        return ("\n".join("".join("1" if x else "0" for x in row) for row in self.adjmat)
                + "\n" + "\n".join(self.labels))

def parse_vertex(vertex):
    """Takes a string like '0, Area(2):1' and returns a pair like
       (0, 'Area(2):1')
    """
    tokens = vertex.split(",")
    return int(tokens[0]), tokens[1].strip()
    

def read_graph(filename):
    with open(filename, "r") as f:
        lines = [line for line in f.readlines()]

    l0 = lines[0]

    vertices = []

    vertex_str = ""
    paren_depth = 0
    for char in l0:
        if char == ")":
            paren_depth -= 1
            if paren_depth == 0:
                vertices.append(parse_vertex(vertex_str))
                vertex_str = ""
        if paren_depth >= 1:
            vertex_str += char
        if char == "(":
            paren_depth += 1

    vertex_idx_to_label = dict(vertices)

    nroots, nvertices, nsites = [int(x) for x in lines[1].strip().split()]

    adjmatstr = [line.strip() for line in lines[nroots+2:nroots+nvertices+2]]
    adjmat = [[ch=="1" for ch in line[:nvertices]] for line in adjmatstr]

    return Graph(adjmat, [vertex_idx_to_label[i] for i in range(nvertices)])

def write_word(f, word):
    f.write(bytearray([word&0xFF, (word&0xFF00)>>8]))

def write_graph(g, label_name_to_number, filename):
    with open(filename, "wb") as f:
        write_word(f, g.n())
        for label in g.labels:
            write_word(f, label_name_to_number[label])
        for row in g.adjmat:
            write_word(f, sum(row))
            for i in range(g.n()):
                if row[i]:
                    write_word(f, i)
                    write_word(f, 1)   # edge label

if __name__=="__main__":
    if len(sys.argv) != 5:
        print "Usage:", sys.argv[0], "IN-FILE-1 IN-FILE-2 OUT-FILE-1 OUT-FILE-2"
        sys.exit(1)
    g0 = read_graph(sys.argv[1])
    g1 = read_graph(sys.argv[2])
    print g0
    print g1

    all_labels = sorted(list(set(g0.labels + g1.labels)))
    label_name_to_number = {label: i for i, label in enumerate(all_labels)}

    write_graph(g0, label_name_to_number, sys.argv[3])
    write_graph(g1, label_name_to_number, sys.argv[4])

