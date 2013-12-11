#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "timer.h"
#include "BlockedData.h"
#undef max

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

void transfer_input(std::istream& in, std::ostream& out)
{
    char buffer[100];
    do {
        in.getline(buffer, 100);
        out << buffer;
    } while (strlen(buffer) == 99);
}

int main(int argc, char* argv[])
{
    int block_size = 56;
    std::istream* original_input_stream = NULL;
    std::istream* modified_input_stream = NULL;
    std::ostream* output_stream = NULL;

    // parse args
    for (int i = 1; i < argc; ++i) {
        std::string arg(argv[i]);
        if (arg.length() < 2 || arg[0] != '-') {
            std::cout << "Invalid arg " << argv[i] << std::endl;
            return EXIT_FAILURE;
        } else if (arg.find("original=", 1) != arg.npos) {
            original_input_stream = new std::ifstream(arg.substr(10).c_str());
        } else if (arg.find("modified=", 1) != arg.npos) {
            modified_input_stream = new std::ifstream(arg.substr(10).c_str());
        } else if (arg.find("output=", 1) != arg.npos) {
            output_stream = new std::ofstream(arg.substr(8).c_str());
        } else if (arg.find("block=", 1) != arg.npos) {
            block_size = atoi(arg.substr(7).c_str());
        } else {
            std::cout << "Invalid arg " << argv[i] << std::endl;
            return EXIT_FAILURE;
        }
    }

    if (original_input_stream == NULL) {
        original_input_stream = new std::stringstream();
        std::cout << "Original string: ";
        transfer_input(std::cin, *static_cast<std::stringstream*>(original_input_stream));
    }

    if (modified_input_stream == NULL) {
        modified_input_stream = new std::stringstream();
        std::cout << "Modified string: ";
        transfer_input(std::cin, *static_cast<std::stringstream*>(modified_input_stream));
    }

    if (output_stream == NULL)
        output_stream = &std::cout;

    int size = 10050;
    char* x = new char[size];
    original_input_stream->read(x, size);
    x[original_input_stream->gcount()] = 0;
    //BlockedData<char> x(*original_input_stream, block_size);

    char* y = new char[size];
    modified_input_stream->read(y, size);
    y[modified_input_stream->gcount()] = 0;
    //BlockedData<char> y(*modified_input_stream, block_size);

    std::cout << "Comparing strings of size " << strlen(x) << " and " << strlen(y) << std::endl;
    long int start = GetTimeInMilliseconds();
    int* table = LCS_create_table(strlen(x), strlen(y));
    LCS_compute_table(x, strlen(x), y, strlen(y), table, block_size);
    long int end = GetTimeInMilliseconds();
    std::cout << "Op took " << end - start << " ms" << std::endl;

    int lcs_len = LCS_length(table, strlen(x)+1, strlen(y)+1);
    std::cout << "LCS Length: " <<  lcs_len << std::endl;

    /*std::ostringstream out;
    LCS_read(C, x, y, out);
    std::string str = out.str();
    std::cout << "LCS Sequence:" <<std::endl;
    for (std::string::reverse_iterator it = str.rbegin(); it != str.rend(); ++it)
    std::cout << *it;*/

    //LCS_print_table(C, x, y);
    delete[] table;
}