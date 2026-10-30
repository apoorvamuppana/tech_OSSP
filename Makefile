all: run-py build-cpp run-cpp

run-py:
	python3 aegis_rtos_simulator.py

build-cpp:
	g++ -std=c++17 -O3 aegis_rtos_simulator.cpp -o aegis_sim

run-cpp: build-cpp
	./aegis_sim

clean:
	rm -f aegis_sim
