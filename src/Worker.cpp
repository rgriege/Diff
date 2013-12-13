#include <thread>

#include "Worker.h"
#include "Task.h"
#include "Scheduler.h"

Worker::Worker(Scheduler& _scheduler)
    : scheduler(_scheduler) {}

Worker* Worker::create(Scheduler& scheduler)
{
    Worker* w = new Worker(scheduler);
    w->start();
    return w;
}

void Worker::start()
{
    std::thread t(*this);
    t.join();
}
void Worker::operator() ()
{
    Task* task = scheduler.check_out();
    while (task) {
        task->execute();
        scheduler.check_in(task);
        task = scheduler.check_out();
    }
}