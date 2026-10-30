#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <cmath>

struct RegisterFile {
    unsigned int pc;
    unsigned int sp;
    unsigned int lr;
    unsigned int psr;
    double arm_theta1;
    double arm_theta2;
    double laser_mw;
};

struct TaskControlBlock {
    int pid;
    std::string name;
    std::string subsystem;
    int arrival_tick;
    int burst_ticks;
    int base_priority;
    int effective_priority;
    bool is_atomic;
    int aging_threshold;
    int aging_step;
    int aging_floor;
    int remaining_ticks;
    int wait_ticks;
    int total_wait_ticks;
    int start_tick;
    int completion_tick;
    int preemption_count;
    int aging_promotions;
    std::string state;
    RegisterFile registers;

    TaskControlBlock(int p, std::string n, std::string sub, int arr, int burst, int prio, bool atomic, int age_thresh, int age_step, int age_fl)
        : pid(p), name(n), subsystem(sub), arrival_tick(arr), burst_ticks(burst),
          base_priority(prio), effective_priority(prio), is_atomic(atomic),
          aging_threshold(age_thresh), aging_step(age_step), aging_floor(age_fl),
          remaining_ticks(burst), wait_ticks(0), total_wait_ticks(0),
          start_tick(-1), completion_tick(-1), preemption_count(0),
          aging_promotions(0), state("NEW") {
        registers = {0x08000000u + static_cast<unsigned int>(p * 0x1000), 0x20020000u, 0xFFFFFFFFu, 0x01000000u, 0.0, 0.0, 0.0};
    }
};

struct GanttSlot {
    int tick;
    int pid;
    std::string name;
    int priority;
    int base_priority;
    bool is_atomic;
    std::string mode;
};

class AegisSurgicalRTOSKernel {
public:
    int tick = 0;
    std::vector<TaskControlBlock> tasks;
    std::vector<int> ready_indices;
    std::vector<int> completed_indices;
    int running_index = -1;
    int context_switches = 0;
    int preemptions_performed = 0;
    int atomic_deferrals = 0;
    int aging_events = 0;
    std::vector<GanttSlot> gantt_timeline;
    std::vector<std::string> event_log;

    void register_task(const TaskControlBlock& tcb) {
        tasks.push_back(tcb);
    }

    void log_event(const std::string& tag, const std::string& message) {
        std::ostringstream oss;
        oss << "[T=" << std::setw(2) << std::setfill('0') << tick << "ms] ["
            << std::left << std::setw(18) << std::setfill(' ') << tag << "] " << message;
        event_log.push_back(oss.str());
    }

    int select_highest_priority_ready_task() {
        if (ready_indices.empty()) return -1;
        int best_idx = ready_indices[0];
        for (size_t i = 1; i < ready_indices.size(); ++i) {
            int idx = ready_indices[i];
            const auto& a = tasks[idx];
            const auto& b = tasks[best_idx];
            if (a.effective_priority != b.effective_priority) {
                if (a.effective_priority < b.effective_priority) best_idx = idx;
            } else if (a.wait_ticks != b.wait_ticks) {
                if (a.wait_ticks > b.wait_ticks) best_idx = idx;
            } else if (a.arrival_tick != b.arrival_tick) {
                if (a.arrival_tick < b.arrival_tick) best_idx = idx;
            } else if (a.pid < b.pid) {
                best_idx = idx;
            }
        }
        return best_idx;
    }

    void remove_from_ready(int task_idx) {
        ready_indices.erase(std::remove(ready_indices.begin(), ready_indices.end(), task_idx), ready_indices.end());
    }

    void save_context(TaskControlBlock& tcb) {
        tcb.registers.pc += 0x40;
        tcb.registers.sp -= 0x20;
        tcb.registers.arm_theta1 = std::round(std::sin(tick * 0.25) * 4500.0) / 100.0;
        tcb.registers.arm_theta2 = std::round(std::cos(tick * 0.25) * 3000.0) / 100.0;
        tcb.registers.laser_mw = tcb.is_atomic ? 450.0 : 15.0;
    }

