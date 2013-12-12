#include <iostream>
#include <algorithm>
#include <array>

#include "BlockedData.h"
#include "ArrayTable.h"
#include "Table.h"

#undef max
#undef min

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

//template <class T>
//void LCS_compute_table_ij(T x, T y, ArrayTable<int> table)
//{
//    for (size_t i = 0; i <= x.length(); i++)
//        table.at(i, 0) = 0;
//    for (size_t j = 0; j <= y.length(); j++)
//        table.at(0, j) = 0;
//    for (size_t i = 1; i <= x.length(); ++i) {
//        for (size_t j = 1; j <= y.length(); ++j) {
//            if (x[i-1] == y[j-1])
//                table.at(i, j) = table.at(i-1, j-1) + 1;
//            else
//                table.at(i, j) = std::max(table.at(i, j-1), table.at(i-1, j));
//        }
//    }
//}

template <class T>
void LCS_compute_table_ij(T& x, T& y, ArrayTable<int>& table)
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

template <class T>
void LCS_compute_table_ji(T& x, T& y, ArrayTable<int>& table)
{
    for (size_t i = 0; i <= x.length(); i++)
        table[i][0] = 0;
    for (size_t j = 0; j <= y.length(); j++)
        table[0][j] = 0;
    for (size_t j = 1; j <= y.length(); ++j) {
        for (size_t i = 1; i <= x.length(); ++i) {
            if (x[i-1] == y[j-1])
                table[i][j] = table[i-1][j-1] + 1;
            else
                table[i][j] = std::max(table[i][j-1], table[i-1][j]);
        }
    }
}

template <class T>
void LCS_compute_table_ijij(T& x, T& y, ArrayTable<int>& table, int block_size)
{
    for (size_t i = 0; i <= x.length(); i++)
        table[i][0] = 0;
    for (size_t j = 0; j <= y.length(); j++)
        table[0][j] = 0;
    for (size_t ii = 1; ii <= x.length(); ii += block_size) {
        size_t imax = std::min(ii+block_size, x.length());
        for (size_t jj = 1; jj <= y.length(); jj += block_size) {
            size_t jmax = std::min(jj+block_size, y.length());
            for (size_t i = ii; i <= imax; ++i) {
                for (size_t j = jj; j <= jmax; ++j) {
                    if (x[i-1] == y[j-1])
                        table[i][j] = table[i-1][j-1] + 1;
                    else
                        table[i][j] = std::max(table[i][j-1], table[i-1][j]);
                }
            }
        }
    }
}

template <class T>
void LCS_compute_table_jiji(T& x, T& y, ArrayTable<int>& table, int block_size)
{
    for (size_t i = 0; i <= x.length(); i++)
        table[i][0] = 0;
    for (size_t j = 0; j <= y.length(); j++)
        table[0][j] = 0;
    for (size_t jj = 1; jj <= y.length(); jj += block_size) {
        size_t jmax = std::min(jj+block_size, y.length());
        for (size_t ii = 1; ii <= x.length(); ii += block_size) {
            size_t imax = std::min(ii+block_size, x.length());
            for (size_t j = jj; j <= jmax; ++j) {
                for (size_t i = ii; i <= imax; ++i) {
                    if (x[i-1] == y[j-1])
                        table[i][j] = table[i-1][j-1] + 1;
                    else
                        table[i][j] = std::max(table[i][j-1], table[i-1][j]);
                }
            }
        }
    }
}

template <class T>
void LCS_compute_table_jij(T& x, T& y, ArrayTable<int>& table, int block_size)
{
    for (size_t i = 0; i <= x.length(); i++)
        table[i][0] = 0;
    for (size_t j = 0; j <= y.length(); j++)
        table[0][j] = 0;
    for (size_t jj = 1; jj <= y.length(); jj += block_size) {
        size_t jmax = std::min(jj+block_size, y.length());
        for (size_t i = 1; i <= x.length(); ++i) {
            for (size_t j = jj; j <= jmax; ++j) {
                if (x[i-1] == y[j-1])
                    table[i][j] = table[i-1][j-1] + 1;
                else
                    table[i][j] = std::max(table[i][j-1], table[i-1][j]);
            }
        }
    }
}

int LCS_length(const ArrayTable<int>& table)
{
    return table[table.height()-1][table.width()-1];
}

template <class T>
void LCS_print_table(const T& x, const T& y, const ArrayTable<int>& table, std::ostream& out)
{
    out << "  0 ";
    for (size_t j = 0; j < y.length(); j++)
        out << y[j] << " ";
    out << std::endl;
    for (size_t i = 0; i < x.length(); i++) {
        out << x[i] << " ";
        for (size_t j = 0; j <= y.length(); j++)
            out << table[i+1][j] << " ";
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
void LCS_read_helper(const T& x, const T& y, const ArrayTable<int>& table, size_t i, size_t j,
                     std::ostream& out, OutputMode mode)
{
    if (i == 0 || j == 0) {
        return;
    } else if (x[i-1] == y[j-1]) {
        if (mode != NORMAL)
            switch_from_mode(mode, out);
        out << x[i-1];
        LCS_read_helper(x, y, table, i-1, j-1, out, NORMAL);
    } else {
        if (table[i][j-1] > table[i-1][j]) {
            if (mode != INSERTION)
                switch_from_mode(mode, out);
            out << y[j-1];
            LCS_read_helper(x, y, table, i, j-1, out, INSERTION);
        } else {
            if (mode != DELETION)
                switch_from_mode(mode, out);
            out << x[i-1];
            LCS_read_helper(x, y, table, i-1, j, out, DELETION);
        }
    }
}

template <class T>
void LCS_read(const T& x, const T& y, const ArrayTable<int>& table, std::ostream& out)
{
    std::ostringstream rout;
    LCS_read_helper(x, y, table, x.length(), y.length(), rout, NORMAL);
    std::string str = rout.str();
    for (std::string::reverse_iterator it = str.rbegin(); it != str.rend(); ++it)
        out << *it;
}
