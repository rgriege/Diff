#include <functional>
#include <deque>
#include <cassert>
#include <mutex>

class Task {
public:
    Task(std::function<void()> func, std::deque<Task*> prereqs, std::deque<Task*> postreqs)
        : func(func), prereqs(prereqs), postreqs(postreqs) {}

    void execute()
    {
        assert(status == QUEUED);
        func();
        status = COMPLETE;
    }

    Task* next_postreq()
    {
        if (postreqs.empty())
            return NULL;

        Task* next = postreqs.front();
        postreqs.pop_front();
        return next;
    }

    enum TaskStatus {
        WAITING,
        QUEUED,
        COMPLETE
    };

    TaskStatus status;

    std::mutex status_mtx;

private:

    const std::function<void()> func;
    std::deque<Task*> postreqs;
    std::deque<Task*> prereqs;
};