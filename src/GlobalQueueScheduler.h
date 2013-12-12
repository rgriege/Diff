#include <thread>
#include <condition_variable>
#include <cassert>
#include <iostream>
#include <vector>

#include "Queue.h"
#include "ArrayTable.h"
#include "Block.h"

using namespace std;

template <class T>
void LCS_compute_table_block_ij(T& x, T& y, Block<int>& table)
{
    for (size_t i = 0; i <= x.length(); i++)
        table[i][0] = 0;
    for (size_t j = 0; j <= y.length(); j++)
        table[0][j] = 0;
    for (size_t i = 1; i <= x.length(); ++i) {
        for (size_t j = 1; j <= y.length(); ++j) {
            if (x[i-1] == y[j-1])
                table[i][j] = table[i-1][j-1] + 1;
            else
                table[i][j] = std::max(table[i][j-1], table[i-1][j]);
        }
    }
}
template <typename T>
struct global_locked_queue : public locked_queue<T> {
    global_locked_queue() : num_waiting_threads(0) {}
    int num_waiting_threads;
};

static condition_variable cv;
static int num_threads;

void global_queue_thread_func(global_locked_queue<Block<int> >& g_queue, ArrayTable<Block<int> >& blocks, int idx)
{
    vector<Block<int> > acquired_tasks;
    vector<Block<int> > generated_tasks;
    while (1) {
        /* Get some work, or exit if no work remains */
        {
            unique_lock<mutex> lk(g_queue.mtx);
            if (g_queue.q.empty()) {
                g_queue.num_waiting_threads++;
                if (g_queue.num_waiting_threads == num_threads) {
                    cv.notify_all();
                } else {
                    while(g_queue.q.empty() && !g_queue.num_waiting_threads == num_threads)
                        cv.wait(lk);
                }
                if (g_queue.num_waiting_threads == num_threads)
                    return;
                else
                    g_queue.num_waiting_threads--;
            }
            for (int i = 0; i < 1; ++i) {
                if (g_queue.q.empty())
                    break;
                acquired_tasks.push_back(g_queue.next());
                g_queue.q.pop();
            }
        }

        for (unsigned int i = 0; i < acquired_tasks.size(); ++i) {
            /* Process the current node */
            Block<int>& block = acquired_tasks[i];
            LCS_compute_table_block_ij();
            /* Possibly enqueue the block immediately below */
            if (block.row < table.height() - 1) {
                Block<int>& d_block = blocks[block.row+1][block.col];
                if (d_block.try_enqueue(block.col > 0 ? &blocks[block.row+1][block.col-1] : NULL, &block))
                    generated_tasks.push_back(d_block);
            }
            /* Possibly enqueue the block immediately to the right */
            if (block.col < table.width() - 1) {
                Block<int>& r_block = blocks[block.row][block.col+1];
                if (r_block.try_enqueue(&block, block.row > 0 ? &blocks[block.row-1][block.col+1] : 0))
                    generated_tasks.push_back(r_block);
            }
        }
        {
            lock_guard<mutex> q_lk(g_queue.mtx);
            for (unsigned int i = 0; i < generated_tasks.size(); ++i)
                g_queue.q.push(generated_tasks[i]);
        }
        for (unsigned int i = 0; i < generated_tasks.size(); ++i)
            cv.notify_one();
        acquired_tasks.clear();
        generated_tasks.clear();
    }
}

void schedule_tasks(ArrayTable<int>& table, int _num_threads, int block_size) {
    /* Intialize the block table, setting the width, height, row increment, and status */
    Block<int> dataless_block = {0, block_size, block_size, 0, 0, table.width(), WAITING};
    ArrayTable<Block<int> > blocks((table.height() + block_size - 1)/ block_size,
        (table.width() + block_size - 1)/ block_size, dataless_block);
    /* set the data pointer, row, and column for each block */
    for (size_t i = 0; i < blocks.height(); ++i) {
        for (size_t j = 0; j < blocks.width(); ++j) {
            blocks[i][j].data = &table[i*block_size][j*block_size];
            blocks[i][j].row = i;
            blocks[i][j].col = j;
        }
    }

    num_threads = _num_threads;

    global_locked_queue<Block<int> > queue;
    queue.num_waiting_threads = 0;
    blocks[0][0].status = QUEUED;
    queue.q.push(blocks[0][0]);

    thread* threads = new thread[num_threads];

    for (int i = 0; i < num_threads; ++i)
        threads[i] = thread(global_queue_thread_func, std::ref(queue), std::ref(blocks) i);

    for (int i = 0; i < num_threads; ++i)
        threads[i].join();

    assert(queue.q.empty());

    delete[] threads;
}
