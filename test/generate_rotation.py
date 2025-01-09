from qiskit import QuantumCircuit
import numpy as np
from qiskit.quantum_info import Statevector
from qiskit.circuit.library import StatePreparation
from qiskit import transpile
import sys
import ast

import re

def parse_complex_list(data):
    # Regular expression to capture complex numbers in the format "a+bi"
    pattern = r"([-+]?\d*\.?\d+)([-+]\d*\.?\d+)i"
    matches = re.findall(pattern, data)
    
    # Convert matched pairs into Python complex numbers
    complex_list = [complex(float(real), float(imag)) for real, imag in matches]
    return complex_list


def prepare_state(state_vector):
    # Ensure the input state is normalized
    state_vector = state_vector / np.linalg.norm(state_vector)
    
    # Get the number of qubits needed (log2 of the length of state_vector)
    n = int(np.log2(len(state_vector)))
    
    # Create a QuantumCircuit with n qubits
    qc = QuantumCircuit(n)
    
    # Use StatePreparation to prepare the state on the quantum circuit
    state_preparation = StatePreparation(state_vector)
    
    # Append the state preparation to the quantum circuit
    qc.append(state_preparation, range(n))
    
    # Decompose the state preparation into individual gates
    qc = transpile(qc, basis_gates=['h', 'cx', 'rz', 'ry'])
    
    return qc


# Retrieve arguments from the command line
qubits_ind = ast.literal_eval(sys.argv[1])
state_vector = parse_complex_list(sys.argv[2])

# Prepare the state on the quantum circuit
qc = prepare_state(state_vector)


vector_representation = []
for instruction in qc.data:
    instr = instruction.operation
    qubits = instruction.qubits
    clbits = instruction.clbits
    qubit_indices = [qubits_ind[qc.find_bit(q).index] for q in qubits]
    params = instr.params
    name = instruction.operation.name
    vector_representation.append((name, qubit_indices, params))

with open("rotation.txt", "w") as file:
    for op in vector_representation:
        file.write(f"{op}\n")