#include <cassert>
#include <thread>
#include <iostream>

#include "Scheduler.h"

Scheduler::Scheduler(size_type _worker_count)
    : worker_count(_worker_count), waiting_worker_count(0), num_tasks(0) {}

void Scheduler::add_task(Task* task)
{
    assert(task != NULL);
    if (task->is_ready()) {
        task->schedule();
        queue.push(task);
    }
}

void Scheduler::add_postrequisite(Task* task, Task* postrequisite)
{
    assert(task != NULL);
    assert(postrequisite != NULL);
    postreqs.insert(std::pair<Task*, Task*>(task, postrequisite));
}

void Scheduler::run()
{
    std::vector<Worker> workers(worker_count, Worker(*this));
    std::thread* threads = new std::thread[worker_count];
    for (size_type i = 0; i < worker_count; ++i)
        threads[i] = std::thread(workers[i]);
    for (size_type i = 0; i < worker_count; ++i)
        threads[i].join();
}

Task* Scheduler::check_out()
{
    /* Get some work, or exit if no work remains */
    std::unique_lock<std::mutex> lk(mtx);
    if (queue.empty()) {
        waiting_worker_count++;
        if (waiting_worker_count == worker_count) {
            cv.notify_all();
        } else {
            while(queue.empty() && waiting_worker_count != worker_count)
                cv.wait(lk);
        }
        if (waiting_worker_count == worker_count)
            return NULL;
        else
            waiting_worker_count--;
    }
    ++num_tasks;
    Task* task = queue.front();
    queue.pop();
    return task;
}

void Scheduler::check_in(Task* task)
{
    auto it_pair = postreqs.equal_range(task);
    for (auto it = it_pair.first; it != it_pair.second; ++it) {
        if (it->second->is_ready()) {
            it->second->schedule();
            {
                std::lock_guard<std::mutex> q_lk(mtx);
                queue.push(it->second);
            }
            cv.notify_one();
        }
    }
}