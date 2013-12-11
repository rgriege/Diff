#include <iostream>
#include <algorithm>
#undef max

#include "BlockedData.h"

int* LCS_create_table(int x_len, int y_len)
{
    int cx_len = x_len + 1;
    int cy_len = y_len + 1;
    return new int[cx_len*cy_len];
}

template <class Iterator>
int* LCS_create_table(Iterator x_begin, Iterator x_end, Iterator y_begin, Iterator y_end)
{
    int x_size = 1;
    x_it = x_begin;
    while(x_it != x_end) {
        ++x_it;
        ++x_size;
    }

    int y_size = 1;
    y_it = y_begin;
    while(y_it != y_end) {
        ++y_it;
        ++y_size;
    }

    return new int[x_size*y_size];
}

template <class T>
void LCS_compute_table(T* x, int x_len, T* y, int y_len, int* table)
{
    int cx_len = x_len + 1;
    int cy_len = y_len + 1;
    for (int i = 0; i <= x_len; i++)
        table[i*cy_len] = 0;
    for (int j = 0; j <= y_len; j++)
        table[j] = 0;
    for (int i = 1; i <= x_len; ++i) {
        for (int j = 1; j <= y_len; ++j) {
            if (x[i-1] == y[i-1])
                table[i*cy_len+j] = table[(i-1)*cy_len+(j-1)] + 1;
            else
                table[i*cy_len+j] = std::max(table[i*cy_len+(j-1)], table[(i-1)*cy_len+j]);
        }
    }
}

template <class T>
void LCS_compute_table(BlockedData<T>& x, BlockedData<T>& y, int* table)
{
    int cx_len = x.length() + 1;
    int cy_len = y.length() + 1;
    for (int i = 0; i <= x.length(); i++)
        table[i*cy_len] = 0;
    for (int j = 0; j <= y.length(); j++)
        table[j] = 0;
    for (int i = 1; i <= x.length(); i++) {
        for (int j = 1; j <= y.length(); j++) {
            if (x[i-1] == y[i-1])
                table[i*cy_len+j] = table[(i-1)*cy_len+(j-1)] + 1;
            else
                table[i*cy_len+j] = std::max(table[i*cy_len+(j-1)], table[(i-1)*cy_len+j]);
        }
    }
}

template <class T>
void LCS_compute_table_blocked(T* x, int x_len, T* y, int y_len, int* table, int b)
{
    int cx_len = x_len + 1;
    int cy_len = y_len + 1;
    for (int i = 0; i <= x_len; i++)
        table[i*cy_len] = 0;
    for (int j = 0; j <= y_len; j++)
        table[j] = 0;
    for (int ii = 1; ii <= x_len; ii += b) {
        int imax = min(ii+b, x_len);
        for (int jj = 1; jj <= y_len; jj += b) {
            int jmax = min(jj+b, y_len);
            for (int i = ii; i <= imax; ++i) {
                for (int j = jj; j <= jmax; ++j) {
                    if (x[i-1] == y[i-1])
                        table[i*cy_len+j] = table[(i-1)*cy_len+(j-1)] + 1;
                    else
                        table[i*cy_len+j] = std::max(table[i*cy_len+(j-1)], table[(i-1)*cy_len+j]);
                }
            }
        }
    }
}

int LCS_length(int* table, int cx_len, int cy_len)
{
    return table[cx_len*cy_len-1];
}

template <class T>
void LCS_print_table(int* C, BlockedData<T>& x, BlockedData<T>& y, std::ostream& out)
{
    out << "  0 ";
    for (int j = 0; j < y.length(); j++)
        out << y[j] << " ";
    out << std::endl;
    for (int i = 0; i <= x.length(); i++) {
        out << (i == 0 ? '0' : x[i-1]) << " ";
        for (int j = 0; j <= y.length(); j++) {
            out << C[i*cy_len+j] << " ";
        }
        out << std::endl;
    }
}

enum OutputMode {
    NORMAL,
    INSERTION,
    DELETION,
};

void switch_from_mode(OutputMode mode, std::ostream& out)
{
    if (mode == INSERTION)
        out << "+";
    else if (mode == DELETION)
        out << "-";
    out << std::endl;
}

template <class T>
void LCS_read_helper(int*& C, BlockedData<T>& x, BlockedData<T>& y, int i, int j,
                     std::ostream& out, OutputMode mode)
{
    if (i == 0 || j == 0) {
        return;
    } else if (x[i-1] == y[j-1]) {
        if (mode != NORMAL)
            switch_from_mode(mode, out);
        out << x[i-1];
        LCS_read_helper(C, x, y, i-1, j-1, out, NORMAL);
    } else {
        if (C[i*cy_len+(j-1)] > C[(i-1)*cy_len+j]) {
            if (mode != INSERTION)
                switch_from_mode(mode, out);
            out << y[j-1];
            LCS_read_helper(C, x, y, i, j-1, out, INSERTION);
        } else {
            if (mode != DELETION)
                switch_from_mode(mode, out);
            out << x[i-1];
            LCS_read_helper(C, x, y, i-1, j, out, DELETION);
        }
    }
}

template <class T>
void LCS_read(int* C, BlockedData<T>& x, BlockedData<T>& y, std::ostream& out)
{
    LCS_read_helper(C, x, y, x.length(), y.length(), out, NORMAL);
}