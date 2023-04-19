#!/bin/bash

ROOT_DIR="$PWD"

echo "$ROOT_DIR/bench.zip"

curl 'https://www.cda.cit.tum.de/mqtbench/download' \
        -X 'POST' -H 'Referer: https://www.cda.cit.tum.de/mqtbench/' \
        -H 'Origin: https://www.cda.cit.tum.de' \
        -H 'Content-Type: application/x-www-form-urlencoded' \
        -H 'Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8' \
        -H 'User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/16.3 Safari/605.1.15' \
        --data 'all_benchmarks=true&selectBench_1=Amplitude+Estimation+%28AE%29&selectBench_2=Deutsch-Jozsa&selectBench_3=Graph+State&selectBench_4=GHZ+State&selectBench_5=Grover%27s+%28no+ancilla%29&selectBench_6=Grover%27s+%28v-chain%29&selectBench_7=Portfolio+Optimization+with+QAOA&selectBench_8=Portfolio+Optimization+with+VQE&selectBench_9=Quantum+Approximation+Optimization+Algorithm+%28QAOA%29&selectBench_10=Quantum+Fourier+Transformation+%28QFT%29&selectBench_11=QFT+Entangled&selectBench_12=Quantum+Generative+Adversarial+Network&selectBench_13=Quantum+Phase+Estimation+%28QPE%29+exact&selectBench_14=Quantum+Phase+Estimation+%28QPE%29+inexact&selectBench_15=Quantum+Walk+%28no+ancilla%29&selectBench_16=Quantum+Walk+%28v-chain%29&selectBench_17=Variational+Quantum+Eigensolver+%28VQE%29&selectBench_18=Efficient+SU2+ansatz+with+Random+Parameters&selectBench_19=Real+Amplitudes+ansatz+with+Random+Parameters&selectBench_20=Two+Local+ansatz+with+Random+Parameters&selectBench_21=W-State&minQubits=&maxQubits=&selectBench_22=Ground+State&selectBench_23=HHL&selectBench_24=Pricing+Call+Option&selectBench_25=Pricing+Put+Option&selectBench_26=Routing&selectBench_27=Shor%27s&selectBench_28=Travelling+Salesman&indep_qiskit_compiler=true' \
        --output "$ROOT_DIR/mqt.zip"

unzip -o "$ROOT_DIR/mqt.zip" -d "$ROOT_DIR/mqt"
rm "$ROOT_DIR/mqt.zip"