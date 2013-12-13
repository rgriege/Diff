#include <thread>
#include <iostream>

#include "Worker.h"
#include "Task.h"
#include "Scheduler.h"

unsigned Worker::n = 0;

Worker::Worker(Scheduler& _scheduler)
    : scheduler(_scheduler), id(n++) {}

Worker::Worker(const Worker& other)
    : scheduler(other.scheduler), id(n++) {}

void Worker::operator() ()
{
    std::cout << "Worker " << id << " starting" << std::endl << std::flush;
    Task* task = scheduler.check_out();
    while (task) {
        //std::cout << "Worker " << id << " executing Task " << task->id << std::endl << std::flush;
        task->execute();
        scheduler.check_in(task);
        task = scheduler.check_out();
    }
    std::cout << "Worker " << id << " finishing" << std::endl << std::flush;
}