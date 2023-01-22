OPENQASM 2.0;
include "qelib1.inc";
creg q[3];
qreg q[3];
h q[0];
x q[0];
h q[0];
measure q -> c;
