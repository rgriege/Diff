#include "BlockedCstring.h"

BlockedCstring::BlockedCstring(FILE* f, int block_size)
    : block_size(block_size), num_blocks(0)
{
    size_t result;
    do {
        data.push_back(new char[block_size]);
        result = fread(data[num_blocks], 1, block_size, f);
        ++num_blocks;
    } while (result == block_size);

    if (result == 0) {
        --num_blocks;
        delete[] data[num_blocks];
        data.pop_back();
    } else {
        for (int i = result; i < block_size; ++i)
            data[num_blocks-1][i] = 0;
    }
}

BlockedCstring::BlockedCstring(std::istream& i, int block_size)
    : block_size(block_size), num_blocks(0)
{
    std::streamsize result;
    do {
        data.push_back(new char[block_size]);
        i.read(data[num_blocks], block_size);
        result = i.gcount();
        ++num_blocks;
    } while (result == block_size);

    if (result == 0) {
        --num_blocks;
        delete[] data[num_blocks];
        data.pop_back();
    } else {
        for (int i = result; i < block_size; ++i)
            data[num_blocks-1][i] = 0;
    }
}

char& BlockedCstring::operator[](int i)
{
    return data[i / block_size][i % block_size];
}

const char& BlockedCstring::operator[](int i) const
{
    return data[i / block_size][i % block_size];
}

size_t strlen (const char* _Str, size_t max)
{
    size_t i = 0;
    while (i < max && _Str[i] != 0) { ++i; }
    return i;
}

int BlockedCstring::length() const
{
    return (num_blocks-1)*56 + strlen(data[num_blocks-1], block_size);
}