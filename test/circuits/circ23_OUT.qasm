// i 0 1 2 3 4
// o 0
OPENQASM 2.0;
include "qelib1.inc";
qreg q[5];

gate p_x(prob) q {
 x q;
}

creg c0[1];
creg c1[1];
creg c2[1];
creg c3[1];
creg c4[1];
creg c[5];
h q[0];
rz(0) q[0];
ry(-6.28318530717959) q[0];
rz(0) q[0];
prob_x(0.5) q[0];
cx q[0], q[2];
x q[3];
