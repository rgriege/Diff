#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "timer.h"
#include "BlockedData.h"
#include "Lcs.h"

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
    LCS_compute_table(x, strlen(x), y, strlen(y), table);
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