from qiskit import QuantumRegister, ClassicalRegister, QuantumCircuit
from qiskit.transpiler import PassManager
from Qcprop import Qcprop

q = QuantumRegister(7, 'q')
c = ClassicalRegister(7, 'c')

in_circ = QuantumCircuit(q, c)
in_circ.h(q[0])
in_circ.cx(q[1], q[2])
in_circ.cx(q[0], q[4])
in_circ.cx(q[2], q[3])
in_circ.cx(q[6], q[1])
in_circ.cx(q[5], q[0])
in_circ.rz(0.1, q[2])
in_circ.cx(q[5], q[0])

pm = PassManager()

pm.append([Qcprop(maxAmpls=1024, threshold=1e-8)])

print(in_circ.qasm())

out_circ = pm.run(in_circ)

print(out_circ.qasm())