    void apply_aging() {
        for (int idx : ready_indices) {
            TaskControlBlock& tcb = tasks[idx];
            tcb.wait_ticks += 1;
            tcb.total_wait_ticks += 1;
            if (tcb.wait_ticks > 0 && tcb.wait_ticks % tcb.aging_threshold == 0) {
                int old_prio = tcb.effective_priority;
                int new_prio = std::max(tcb.aging_floor, tcb.effective_priority - tcb.aging_step);
                if (new_prio < old_prio) {
                    tcb.effective_priority = new_prio;
                    tcb.aging_promotions += 1;
                    aging_events += 1;
                    std::ostringstream msg;
                    msg << "PID-" << tcb.pid << " (" << tcb.name << ") waited " << tcb.wait_ticks
                        << "ms -> Priority promoted P" << old_prio << " => P" << new_prio << " (Starvation Prevention)";
                    log_event("AGING_BOOST", msg.str());
                }
            }
        }
    }

    void admit_new_arrivals() {
        for (size_t i = 0; i < tasks.size(); ++i) {
            TaskControlBlock& tcb = tasks[i];
            if (tcb.state == "NEW" && tcb.arrival_tick == tick) {
                tcb.state = "READY";
                ready_indices.push_back(static_cast<int>(i));
                std::string mode_str = tcb.is_atomic ? "NON-PREEMPTIVE ATOMIC" : "PREEMPTIBLE";
                std::ostringstream msg;
                msg << "PID-" << tcb.pid << " (" << tcb.name << ") arrived | Base: P" << tcb.base_priority
                    << " | Burst: " << tcb.burst_ticks << "ms | Mode: " << mode_str;
                log_event("TASK_ARRIVAL", msg.str());
            }
        }
    }

    void evaluate_scheduler() {
        int candidate_idx = select_highest_priority_ready_task();
        if (running_index == -1) {
            if (candidate_idx != -1) {
                remove_from_ready(candidate_idx);
                TaskControlBlock& candidate = tasks[candidate_idx];
                candidate.state = "RUNNING";
                candidate.wait_ticks = 0;
                if (candidate.start_tick == -1) candidate.start_tick = tick;
                running_index = candidate_idx;
                context_switches += 1;
                std::string lock_banner = candidate.is_atomic ? " [ATOMIC HARDWARE LOCK ENGAGED]" : " [PREEMPTIVE MODE]";
                std::ostringstream msg;
                msg << "Dispatched PID-" << candidate.pid << " (" << candidate.name << ") at Effective P"
                    << candidate.effective_priority << " (Rem: " << candidate.remaining_ticks << "ms)" << lock_banner;
                log_event("DISPATCH", msg.str());
            }
        } else {
            if (candidate_idx != -1 && tasks[candidate_idx].effective_priority < tasks[running_index].effective_priority) {
                TaskControlBlock& running = tasks[running_index];
                TaskControlBlock& candidate = tasks[candidate_idx];
                if (running.is_atomic) {
                    atomic_deferrals += 1;
                    std::ostringstream msg;
                    msg << "CRITICAL BLOCK: PID-" << candidate.pid << " (" << candidate.name << ", P" << candidate.effective_priority
                        << ") CANNOT preempt Atomic PID-" << running.pid << " (" << running.name << ", P" << running.effective_priority
                        << ")! " << running.remaining_ticks << "ms remaining in critical section.";
                    log_event("ATOMIC_LOCK_HOLD", msg.str());
                } else {
                    save_context(running);
                    running.state = "READY";
                    running.preemption_count += 1;
                    ready_indices.push_back(running_index);
                    remove_from_ready(candidate_idx);
                    candidate.state = "RUNNING";
                    candidate.wait_ticks = 0;
                    if (candidate.start_tick == -1) candidate.start_tick = tick;
                    running_index = candidate_idx;
                    context_switches += 1;
                    preemptions_performed += 1;
                    std::ostringstream msg;
                    msg << "HARDWARE INTERRUPT! PID-" << candidate.pid << " (" << candidate.name << ", P" << candidate.effective_priority
                        << ") PREEMPTED PID-" << running.pid << " (" << running.name << ", P" << running.effective_priority
                        << ") | Saved PC=0x" << std::hex << running.registers.pc << " SP=0x" << running.registers.sp << std::dec
                        << " Theta=(" << running.registers.arm_theta1 << "," << running.registers.arm_theta2 << ")";
                    log_event("PREEMPT_SWITCH", msg.str());
                    if (candidate.is_atomic) {
                        std::ostringstream lock_msg;
                        lock_msg << "PID-" << candidate.pid << " (" << candidate.name
                                 << ") acquired Non-Preemptive Robotic Actuator Lock for " << candidate.remaining_ticks << "ms.";
                        log_event("ATOMIC_LOCK_ACQ", lock_msg.str());
                    }
                }
            }
        }
    }

