#ifndef __BLOCK_H__
#define __BLOCK_H__

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
    size_t row_incr;
    BlockStatus status;
};

#endif