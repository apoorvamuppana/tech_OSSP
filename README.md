# AEGIS-RTOS v4.2 — Autonomous Robotic Triage & Surgery Priority Operating System

**IEC 62304 Class C Deterministic Hybrid Real-Time Operating System (RTOS) Kernel & Interactive Telemetry Cockpit**

---

## 1. Executive Summary

**AEGIS-RTOS v4.2** is a real-time operating system scheduler architected for autonomous surgical robotics and acute patient triage. In robotic surgery, an OS kernel faces two seemingly contradictory life-critical requirements:

1. **Preemptive Priority Scheduling (`is_atomic = False`)**: When a sudden physiological crisis occurs (e.g., acute arterial hemorrhage or rapid blood gas desaturation), the scheduler must immediately preempt lower-priority background routines (such as routine telemetry logging) via a sub-millisecond hardware context switch.
2. **Non-Preemptive Priority Scheduling (`is_atomic = True`)**: When a surgical robotic actuator is actively executing an irreversible physical maneuver (such as a **5ms Femoral Artery Laser Incision**), interrupting the arm mid-cut would cause fatal vascular perforation. The task asserts an atomic hardware lock (`SCHED_ATOMIC_LOCK`) and **must run to completion**, queuing even a `P1 Code Red Ventricular Fibrillation` event until the critical section finishes.
3. **Bounded Priority Ceiling Dynamic Aging**: To guarantee that preemptible tasks never suffer indefinite starvation under heavy emergency load, every task waiting in the `READY` queue receives periodic priority boosts (`effective_priority = max(aging_floor, effective_priority - aging_step)`).

---

## 2. Repository Structure

| File / Path | Description |
| :--- | :--- |
| `index.html` | **Apple-Grade Interactive Web Application & Real-Time Kernel Telemetry Cockpit** (Self-contained HTML5/Tailwind/Chart.js/Canvas app ready for GitHub Pages deployment) |
| `aegis_rtos_simulator.py` | **Standalone Python 3 Terminal Simulator** (Zero comments, full Task Control Block register context saving, Gantt ribbon, and analytical proof report) |
| `aegis_rtos_simulator.cpp` | **Standalone C++17/C++20 Terminal Simulator** (Zero comments, deterministic priority queue & aging engine) |
| `logs/execution_log.txt` | **Verified Millisecond-Accurate Kernel Execution Log** proving Preemption, Non-Preemptive Atomic Deferral, and Anti-Starvation Aging |
| `docs/ARCHITECTURE.md` | **Complete OS Kernel System Architecture Specification** |
| `Makefile` | Build & execution automation for Python and C++ simulators |

---

## 3. Quick Start (Terminal & Web)

### Run the Python Terminal Simulation
```bash
python3 aegis_rtos_simulator.py
```

### Compile & Run the C++17 Terminal Simulation
```bash
g++ -std=c++17 -O3 aegis_rtos_simulator.cpp -o aegis_sim
./aegis_sim
```

### Launch the Interactive Web Cockpit
Open `index.html` directly in any modern browser or deploy the repository root to **GitHub Pages** / **Vercel** / **Netlify**.
