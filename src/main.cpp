#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "timer.h"
#include "BlockedData.h"
#undef max

int cx_len;
int cy_len;

template <class T>
int* LCS_compute_table(BlockedData<T>& x, BlockedData<T>& y)
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

    BlockedData<char> x(*original_input_stream, block_size);

    BlockedData<char> y(*modified_input_stream, block_size);

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

    std::cout << "Press ENTER to exit...";
    char line[1];
    std::cin.getline(line, 1);
}