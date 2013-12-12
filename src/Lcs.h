#include <iostream>
#include <algorithm>
#include <array>

#include "BlockedData.h"
#include "Table.h"

#undef max
#undef min

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
    auto x_it = x_begin;
    while(x_it != x_end) {
        ++x_it;
        ++x_size;
    }

    int y_size = 1;
    auto y_it = y_begin;
    while(y_it != y_end) {
        ++y_it;
        ++y_size;
    }

    return new int[x_size*y_size];
}

template <class T>
void LCS_compute_table_ij(T* x, int x_len, T* y, int y_len, int* table)
{
    int cx_len = x_len + 1;
    int cy_len = y_len + 1;
    for (int i = 0; i <= x_len; i++)
        table[i*cy_len] = 0;
    for (int j = 0; j <= y_len; j++)
        table[j] = 0;
    for (int i = 1; i <= x_len; ++i) {
        for (int j = 1; j <= y_len; ++j) {
            if (x[i-1] == y[j-1])
                table[i*cy_len+j] = table[(i-1)*cy_len+(j-1)] + 1;
            else
                table[i*cy_len+j] = std::max(table[i*cy_len+(j-1)], table[(i-1)*cy_len+j]);
        }
    }
}

template <class T>
void LCS_compute_table_ji(T* x, int x_len, T* y, int y_len, int* table)
{
    int cx_len = x_len + 1;
    int cy_len = y_len + 1;
    for (int i = 0; i <= x_len; i++)
        table[i*cy_len] = 0;
    for (int j = 0; j <= y_len; j++)
        table[j] = 0;
    for (int j = 1; j <= y_len; ++j) {
        for (int i = 1; i <= x_len; ++i) {
            if (x[i-1] == y[j-1])
                table[i*cy_len+j] = table[(i-1)*cy_len+(j-1)] + 1;
            else
                table[i*cy_len+j] = std::max(table[i*cy_len+(j-1)], table[(i-1)*cy_len+j]);
        }
    }
}

template <class T>
void LCS_compute_table(T* x, T* y, Table<int>& table)
{
    for (Table<int>::size_type i = 0; i < table.height; i++)
        table[i][0] = 0;
    for (Table<int>::size_type j = 0; j < table.width; j++)
        table[0][j] = 0;
    for (Table<int>::size_type i = 1; i < table.height; ++i) {
        for (Table<int>::size_type j = 1; j < table.width; ++j) {
            if (x[i-1] == y[j-1]) {
                table[i][j] = table[i-1][j-1] + 1;
            } else
                table[i][j] = std::max(table[i][j-1], table[i-1][j]);
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
            if (x[i-1] == y[j-1])
                table[i*cy_len+j] = table[(i-1)*cy_len+(j-1)] + 1;
            else
                table[i*cy_len+j] = std::max(table[i*cy_len+(j-1)], table[(i-1)*cy_len+j]);
        }
    }
}

template <class T>
void LCS_compute_table_ijij(T* x, int x_len, T* y, int y_len, int* table, int b)
{
    int cx_len = x_len + 1;
    int cy_len = y_len + 1;
    for (int i = 0; i <= x_len; i++)
        table[i*cy_len] = 0;
    for (int j = 0; j <= y_len; j++)
        table[j] = 0;
    for (int ii = 1; ii <= x_len; ii += b) {
        int imax = std::min(ii+b, x_len);
        for (int jj = 1; jj <= y_len; jj += b) {
            int jmax = std::min(jj+b, y_len);
            for (int i = ii; i <= imax; ++i) {
                for (int j = jj; j <= jmax; ++j) {
                    if (x[i-1] == y[j-1])
                        table[i*cy_len+j] = table[(i-1)*cy_len+(j-1)] + 1;
                    else
                        table[i*cy_len+j] = std::max(table[i*cy_len+(j-1)], table[(i-1)*cy_len+j]);
                }
            }
        }
    }
}

template <class T>
void LCS_compute_table_jiji(T* x, int x_len, T* y, int y_len, int* table, int b)
{
    int cx_len = x_len + 1;
    int cy_len = y_len + 1;
    for (int i = 0; i <= x_len; i++)
        table[i*cy_len] = 0;
    for (int j = 0; j <= y_len; j++)
        table[j] = 0;
    for (int jj = 1; jj <= y_len; jj += b) {
        int jmax = std::min(jj+b, y_len);
        for (int ii = 1; ii <= x_len; ii += b) {
            int imax = std::min(ii+b, x_len);
            for (int j = jj; j <= jmax; ++j) {
                for (int i = ii; i <= imax; ++i) {
                    if (x[i-1] == y[j-1])
                        table[i*cy_len+j] = table[(i-1)*cy_len+(j-1)] + 1;
                    else
                        table[i*cy_len+j] = std::max(table[i*cy_len+(j-1)], table[(i-1)*cy_len+j]);
                }
            }
        }
    }
}

template <class T>
void LCS_compute_table_jij(T* x, int x_len, T* y, int y_len, int* table, int b)
{
    int cx_len = x_len + 1;
    int cy_len = y_len + 1;
    for (int i = 0; i <= x_len; i++)
        table[i*cy_len] = 0;
    for (int j = 0; j <= y_len; j++)
        table[j] = 0;
    for (int jj = 1; jj <= y_len; jj += b) {
        int jmax = std::min(jj+b, y_len);
        for (int i = 1; i <= x_len; ++i) {
            for (int j = jj; j <= jmax; ++j) {
                if (x[i-1] == y[j-1])
                    table[i*cy_len+j] = table[(i-1)*cy_len+(j-1)] + 1;
                else
                    table[i*cy_len+j] = std::max(table[i*cy_len+(j-1)], table[(i-1)*cy_len+j]);
            }
        }
    }
}

int LCS_length(int* table, int cx_len, int cy_len)
{
    return table[cx_len*cy_len-1];
}

int LCS_length(const Table<int>& table)
{
    return table[table.height-1][table.width-1];
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
            out << C[i*(y.length()+1)+j] << " ";
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
        int cy_len = y.length() + 1;
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

template <class T>
void LCS_read_helper(int*& table, T*& x, T*& y, const int& cy_len, int i, int j,
                     std::ostream& out, OutputMode mode)
{
    if (i == 0 || j == 0) {
        return;
    } else if (x[i-1] == y[j-1]) {
        if (mode != NORMAL)
            switch_from_mode(mode, out);
        out << x[i-1];
        LCS_read_helper(table, x, y, cy_len, i-1, j-1, out, NORMAL);
    } else {
        if (table[i*cy_len+(j-1)] > table[(i-1)*cy_len+j]) {
            if (mode != INSERTION)
                switch_from_mode(mode, out);
            out << y[j-1];
            LCS_read_helper(table, x, y, cy_len, i, j-1, out, INSERTION);
        } else {
            if (mode != DELETION)
                switch_from_mode(mode, out);
            out << x[i-1];
            LCS_read_helper(table, x, y, cy_len, i-1, j, out, DELETION);
        }
    }
}

void LCS_read(int* table, const char* x, const char* y, std::ostream& out)
{
    LCS_read_helper(table, x, y, strlen(y)+1, strlen(x), strlen(y), out, NORMAL);
}
