#ifndef __TASK_H__
#define __TASK_H__

#include <functional>
#include <deque>
#include <mutex>
#ifndef LONESTAR
#include <atomic>
#else
#include <cstdatomic>
#endif

enum TaskStatus {
    WAITING,
    READY,
    SCHEDULED,
    COMPLETE
};

class Task {
public:
    Task(std::function<void()> _func, TaskStatus _status = WAITING, unsigned _priority = 32);

    void add_prerequisite(Task* task);

    bool is_ready();

    void schedule();

    void execute();

    bool operator<(const Task& rhs) const { return priority < rhs.priority; }

    unsigned id;

private:
    const std::function<void()> func;
    const unsigned priority;

    std::deque<Task*> prereqs;

    /*TaskStatus status;
    std::mutex status_mtx;*/
    std::atomic<TaskStatus> status;
};

#endif