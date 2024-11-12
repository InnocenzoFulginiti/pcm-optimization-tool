import numpy as np
import argparse
import sys
from qiskit.circuit import QuantumRegister, ClassicalRegister, QuantumCircuit
from qiskit.circuit import Reset
from qiskit.circuit.library.standard_gates import (
    IGate,
    U1Gate,
    U2Gate,
    U3Gate,
    XGate,
    YGate,
    ZGate,
    HGate,
    SGate,
    SdgGate,
    TGate,
    TdgGate,
    RXGate,
    RYGate,
    RZGate,
    CXGate,
    CYGate,
    CZGate,
    CHGate,
    CRZGate,
    CU1Gate,
    CU3Gate,
    SwapGate,
    RZZGate,
    CCXGate,
    CSwapGate,
)
from qiskit.circuit.exceptions import CircuitError
import os
import subprocess

def my_random_circuit(
    num_qubits, depth, max_operands=3, measure=False, conditional=True, reset=False, seed=None, measurements_density=1
):
    """Generate random circuit of arbitrary size and form.

    This function will generate a random circuit by randomly selecting gates
    from the set of standard gates in :mod:`qiskit.extensions`. For example:

    .. jupyter-execute::

        from qiskit.circuit.random import random_circuit

        circ = random_circuit(2, 2, measure=True)
        circ.draw(output='mpl')

    Args:
        num_qubits (int): number of quantum wires
        depth (int): layers of operations (i.e. critical path length)
        max_operands (int): maximum operands of each gate (between 1 and 3)
        measure (bool): if True, measure all qubits at the end
        conditional (bool): if True, insert middle measurements and conditionals
        reset (bool): if True, insert middle resets
        seed (int): sets random seed (optional)

    Returns:
        QuantumCircuit: constructed circuit

    Raises:
        CircuitError: when invalid options given
    """
    if max_operands < 1 or max_operands > 3:
        raise CircuitError("max_operands must be between 1 and 3")

    one_q_ops = [
        #IGate,
        U1Gate,
        U2Gate,
        U3Gate,
        XGate,
        YGate,
        ZGate,
        HGate,
        SGate,
        SdgGate,
        TGate,
        TdgGate,
        RXGate,
        RYGate,
        RZGate,
    ]
    
    one_param = [
        U1Gate,
        RXGate,
        RYGate,
        RZGate,
        RZZGate,
        CU1Gate,
        CRZGate
    ]
    
    two_param = [
        U2Gate
    ]
    
    three_param = [
        U3Gate, 
        CU3Gate
    ]
    
    two_q_ops = [
        CXGate,
        CYGate,
        CZGate,
        CHGate,
        CRZGate,
        CU1Gate,
        CU3Gate,
        SwapGate,
        RZZGate
    ]
    
    three_q_ops = [CCXGate, CSwapGate]

    qr = QuantumRegister(num_qubits, "q")
    qc = QuantumCircuit(num_qubits)
    
    cr_i = []
    if conditional:
        for i in range(num_qubits):
            cr_i.append(ClassicalRegister(1, f"c{i}"))
            qc.add_register(cr_i[i])

    if measure or conditional:
        cr = ClassicalRegister(num_qubits, "c")
        qc.add_register(cr)

    if reset:
        one_q_ops += [Reset]

    if seed is None:
        seed = np.random.randint(0, np.iinfo(np.int32).max)
        
    rng = np.random.default_rng(seed)

    # apply arbitrary random operations at every depth
    for _ in range(depth):
        # choose either 1, 2, or 3 qubits for the operation
        remaining_qubits = list(range(num_qubits))
        rng.shuffle(remaining_qubits)
        while remaining_qubits:
            max_possible_operands = min(len(remaining_qubits), max_operands)
            num_operands = rng.choice(range(max_possible_operands)) + 1
            operands = [remaining_qubits.pop() for _ in range(num_operands)]
            if num_operands == 1:
                operation = rng.choice(one_q_ops)
            elif num_operands == 2:
                operation = rng.choice(two_q_ops)
            elif num_operands == 3:
                operation = rng.choice(three_q_ops)
            if operation in one_param:
                num_angles = 1
            elif operation in two_param:
                num_angles = 2
            elif operation in three_param:
                num_angles = 3
            else:
                num_angles = 0
            angles = [rng.uniform(0, 2 * np.pi) for x in range(num_angles)]
            register_operands = [qr[i] for i in operands]
            op = operation(*angles)

            # with some low probability, condition on classical bit values
            if conditional and rng.choice(range(measurements_density)) == 0 and len(remaining_qubits) > 0 and num_operands == 1:
                value = 1 # Condition value for the classical controlled operation
                
                measured_qubit = remaining_qubits.pop()
                
                qc.measure(qr[measured_qubit], cr_i[measured_qubit])
                
                op = op.c_if(cr_i[measured_qubit], 1)
                
            qc.append(op, register_operands)

    if measure:
        qc.measure(qr, cr)

    return qc

