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
    BlockStatus status;
    std::mutex status_mtx;

    bool operator<(const Block<T>& other) const { return true; }
    T* operator[](size_t i) { return data + row_incr*i; }

    bool try_enqueue(Block<T>* left_block, Block<T>* up_block)
    {
        lock_guard<mutex> m_lk(status_mtx);
        if (status == COMPLETE && status == QUEUED)
            return false;

        Block<T>* single_block = !left_block ? up_block : !up_block ? left_block : NULL;
        if (single_block) {
            lock_guard<mutex> o_lk(single_block->status_mtx);
            if (single_block->status == COMPLETE)
                status = QUEUED;
        } else {
            std::lock(left_block->status_mtx, up_block->status_mtx);
            if (left_block->status == COMPLETE && up_block->status == COMPLETE)
                status = QUEUED;
            left_block->status_mtx.unlock();
            up_block->status_mtx.unlock();
        }
        return status == QUEUED;
    }
};

#endif
