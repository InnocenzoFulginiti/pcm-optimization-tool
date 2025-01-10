import numpy as np
import qiskit.qasm2
import qiskit.qasm3

from qiskit.transpiler.passes import RemoveResetInZeroState, RemoveFinalReset, RemoveFinalMeasurements, ResetAfterMeasureSimplification
from qiskit.transpiler import PassManager
from qiskit.transpiler.preset_passmanagers import generate_preset_pass_manager
from qiskit_ibm_runtime import QiskitRuntimeService

from distutils.util import strtobool
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
    num_qubits, depth, max_operands=3, measure=False, conditional=True, reset=False, seed=None, measurements_density=8, reset_density=16
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

    # if reset:
    #     one_q_ops += [Reset]

    if seed is None:
        seed = np.random.randint(0, np.iinfo(np.int32).max)
        
    rng = np.random.default_rng(seed)

    # apply arbitrary random operations at every depth
    for _ in range(depth):
        # choose either 1, 2, or 3 qubits for the operation
        remaining_qubits = list(range(num_qubits))
        rng.shuffle(remaining_qubits)
        while remaining_qubits:
            if reset and rng.choice(range(reset_density)) == 0:
                operands = [remaining_qubits.pop()]
                angles = []
                register_operands = [qr[i] for i in operands]
                op = operation(*angles)
            else:
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
                    
                    op = op.c_if(cr_i[measured_qubit], value)
                
            qc.append(op, register_operands)

    if measure:
        qc.measure(qr, cr)

    return qc

def generate_circuits(num_qubits, depth, measurement_density, reset_density, num_circuits, measure, reset, seed, max_operands, pass_manager=None):
    dataset = []
    dataset_post = []
    post_pass_manager = PassManager()
    post_pass_manager.append(RemoveResetInZeroState())
    # post_pass_manager.append(ResetAfterMeasureSimplification())

    for i in range(num_circuits):
        qc = my_random_circuit(num_qubits=num_qubits, depth=depth, max_operands=max_operands, measure=measure, reset=reset, seed=seed, measurements_density=measurement_density, reset_density=reset_density)
        if pass_manager != None:
            qc = pass_manager.run(qc)

        qc_post = post_pass_manager.run(qc)
        print(f"Generated circuit #{i}")

        dataset.append(qiskit.qasm2.dumps(qc))
        dataset_post.append(qiskit.qasm2.dumps(qc_post))
        
    return [dataset, dataset_post]

# Function to count the total number of measurements and probabilisti cates in the files of a given folder
def count_measure_reset_operations(folder, delete_file=False):
    measure_count = 0
    prob_gate_count = 0
    num_circuits = 0
    reset_count = 0
    
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
                reset_count += line.count("reset")

        if delete_file:
            os.remove(file_path)
    
    return [measure_count, prob_gate_count, reset_count, num_circuits]

