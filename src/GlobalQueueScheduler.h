#include <thread>
#include <condition_variable>
#include <cassert>
#include <iostream>
#include <vector>

#include "Queue.h"
#include "ArrayTable.h"
#include "Block.h"

using namespace std;

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
            LCS_compute_table_ij(
            //acquired_tasks[i].execute();
            //for (auto it = acquired_tasks[i].begin_followup_tasks();
            //          it != acquired_tasks[i].end_followup_tasks();
            //          ++it)
            //    generated_tasks.push_back(*it);
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
    Block<int> dataless_block = {0, block_size, block_size, table.width() - block_size, WAITING};
    ArrayTable<Block<int> > blocks((table.height() + block_size - 1)/ block_size,
        (table.width() + block_size - 1)/ block_size, dataless_block);
    /* set the data pointer for each block */
    for (size_t i = 0; i < blocks.height(); ++i)
        for (size_t j = 0; j < blocks.width(); ++j)
            blocks[i][j].data = &table[i*block_size][j*block_size];

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