    void step() {
        admit_new_arrivals();
        evaluate_scheduler();
        if (running_index != -1) {
            TaskControlBlock& active = tasks[running_index];
            std::string sched_mode = active.is_atomic ? "NON-PREEMPTIVE_ATOMIC" : "PREEMPTIVE";
            gantt_timeline.push_back({tick, active.pid, active.name, active.effective_priority, active.base_priority, active.is_atomic, sched_mode});
            std::ostringstream msg;
            msg << "Executing PID-" << active.pid << " (" << active.name << ") [Eff:P" << active.effective_priority
                << " Base:P" << active.base_priority << "] | Mode: " << sched_mode << " | Progress: "
                << (active.burst_ticks - active.remaining_ticks + 1) << "/" << active.burst_ticks << "ms";
            log_event("CPU_EXEC", msg.str());
            active.remaining_ticks -= 1;
            apply_aging();
            if (active.remaining_ticks == 0) {
                active.state = "TERMINATED";
                active.completion_tick = tick + 1;
                completed_indices.push_back(running_index);
                std::string release_note = active.is_atomic ? " | Released Non-Preemptive Actuator Lock." : "";
                std::ostringstream comp_msg;
                comp_msg << "PID-" << active.pid << " (" << active.name << ") COMPLETED at T=" << (tick + 1)
                         << "ms | Turnaround: " << (active.completion_tick - active.arrival_tick)
                         << "ms | Wait: " << active.total_wait_ticks << "ms" << release_note;
                log_event("TASK_COMPLETE", comp_msg.str());
                running_index = -1;
            }
        } else {
            gantt_timeline.push_back({tick, 0, "IDLE", 99, 99, false, "IDLE"});
            apply_aging();
        }
        tick += 1;
    }

    void run_until_complete() {
        while (completed_indices.size() < tasks.size() && tick < 200) {
            step();
        }
    }
};

int main() {
    AegisSurgicalRTOSKernel kernel;
    kernel.register_task(TaskControlBlock(101, "Routine Telemetry & Vitals Logging", "BIOSENSOR_BUS", 0, 6, 85, false, 2, 15, 12));
    kernel.register_task(TaskControlBlock(102, "Autonomous Blood Gas Spectrophotometry", "HEMO_ANALYZER", 1, 4, 55, false, 2, 12, 14));
    kernel.register_task(TaskControlBlock(103, "Micro-Robotic Femoral Artery Laser Incision", "ACTUATOR_ARM_ALPHA", 2, 5, 22, true, 2, 8, 10));
    kernel.register_task(TaskControlBlock(104, "CODE RED: Ventricular Fibrillation Defibrillation", "CARDIAC_LIFE_SUPPORT", 4, 3, 1, true, 1, 2, 1));
    kernel.register_task(TaskControlBlock(105, "Intracranial Pressure Bolus Titration", "NEURO_INFUSION_PUMP", 5, 3, 38, false, 2, 6, 18));
    kernel.register_task(TaskControlBlock(106, "Optical Coherence Tomography Margin Scan", "IMAGING_ARRAY", 11, 3, 32, false, 2, 6, 16));
    kernel.register_task(TaskControlBlock(107, "Acute Hemorrhage Electro-CauterizationClamp", "ACTUATOR_ARM_BETA", 12, 2, 4, true, 1, 2, 1));

    kernel.run_until_complete();

    std::string sep(118, '=');
    std::string subsep(118, '-');
    std::cout << sep << "\n";
    std::cout << "AEGIS-RTOS v4.2 (C++20 KERNEL BUILD) // AUTONOMOUS ROBOTIC TRIAGE & SURGERY HYBRID PRIORITY SCHEDULER\n";
    std::cout << sep << "\n\n";
    for (const auto& line : kernel.event_log) {
        std::cout << line << "\n";
    }
    std::cout << subsep << "\n";
    std::cout << "Total Simulation Horizon : " << kernel.tick << " ms | Context Switches: " << kernel.context_switches
              << " | Preemptions: " << kernel.preemptions_performed
              << " | Atomic Deferrals: " << kernel.atomic_deferrals
              << " | Aging Promotions: " << kernel.aging_events << "\n";
    std::cout << sep << "\n";
    return 0;
}
