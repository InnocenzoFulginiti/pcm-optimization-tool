// i 0 1
// o 0
OPENQASM 2.0;
include "qelib1.inc";
qreg q[2];
creg c0[1];
creg c1[1];
h q[1];
z q[1];
h q[0];
cx q[0], q[1];
h q[0];
x q[0];
h q[0];
cx q[0], q[1];
h q[0];
x q[0];
h q[0];
cx q[0], q[1];
h q[0];
x q[0];
h q[0];
cx q[0], q[1];
h q[0];
x q[0];
h q[0];
cx q[0], q[1];
h q[0];
x q[0];
h q[0];
cx q[0], q[1];
h q[0];
x q[0];
h q[1];
