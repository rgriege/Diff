#ifndef __WORKER_H__
#define __WORKER_H__

class Scheduler;

class Worker {
public:
    Worker(Scheduler& _scheduler);
    Worker(const Worker& other);

    void operator() ();

    unsigned id;
    static unsigned n;

private:
    Scheduler& scheduler;
}; 

#endif