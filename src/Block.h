#ifndef __BLOCK_H__
#define __BLOCK_H__

struct Block {
    size_t row;
    size_t col;
    size_t width;
    size_t height;

    size_t max_col() { return col + width; }
    size_t max_row() { return row + height; }
};

#endif