def main():
    # Parse command line arguments
    parser = argparse.ArgumentParser()
    parser.add_argument("num_circuits", type=int)
    parser.add_argument("num_qubits", type=int)
    parser.add_argument("depth", type=int)
    parser.add_argument("meas_density", type=int)
    parser.add_argument("reset", type=lambda x: bool(strtobool(x)))
    parser.add_argument("res_density", type=int)
    try:
        args = parser.parse_args()
    except SystemExit as e:
        # Customize the error message or usage here if needed
        print("\033[33mUsage: python mmr.py <num_circuits> <num_qubits> <depth> <density> <reset>")
        sys.exit(1)

    num_circuits = args.num_circuits
    num_qubits = args.num_qubits
    depth = args.depth
    meas_density = args.meas_density
    reset = args.reset
    res_density = args.res_density

    max_operands = 3
    measure = False
    seed = None

    # Pass manager initialization
    pass_manager = None
    use_pass_manager = False
    if use_pass_manager:
        backend_name = "ibm_sherbrooke"
        service = QiskitRuntimeService()
        backend = service.backend(backend_name)
        pass_manager = generate_preset_pass_manager(backend=backend, optimization_level=3)
    if pass_manager is not None:
        print("Pass manager initialized")

    # Flag to generate another directory where circuits are optimized using qiskit passes:
    # RemoveResetInZeroState, RemoveFinalReset, RemoveFinalMeasurements
    generate_qiskit_opt = True

    cpp_program = "../build/qcprop_main"
    
    non_opt_folder = f"./non_opt_circuits_n_{num_qubits}_dp_{depth}_dn_{meas_density}_res_{reset}_res_dn_{res_density}"
    mmr_opt_folder = f"./mmr_opt_circuits_n_{num_qubits}_dp_{depth}_dn_{meas_density}_res_{reset}_res_dn_{res_density}"

    os.makedirs(non_opt_folder, exist_ok=True)
    os.makedirs(mmr_opt_folder, exist_ok=True)

    [generated_circuits_non_opt, generated_circuits_qiskit_opt] = generate_circuits(num_qubits=num_qubits, depth=depth, num_circuits=num_circuits, 
        meas_density=meas_density, res_density=res_density, reset=reset, max_operands=max_operands, seed=seed, measure=measure, pass_manager=pass_manager)

    folder_qiskit_opt = ""
    if generate_qiskit_opt:
        folder_qiskit_opt = f"./qiskit_opt_circuits_n_{num_qubits}_dp_{depth}_dn_{meas_density}_res_{reset}_res_dn_{res_density}"
        os.makedirs(folder_qiskit_opt, exist_ok=True)
        for i, circuit in enumerate(generated_circuits_qiskit_opt):
            filename = os.path.join(folder_qiskit_opt, f"qiskit_opt_{i}.qasm")
            with open(filename, 'w') as file:
                file.write(circuit + "\n")

    for i, circuit in enumerate(generated_circuits_non_opt):
        filename = os.path.join(non_opt_folder, f"non_opt_{i}.qasm")
        with open(filename, 'w') as file:
            file.write(circuit + "\n")

    for input_filename in os.listdir(non_opt_folder):
        input_file_path = os.path.join(non_opt_folder, input_filename)
        
        # Don't consider folders
        if not os.path.isfile(input_file_path):
            continue
            
        if not input_file_path.endswith(".qasm"):
            continue
        
        # Generate the output filename and path
        output_filename = f"mmr_opt_{input_filename}"
        output_file_path = os.path.join(mmr_opt_folder, output_filename)
        
        # Run the C++ program with the input and output paths as arguments
        try:
            subprocess.run([cpp_program, input_file_path, output_file_path], check=True)
            print(f"\033[92mCorrectly processed {input_filename} -> {output_filename}\033[0m")
        except subprocess.CalledProcessError as e:
            print(f"\033[91mError processing {input_filename}: {e}\033[0m")
        
    [non_opt_measurements, _, non_opt_resets, non_opt_num_circuits] = count_measure_reset_operations(non_opt_folder)
    [mmr_opt_measurements, mmr_opt_prob_gates, mmr_opt_resets, mmr_opt_num_circuits] = count_measure_reset_operations(mmr_opt_folder)

    qiskit_opt_measurements = -1
    qiskit_opt_resets = -1
    qiskit_opt_num_circuits = -1

    if generate_qiskit_opt:
        [qiskit_opt_measurements, _, qiskit_opt_resets, qiskit_opt_num_circuits] = count_measure_reset_operations(folder_qiskit_opt)
    
    report_file_name = f"report_n_{num_qubits}_dp_{depth}_dn_{meas_density}_res_{reset}_res_dn_{res_density}.txt"
    report_file_path = os.path.join(non_opt_folder, report_file_name)

    with open(report_file_path, "w") as file:
        file.write(f"Num circuits in non-opt folder: {non_opt_num_circuits}\n")
        file.write(f"Total measurements: {non_opt_measurements}\n")
        file.write(f"Total resets: {non_opt_resets}\n")
        file.write(f"Average measurement operations: {non_opt_measurements / non_opt_num_circuits}\n")
        file.write("-------\n")
        file.write(f"Num circuits in mmr-opt folder: {mmr_opt_num_circuits}\n")
        file.write(f"Total measurements: {mmr_opt_measurements}\n")
        file.write(f"Total resets: {mmr_opt_resets}\n")
        file.write(f"Average measurement operations: {mmr_opt_measurements / mmr_opt_num_circuits}\n")
        file.write(f"Probabilistic gates: {mmr_opt_prob_gates}\n")
        file.write("-------\n")
        if generate_qiskit_opt:
            file.write(f"Num circuits in qiskit-opt folder: {qiskit_opt_num_circuits}\n")
            file.write(f"Total measurements: {qiskit_opt_measurements}\n")
            file.write(f"Total resets: {qiskit_opt_resets}\n")
            file.write(f"Average measurement operations: {qiskit_opt_measurements / qiskit_opt_num_circuits}\n")
            file.write("-------\n")
            file.write("-------\n")
            file.write("Comparison non-optimized <-> qiskit passes\n")
            if non_opt_measurements != 0:
                file.write("-------\n")
                file.write(f"Measurement reduction: {100 - qiskit_opt_measurements * 100 / non_opt_measurements}%\n")
                file.write(f"Number of measurements removed: {non_opt_measurements - qiskit_opt_measurements}\n") 
            if non_opt_resets != 0:
                file.write("-------\n")
                file.write(f"Reset reduction: {100 - qiskit_opt_resets * 100 / non_opt_resets}%\n")
                file.write(f"Number of resets removed: {non_opt_resets - qiskit_opt_resets}\n")
        
        file.write("-------\n")
        file.write("-------\n")
        file.write("Comparison non-optimized <-> mmr\n")
        if non_opt_measurements != 0:
            file.write("-------\n")
            file.write(f"Measurement reduction: {100 - mmr_opt_measurements * 100 / non_opt_measurements}%\n")
            file.write(f"Number of measurements removed: {non_opt_measurements - mmr_opt_measurements}\n") 
        if non_opt_resets != 0:
            file.write("-------\n")
            file.write(f"Reset reduction: {100 - mmr_opt_resets * 100 / non_opt_resets}%\n")
            file.write(f"Number of resets removed: {non_opt_resets - mmr_opt_resets}\n")


if __name__ == "__main__":
    main()