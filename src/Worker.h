#ifndef __WORKER_H__
#define __WORKER_H__

class Scheduler;

class Worker {
private:
    Worker(Scheduler& _scheduler);

    void start();

public:
    static Worker* create(Scheduler& scheduler);

    void operator() ();

    Scheduler& scheduler;
}; 

#endif