#ifndef __BLOCKED_CSTRING_H__
#define __BLOCKED_CSTRING_H__

#include <istream>
#include <vector>

class BlockedCstring {
public:
    BlockedCstring(FILE* f, int block_size);
    BlockedCstring(std::istream& i, int block_size);
    char& operator[](int);
    const char& operator[](int) const;
    int length() const;

private:
    std::vector<char*> data;
    int block_size;
    int num_blocks;
};

#endif