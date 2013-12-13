#include <iostream>
#include <algorithm>
#include <stack>
#include <stdexcept>

#include "TextColor.h"

#undef max
#undef min

//template <class T>
//void LCS_compute_table_ij(T x, T y, Table table)
//{
//    for (size_t i = 0; i < table.height(); i++)
//        table.at(i, 0) = 0;
//    for (size_t j = 0; j < table.width(); j++)
//        table.at(0, j) = 0;
//    for (size_t i = 1; i < table.height(); ++i) {
//        for (size_t j = 1; j < table.width(); ++j) {
//            if (x[i-1] == y[j-1])
//                table.at(i, j) = table.at(i-1, j-1) + 1;
//            else
//                table.at(i, j) = std::max(table.at(i, j-1), table.at(i-1, j));
//        }
//    }
//}

template <class T, class Table>
void LCS_compute_table_ij(T& x, T& y, Table& table)
{
    for (size_t i = 0; i < table.height(); i++)
        table[i][0] = 0;
    for (size_t j = 0; j < table.width(); j++)
        table[0][j] = 0;
    for (size_t i = 1; i < table.height(); ++i) {
        for (size_t j = 1; j < table.width(); ++j) {
            if (x[i-1] == y[j-1])
                table[i][j] = table[i-1][j-1] + 1;
            else
                table[i][j] = std::max(table[i][j-1], table[i-1][j]);
        }
    }
}

template <class T, class Table>
void LCS_compute_table_ji(T& x, T& y, Table& table)
{
    for (size_t i = 0; i < table.height(); i++)
        table[i][0] = 0;
    for (size_t j = 0; j < table.width(); j++)
        table[0][j] = 0;
    for (size_t j = 1; j < table.width(); ++j) {
        for (size_t i = 1; i < table.height(); ++i) {
            if (x[i-1] == y[j-1])
                table[i][j] = table[i-1][j-1] + 1;
            else
                table[i][j] = std::max(table[i][j-1], table[i-1][j]);
        }
    }
}

template <class T, class Table>
void LCS_compute_table_ijij(T& x, T& y, Table& table, int block_size)
{
    for (size_t i = 0; i < table.height(); i++)
        table[i][0] = 0;
    for (size_t j = 0; j < table.width(); j++)
        table[0][j] = 0;
    for (size_t ii = 1; ii < table.height(); ii += block_size) {
        size_t imax = std::min(ii+block_size, x.length());
        for (size_t jj = 1; jj < table.width(); jj += block_size) {
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

template <class T, class Table>
void LCS_compute_table_jiji(T& x, T& y, Table& table, int block_size)
{
    for (size_t i = 0; i < table.height(); i++)
        table[i][0] = 0;
    for (size_t j = 0; j < table.width(); j++)
        table[0][j] = 0;
    for (size_t jj = 1; jj < table.width(); jj += block_size) {
        size_t jmax = std::min(jj+block_size, y.length());
        for (size_t ii = 1; ii < table.height(); ii += block_size) {
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

template <class T, class Table>
void LCS_compute_table_jij(T& x, T& y, Table& table, int block_size)
{
    for (size_t i = 0; i < table.height(); i++)
        table[i][0] = 0;
    for (size_t j = 0; j < table.width(); j++)
        table[0][j] = 0;
    for (size_t jj = 1; jj < table.width(); jj += block_size) {
        size_t jmax = std::min(jj+block_size, y.length());
        for (size_t i = 1; i < table.height(); ++i) {
            for (size_t j = jj; j <= jmax; ++j) {
                if (x[i-1] == y[j-1])
                    table[i][j] = table[i-1][j-1] + 1;
                else
                    table[i][j] = std::max(table[i][j-1], table[i-1][j]);
            }
        }
    }
}

template <class Table>
int LCS_length(const Table& table)
{
    return table[table.height()-1][table.width()-1];
}

template <class T, class Table>
void LCS_print_table(const T& x, const T& y, const Table& table, std::ostream& out)
{
    out << "  0 ";
    for (size_t j = 0; j < y.length(); j++)
        out << y[j] << " ";
    out << std::endl;
    for (size_t i = 0; i < x.length(); i++) {
        out << x[i] << " ";
        for (size_t j = 0; j < table.width(); j++)
            out << table[i][j] << " ";
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

template <class T, class Table>
void LCS_read_helper(const T& x, const T& y, const Table& table, size_t i, size_t j,
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

template <class T, class Table>
void LCS_read_recursive(const T& x, const T& y, const Table& table, std::ostream& out)
{
    std::ostringstream rout;
    LCS_read_helper(x, y, table, x.length(), y.length(), rout, NORMAL);
    std::string str = rout.str();
    for (std::string::reverse_iterator it = str.rbegin(); it != str.rend(); ++it)
        out << *it;
}

struct change {
    OutputMode mode;
    unsigned size;
};

template <class T, class Table>
void LCS_read(const T& x, const T& y, const Table& table, std::ostream& out)
{
    /* populate stack */
    std::stack<change> stack;
    unsigned i = x.length();
    unsigned j = y.length();
    change current_change = {NORMAL, 0};
    while (i != 0 && j != 0) {
        if (x[i-1] == y[j-1]) {
            if (current_change.mode != NORMAL) {
                stack.push(current_change);
                current_change.size = 0;
                current_change.mode = NORMAL;
            }
            ++current_change.size;
            --i;
            --j;
        } else if (table[i][j-1] > table[i-1][j]) {
            if (current_change.mode != INSERTION) {
                stack.push(current_change);
                current_change.size = 0;
                current_change.mode = INSERTION;
            }
            ++current_change.size;
            --j;
        } else {
            if (current_change.mode != DELETION) {
                stack.push(current_change);
                current_change.size = 0;
                current_change.mode = DELETION;
            }
            ++current_change.size;
            --i;
        }
    }
    stack.push(current_change);

    /* reverse */
    store_default();
    while (!stack.empty()) {
        current_change = stack.top();
        stack.pop();
        switch (current_change.mode) {
            case NORMAL:
            {
                unsigned max = j + current_change.size;
                out << DEFAULT;
                for ( ; j < max; ++j)
                    out << y[j];
                i += current_change.size;
                out << std::endl;
                break;
            }
            case INSERTION:
            {
                out << GREEN;
                out << "+";
                unsigned max = j + current_change.size;
                for ( ; j < max; ++j)
                    out << y[j];
                out << std::endl;
                break;
            }
            case DELETION:
            {
                out << RED;
                out << "-";
                unsigned max = i + current_change.size;
                for ( ; i < max; ++i)
                    out << x[i];
                out << std::endl;
                break;
            }
        }
    }
    out << DEFAULT;
}
