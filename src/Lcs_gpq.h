#include <cassert>

#include "Scheduler.h"
#include "Task.h"
#include "Block.h"

template <class T, class Table>
void LCS_compute_corner_block_ij(T& x, T& y, Table& table, Block block)
{
    assert(block.row == 0);
    assert(block.col == 0);

    for (size_t i = 0; i < block.height; ++i)
        table[i][0] = 0;
    for (size_t j = 0; j < block.width; ++j)
        table[0][j] = 0;
    for (size_t i = 1; i < block.height; ++i) {
        for (size_t j = 1; j < block.width; ++j) {
            if (x[i-1] == y[j-1])
                table[i][j] = table[i-1][j-1] + 1;
            else
                table[i][j] = std::max(table[i][j-1], table[i-1][j]);
        }
    }
}

template <class T, class Table>
void LCS_compute_top_block_ij(T& x, T& y, Table& table, Block block)
{
    assert(block.row == 0);

    for (size_t j = block.col; j < block.max_col(); ++j)
        table[0][j] = 0;

    for (size_t i = block.row+1; i < block.height; ++i) {
        for (size_t j = block.col; j < block.max_col(); ++j) {
            if (x[i-1] == y[j-1])
                table[i][j] = table[i-1][j-1] + 1;
            else
                table[i][j] = std::max(table[i][j-1], table[i-1][j]);
        }
    }
}

template <class T, class Table>
void LCS_compute_left_block_ij(T& x, T& y, Table& table, Block block)
{
    assert(block.col == 0);

    for (size_t i = block.row; i < block.max_row(); ++i) {
        table[i][0] = 0;
        for (size_t j = 1; j < block.width; ++j) {
            if (x[i-1] == y[j-1])
                table[i][j] = table[i-1][j-1] + 1;
            else
                table[i][j] = std::max(table[i][j-1], table[i-1][j]);
        }
    }
}

template <class T, class Table>
void LCS_compute_inner_block_ij(T& x, T& y, Table& table, Block block)
{
    assert(block.row > 0);
    assert(block.col > 0);

    for (size_t i = block.row; i < block.max_row(); ++i) {
        for (size_t j = block.col; j < block.max_col(); ++j) {
            if (x[i-1] == y[j-1])
                table[i][j] = table[i-1][j-1] + 1;
            else
                table[i][j] = std::max(table[i][j-1], table[i-1][j]);
        }
    }
}

template <class T, class Table>
void LCS_compute_table_gq(T& x, T& y, Table& table, unsigned thread_count, unsigned block_width, unsigned block_height) {
    Scheduler scheduler(thread_count);

    unsigned num_normal_block_rows = table.height() / block_height;
    unsigned num_normal_block_cols = table.width() / block_width;
    unsigned block_padding_height = table.height() % block_height;
    unsigned block_padding_width = table.width() % block_width;
    unsigned num_block_rows = num_normal_block_rows + unsigned(block_padding_height > 0);
    unsigned num_block_cols = num_normal_block_cols + unsigned(block_padding_width > 0);

    /* Create the task array... */
    ArrayTable<Task*> tasks(num_block_rows, num_block_cols);
    Block tlb = {0, 0, block_padding_width ? block_padding_width : block_width,
            block_padding_height ? block_padding_height : block_height};
    /* ...by first populating the top left, possibly irregularly-sized block... */
    {
        tasks[0][0] = new Task(std::bind(LCS_compute_corner_block_ij<T, Table>, std::ref(x), std::ref(y), std::ref(table), tlb), READY);
        scheduler.add_task(tasks[0][0]);
    }
    /* ...and then all the left, possibly irregular-width blocks... */
    {
        Block b = {0, 0, block_padding_width ? block_padding_width : block_width, block_height};
        for (unsigned i = 1; i < num_block_rows; ++i) {
            b.row = (i-1)*block_height + tlb.height;
            tasks[i][0] = new Task(std::bind(LCS_compute_left_block_ij<T, Table>, std::ref(x), std::ref(y), std::ref(table), b));
            tasks[i][0]->add_prerequisite(tasks[i-1][0]);
            scheduler.add_postrequisite(tasks[i-1][0], tasks[i][0]);
        }
    }
    /* ...and then all the top, possibly irregular-height blocks... */
    {
        Block b = {0, 0, block_width, block_padding_height ? block_padding_height : block_height};
        for (unsigned j = 1; j < num_block_cols; ++j) {
            b.col = (j-1)*block_width + tlb.width;
            tasks[0][j] = new Task(std::bind(LCS_compute_top_block_ij<T, Table>, std::ref(x), std::ref(y), std::ref(table), b));
            tasks[0][j]->add_prerequisite(tasks[0][j-1]);
            scheduler.add_postrequisite(tasks[0][j-1], tasks[0][j]);
        }
    }
    /* ...and then all the inner, normal sized blocks. */
    for (unsigned i = 1; i < num_block_rows; ++i) {
        for (unsigned j = 1; j < num_block_cols; ++j) {
            Block b = {(i-1)*block_height + tlb.height, (j-1)*block_width + tlb.width, block_width, block_height};
            tasks[i][j] = new Task(std::bind(LCS_compute_inner_block_ij<T, Table>, std::ref(x), std::ref(y), std::ref(table), b));
            tasks[i][j]->add_prerequisite(tasks[i-1][j]);
            tasks[i][j]->add_prerequisite(tasks[i][j-1]);
            scheduler.add_postrequisite(tasks[i-1][j], tasks[i][j]);
            scheduler.add_postrequisite(tasks[i][j-1], tasks[i][j]);
        }
    }

    scheduler.run();
    
    for (unsigned i = 0; i < tasks.height(); ++i)
        for (unsigned j = 0; j < tasks.width(); ++j)
            delete tasks[i][j];
}
