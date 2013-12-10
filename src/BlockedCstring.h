#ifndef __BLOCKED_CSTRING_H__
#define __BLOCKED_CSTRING_H__

#include <istream>
#include <vector>

template <class T>
class BlockedCstring {
public:
    BlockedCstring(FILE* f, int block_size);
    BlockedCstring(std::istream& i, int block_size);
    T& operator[](int);
    const T& operator[](int) const;
    int length() const;

private:
    std::vector<T*> data;
    const int block_size;
    int num_blocks;
};

#endif