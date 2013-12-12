#ifndef __BLOCK_H__
#define __BLOCK_H__

#include <mutex>

enum BlockStatus {
    WAITING,
    QUEUED,
    COMPLETE
};

template <class T>
struct Block {
    T* data;
    size_t width;
    size_t height;
    size_t row;
    size_t col;
    size_t row_incr;
    std::mutex status_mtx;
    BlockStatus status;

    bool operator<(const Block<T>& other) const { return true; }
};

#endif
