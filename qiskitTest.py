from qiskit import QuantumRegister, ClassicalRegister, QuantumCircuit
from qiskit.dagcircuit import DAGCircuit

import sys
sys.path.append("build/")
from qcprop_py import fileToFile

print(fileToFile(
    path_in="/u/halle/zuchna/home_at/master-thesis-jakob-wt22/test/circuits/mqt/ae_indep_qiskit_3.qasm",
    path_out="/u/halle/zuchna/home_at/master-thesis-jakob-wt22/benchmark-results/ae_indep_qiskit_3_out.qasm",
    maxAmpls=1024,
    threshold=0.1))

q = QuantumRegister(7, 'q')


in_circ = QuantumCircuit(q)
in_circ.h(q[0])
in_circ.cx(q[0], q[4])
in_circ.cx(q[2], q[3])
in_circ.cx(q[6], q[1])
in_circ.cx(q[5], q[0])
in_circ.rz(0.1, q[2])
in_circ.cx(q[5], q[0])

from qiskit.transpiler import PassManager
from qiskit.transpiler import CouplingMap
from qiskit import BasicAer
from setup import BasicSwap

pm = PassManager()
coupling = [[0, 1], [1, 2], [2, 3], [3, 4], [4, 5], [5, 6]]
coupling_map = CouplingMap(couplinglist=coupling)

pm.append([BasicSwap(coupling_map)])

out_circ = pm.run(in_circ)