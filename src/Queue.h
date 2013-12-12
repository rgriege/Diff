#ifndef __QUEUE_H__
#define __QUEUE_H__

#include <mutex>
#include <queue>

using namespace std;

template <class T, class Container = std::vector<T>, class Predicate = less<class Container::value_type> >
struct locked_queue {
    priority_queue<T, Container, Predicate> q;
    mutex mtx;

    const T& next() { return q.front(); }
    
    locked_queue() {}
    locked_queue(const locked_queue& other) { q = other.q; };
    locked_queue& operator=(const locked_queue& other) { q = other.q; return *this; }
};

#endif
