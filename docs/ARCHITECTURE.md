# AEGIS-RTOS v4.2 System Architecture & Scheduling Formal Specification

## 1. Kernel State Machine & Task Control Block (TCB)

Each clinical process in **AEGIS-RTOS v4.2** is encapsulated in a deterministic `TaskControlBlock` containing:

- **Identity & Clinical Subsystem**: `pid`, `name`, `subsystem` (`BIOSENSOR_BUS`, `HEMO_ANALYZER`, `ACTUATOR_ARM_ALPHA`, `CARDIAC_LIFE_SUPPORT`, `NEURO_INFUSION_PUMP`, `IMAGING_ARRAY`, `ACTUATOR_ARM_BETA`)
- **Timing Parameters**: `arrival_tick`, `burst_ticks`, `remaining_ticks`, `wait_ticks`, `total_wait_ticks`, `start_tick`, `completion_tick`
- **Priority & Mode Flags**:
  - `base_priority` (1 = highest urgency, 99 = lowest urgency)
  - `effective_priority` (dynamically promoted via aging)
  - `is_atomic` (`True` = Non-Preemptive Hardware Lock Engaged; `False` = Standard Preemptible)
- **Bounded Aging Parameters**: `aging_threshold`, `aging_step`, `aging_floor`
- **Hardware Context Register File (`RegisterFile`)**:
  - `PC` (Program Counter), `SP` (Stack Pointer), `LR` (Link Register), `PSR` (Program Status Register)
  - `arm_theta1`, `arm_theta2` (Robotic kinematic joint angles in degrees), `laser_mw` (Optical laser power in mW)

---

## 2. Deterministic 4-Tuple Ready Queue Comparator

At every 1ms tick, the kernel selects the highest-priority task from the `READY` queue by minimizing the lexicographical 4-tuple:

```
Key(T) = (T.effective_priority, -T.wait_ticks, T.arrival_tick, T.pid)
```

1. **Lowest `effective_priority` number wins** (`P1` beats `P12`).
2. **Tie-Breaker 1**: Longest current wait time (`wait_ticks`).
3. **Tie-Breaker 2**: Earliest arrival time (`arrival_tick` / FCFS).
4. **Tie-Breaker 3**: Lowest Process ID (`pid`).

---

## 3. Millisecond Proof Walkthrough (T = 0ms to T = 26ms)

1. **`T = 0ms` (Initial Dispatch)**: `PID-101` (*Routine Telemetry & Vitals Logging*, `P85`, Preemptive) arrives and is dispatched onto the CPU.
2. **`T = 1ms` (Preemptive Context Switch #1)**: `PID-102` (*Autonomous Blood Gas Spectrophotometry*, `P55`, Preemptive) arrives. Because `P55 < P85` and `PID-101` is preemptible, the kernel saves `PID-101`'s registers (`PC=0x8065040, SP=0x2001ffe0, Theta=(11.13, 29.07)`) and immediately switches to `PID-102`.
3. **`T = 2ms` (Preemptive Context Switch #2 + Atomic Lock Acquisition)**: `PID-103` (*Micro-Robotic Femoral Artery Laser Incision*, `P22`, **Non-Preemptive Atomic**) arrives. Because `PID-102` is preemptible and `P22 < P55`, `PID-103` preempts `PID-102` and engages the **Non-Preemptive Robotic Actuator Lock** for its 5ms critical incision.
4. **`T = 4ms..6ms` (Non-Preemptive Atomic Lock Enforcement)**: At `T = 4ms`, `PID-104` (**CODE RED: Ventricular Fibrillation Defibrillation**, `P1`, Atomic) arrives. Even though `P1 < P22`, `PID-103` is mid-incision (`is_atomic = True`). The kernel blocks preemption for 3 ticks (`T=4ms, 5ms, 6ms`), logging `ATOMIC_LOCK_HOLD` while queuing `PID-104` at the head of the Ready Queue.
5. **`T = 7ms` (Immediate Emergency Dispatch on Lock Release)**: `PID-103` finishes its 5ms incision and releases the actuator lock. `PID-104` (`P1`) immediately dispatches and runs atomically from `T = 7ms` to `T = 10ms`.
6. **`T = 2ms..11ms` (Anti-Starvation Dynamic Aging & Preemptive Switch #3)**: While waiting in the Ready Queue, `PID-101` (Base `P85`) ages every 2ms by `-15` down to its safety ceiling `P12` (`P85 -> P70 -> P55 -> P40 -> P25 -> P12`). At `T = 10ms`, `PID-101` reaches `P12`, overtaking `PID-102` (`P14`), `PID-105` (`P20`), and `PID-106` (`P32`). At `T = 11ms`, `PID-101` (`P12`) preempts `PID-102` (`P14`)!
7. **`T = 12ms` (Preemptive Context Switch #4)**: `PID-107` (*Acute Hemorrhage Electro-Cauterization Clamp*, `P4`, Atomic) arrives and immediately preempts `PID-101` (`P12`) because `PID-101` is preemptible and `P4 < P12`. `PID-107` completes at `T = 14ms`, after which `PID-101`, `PID-102`, `PID-106`, and `PID-105` finish cleanly by `T = 26ms`.
