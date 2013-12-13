#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__

#include <functional>
#include <mutex>
#include <map>
#include <vector>
#include <queue>
#include <condition_variable>

#include "Task.h"
#include "Worker.h"

template <class T>
struct less_ptr {
    bool operator() (const T* lhs, const T* rhs) const
    {
        return *lhs < *rhs;
    }
    typedef T* first_argument_type;
    typedef T* second_argument_type;
    typedef bool result_type;
};

class Scheduler {

    typedef size_t size_type;

public:
    Scheduler(size_type _worker_count);

    void add_task(Task* task);

    void add_postrequisite(Task* task, Task* postrequisite);

    void run();

    Task* check_out();
    void check_in(Task* task);

private:
    const size_type worker_count;
    size_type waiting_worker_count;
    std::priority_queue<Task*, std::vector<Task*>, less_ptr<Task> > queue;
    std::mutex mtx;
    std::condition_variable cv;
    std::multimap<Task*, Task*> postreqs;

    unsigned num_tasks;
};

#endif
