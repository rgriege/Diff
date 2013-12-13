#include <cassert>
#include <iostream>

#include "Task.h"

static unsigned n = 0;

Task::Task(std::function<void()> _func, TaskStatus _status, unsigned _priority)
    : func(_func), priority(_priority), status(_status), id(n++) {}

void Task::add_prerequisite(Task* task)
{
    assert(task != NULL);
    prereqs.push_back(task);
}

bool Task::is_ready()
{
    //std::lock_guard<std::mutex> status_lk(status_mtx);
    switch (status) {
        case WAITING:
            // prereq's execute and this' is_ready are always called sequentially from the same thread
            for (auto it = prereqs.begin(); it != prereqs.end(); ++it)
                if ((*it)->status != COMPLETE)
                    return false;
            status = READY;
        case READY:
            return true;
        case SCHEDULED:
        case COMPLETE:
        default:
            return false;
    }
}

void Task::schedule()
{
    assert(status == READY);
    status = SCHEDULED;
    //std::cout << "Task " << id << " scheduled" << std::endl;
}

void Task::execute()
{
    assert(status == SCHEDULED);
    func();
    //std::lock_guard<std::mutex> status_lk(status_mtx);
    status = COMPLETE;
    //std::cout << "Task " << id << " executed" << std::endl;
}