
class Task;
class Worker;
class Scheduler;

enum TaskStatus {
    WAITING,
    READY,
    SCHEDULED,
    COMPLETE
};

class Task {
    friend bool less_ptr(const Task*& lhs, const Task*& rhs)
    {
        return lhs->priority < rhs->priority;
    }

public:
    Task(std::function<void()> _func, TaskStatus _status = WAITING, unsigned _priority = 32)
        : func(_func), priority(_priority), status(_status) {}

    void schedule()
    {
        status = SCHEDULED;
    }

    void execute()
    {
        assert(status == READY);
        func();
        lock_guard<std::mutex> status_lk(statux_mtx);
        status = COMPLETE;
    }

    bool operator<(const Task& rhs) { return priority < rhs.priority; }

    bool is_ready()
    {
        lock_guard<std::mutex> status_lk(statux_mtx);
        switch (status) {
            case WAITING:
                for (auto it = prereqs.begin() it != prereqs.end(); ++it)
                    if (it->status == COMPLETE) // should access atomically?
                        break;
                return it == prereqs.end();
                break;
            case READY:
                return true;
            case SCHEDULED:
            case COMPLETE:
                return false;
        }
    }

private:
    std::function<void()> func;
    const unsigned priority;

    std::deque<Task*> prereqs;
    std::deque<Task*> postreqs;

    TaskStatus status;
    std::mutex status_mtx;
};

class Worker {
private:
    Worker(Scheduler& _scheduler)
        : scheduler(_scheduler) {}

    void start()
    {
        std::thread t(*this);
    }

    void operator()
    {

    }

public:
    static Worker* create(Scheduler& scheduler)
    {
        Worker* w = new Worker(scheduler);
        w->start();
    }

    Scheduler& scheduler;
}; 

class Scheduler {

    typedef size_t size_type;

public:
    Scheduler(size_type _worker_count)
        : worker_count(_worker_count) {}

    void add_dependency(Task* task, Task* prereq)
    {
        postreqs.insert(std::pair<Task*, Task*>(prereq, task));
    }

    void begin()
    {
        for (size_type i = 0; i < worker_count; ++i)
            workers.push_back(Worker::create(*this));
    }

    void end()
    {

    }

    Task* check_out()
    {
        lock_guard<mutex> q_lk(queue.mtx);
        if (queue.empty())
            return NULL;

        Task* task = queue.q.next();
        queue.q.pop();
        return task;
    }

    void check_in(Task* task)
    {
        auto it_pair = postreqs.equal_range(task);
        for (auto it = it_pair.first; it != it_pair.second; ++it) {
            if (it->is_ready()) {
                it->schedule();
                lock_guard<mutex> q_lk(queue.mtx);
                quque.q.push(*it);
            }
        }
    }

private:
    size_type worker_count;
    locked_queue<Task*, std::vector<Task*>, less_ptr> queue;
    std::multimap<Task*, Task*> postreqs;
    std::vector<Worker*> workers;
};