def generate_circuits(num_qubits, depth, density, num_circuits, measure, reset, seed, max_operands):
    dataset = []
    for i in range(num_circuits):
        dataset.append(my_random_circuit(num_qubits=num_qubits, depth=depth, max_operands=max_operands,
                    measure=measure, reset=reset, seed=seed, measurements_density=density).qasm())
    return dataset

# Function to count the total number of measurements and probabilisti cates in the files of a given folder
def count_measure_operations(folder):
    measure_count = 0
    prob_gate_count = 0
    num_circuits = 0
    
    for filename in os.listdir(folder):
        file_path = os.path.join(folder, filename)
        
        # Only process QASM files
        if not os.path.isfile(file_path) or not filename.endswith(".qasm"):
            continue
        
        with open(file_path, "r") as file:
            num_circuits += 1
            for line in file:
                measure_count += line.count("measure")
                prob_gate_count += line.count("prob_")
    
    return [measure_count, prob_gate_count, num_circuits]

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("num_circuits", type=int)
    parser.add_argument("num_qubits", type=int)
    parser.add_argument("depth", type=int)
    parser.add_argument("density", type=int)
    parser.add_argument("reset", type=bool)
    
    try:
        args = parser.parse_args()
    except SystemExit as e:
        # Customize the error message or usage here if needed
        print("\033[33mUsage: python mmr.py <num_circuits> <num_qubits> <depth> <density> <reset>")
        sys.exit(1)

    num_circuits = args.num_circuits
    num_qubits = args.num_qubits
    depth = args.depth
    density = args.density
    reset = args.reset

    max_operands = 3
    measure = True
    seed = None

    cpp_program = "../build/qcprop_main"

    input_folder = f"./input_circuits_n_{num_qubits}_dp_{depth}_dn_{density}_res_{reset}"
    output_folder = f"./output_circuits_n_{num_qubits}_dp_{depth}_dn_{density}_res_{reset}"

    os.makedirs(input_folder, exist_ok=True)
    os.makedirs(output_folder, exist_ok=True)

    generated_circuits = generate_circuits(num_qubits=num_qubits, depth=depth, num_circuits=num_circuits, 
        density=density, reset=reset, max_operands=max_operands, seed=seed, measure=measure)

    for i, circuit in enumerate(generated_circuits):
        filename = os.path.join(input_folder, f"c_{i}.qasm")
        with open(filename, 'w') as file:
            file.write(circuit)

    for input_filename in os.listdir(input_folder):
        input_file_path = os.path.join(input_folder, input_filename)
        
        # Don't consider folders
        if not os.path.isfile(input_file_path):
            continue
            
        if not input_file_path.endswith(".qasm"):
            continue
        
        # Generate the output filename and path
        output_filename = f"output_{input_filename}"
        output_file_path = os.path.join(output_folder, output_filename)
        
        # Run the C++ program with the input and output paths as arguments
        try:
            subprocess.run([cpp_program, input_file_path, output_file_path], check=True)
            print(f"\033[92mCorrectly processed {input_filename} -> {output_filename}\033[0m")
        except subprocess.CalledProcessError as e:
            print(f"\033[91mError processing {input_filename}: {e}\033[0m")
        
    [input_measurements, _, input_num_circuits] = count_measure_operations(input_folder)
    [output_measurements, output_prob_gates, output_num_circuits] = count_measure_operations(output_folder)

    print("###############################")
    print(f"Total measurements in input folder: {input_measurements}")
    print(f"Num circuits in input folder: {input_num_circuits}")
    print(f"Average measurement operations (inputs): {input_measurements / input_num_circuits}")
    print("-----------------")
    print(f"Total measurements in output folder: {output_measurements}")
    print(f"Num circuits in output folder: {output_num_circuits}")
    print(f"Average measurement operations (outputs): {output_measurements / output_num_circuits}")
    print("-----------------")

    if input_measurements != 0:
        print(f"Measurement reduction: {100 - output_measurements * 100 / input_measurements}%")
        print(f"Number of measuremnts removed: {input_measurements - output_measurements}")
        print(f"Total probabilistic gates in output folder: {output_prob_gates}")


if __name__ == "__main__":
    main()