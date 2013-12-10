#include <iostream>
#include <sstream>
#include <string>

#include "timer.h"
#include "BlockedCstring.h"
#undef max

#define BLOCK_SIZE 56

int cx_len;
int cy_len;

int* LCS_compute_table(BlockedCstring& x, BlockedCstring& y)
{
    int* C = new int[cx_len*cy_len];
    for (int i = 0; i <= x.length(); i++)
        C[i*cy_len] = 0;
    for (int j = 0; j <= y.length(); j++)
        C[j] = 0;
    for (int i = 1; i <= x.length(); i++) {
        for (int j = 1; j <= y.length(); j++) {
            if (x[i-1] == y[j-1])
                C[i*cy_len+j] = C[(i-1)*cy_len+(j-1)] + 1;
            else
                C[i*cy_len+j] = std::max(C[i*cy_len+(j-1)], C[(i-1)*cy_len+j]);
        }
    }
    return C;
}

int LCS_length(int* C, int cx_len, int cy_len)
{
    return C[cx_len*cy_len-1];
}

void LCS_print_table(int* C, BlockedCstring& x, BlockedCstring& y, std::ostream& out)
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

void LCS_read_helper(int*& C, BlockedCstring& x, BlockedCstring& y, int i, int j,
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

void LCS_read(int* C, BlockedCstring& x, BlockedCstring& y, std::ostream& out)
{
    LCS_read_helper(C, x, y, x.length(), y.length(), out, NORMAL);
}

int main()
{
    // TODO - delineate by word

    /*const char* x = "abcfghijk";
    const char* y = "abcdefgjk";*/
    /*const char* x = "compress the size of the changes.";
    const char* y = "compress anything.";*/
    FILE* original_file = fopen("original.txt", "r");
    BlockedCstring x(original_file, BLOCK_SIZE);

    FILE* new_file = fopen("new.txt", "r");
    BlockedCstring y(new_file, BLOCK_SIZE);

    cx_len = x.length() + 1;
    cy_len = y.length() + 1;

    std::cout << "Comparing strings of size " << x.length() << " and " << y.length() << std::endl;
    long int start = GetTimeInMilliseconds();
    int* C = LCS_compute_table(x, y);
    long int end = GetTimeInMilliseconds();
    std::cout << "Op took " << end - start << " ms" << std::endl;

    int lcs_len = LCS_length(C, cx_len, cy_len);
    std::cout << "LCS Length: " <<  lcs_len << std::endl;

    std::ostringstream out;
    LCS_read(C, x, y, out);
    std::string str = out.str();
    std::cout << "LCS Sequence:" <<std::endl;
    for (std::string::reverse_iterator it = str.rbegin(); it != str.rend(); ++it)
        std::cout << *it;
    //LCS_print_table(C, x, y);

    char line[1];
    std::cin.getline(line, 1);
}