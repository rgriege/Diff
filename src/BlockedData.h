#ifndef __BLOCKED_DATA_H__
#define __BLOCKED_DATA_H__

#include <istream>
#include <vector>

size_t strlen (const char* _Str, size_t max)
{
    size_t i = 0;
    while (i < max && _Str[i] != 0) { ++i; }
    return i;
}

template <class T>
class BlockedData {
public:
    BlockedData(FILE* f, int block_size)
        : block_size(block_size), num_blocks(0)
    {
        size_t result;
        do {
            data.push_back(new T[block_size]);
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


    BlockedData(std::istream& i, int block_size)
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
            for (std::streamsize i = result; i < block_size; ++i)
                data[num_blocks-1][i] = 0;
        }
    }


    T& operator[](int i)
    {
        return data[i / block_size][i % block_size];
    }


    const T& operator[](int i) const
    {
        return data[i / block_size][i % block_size];
    }


    int length() const
    {
        return (num_blocks-1)*56 + strlen(data[num_blocks-1], block_size);
    }


private:
    std::vector<T*> data;
    const int block_size;
    int num_blocks;
};

#endif