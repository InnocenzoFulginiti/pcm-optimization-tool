from copy import copy

from qiskit.transpiler.basepasses import BasePass, TransformationPass
from qiskit.transpiler import Layout
from qiskit.circuit.library import SwapGate
from qiskit.dagcircuit import DAGCircuit
from qiskit import QuantumCircuit
from qiskit.converters import dag_to_circuit, circuit_to_dag


import sys
#Target qcprop_py needs to be built
sys.path.append("build/")
from qcprop_py import optimize

class Qcprop(TransformationPass):
    def __init__(self,
                 maxAmpls=1024,
                 threshold=1e-8):

        self.maxAmpls=maxAmpls
        self.threshold=threshold

        super().__init__()


    def run(self, dag):
        qc = dag_to_circuit(dag)
        circ_str = qc.qasm()
        
        new_circ_str = optimize(circ_str, self.maxAmpls, self.threshold)

        new_qc = QuantumCircuit.from_qasm_str(new_circ_str)
        
        return circuit_to_dag(new_qc)

