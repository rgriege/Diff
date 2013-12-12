#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#ifndef _WIN32
#include <papi.h>
#endif
#include <functional>

#include "timer.h"
#include "BlockedData.h"
#include "Lcs.h"
#include "GlobalQueueScheduler.h"

#define NUM_EVENTS 2

void handle_error(int i)
{
  printf("Encountered error %d\n", i);
  assert(0 > 1);
}

void transfer_input(std::istream& in, std::ostream& out)
{
    char buffer[100];
    do {
        in.getline(buffer, 100);
        out << buffer;
    } while (strlen(buffer) == 99);
}

// L1 data cache size is 32kB
// L2 data cache size is 256kB
void flush_cache()
{
    float temp = 0;
    int size = 256*1024;
    float* mat = new float[size];
    for (int i = 0; i < size; i++)
        temp += mat[i];
    delete[] mat;
}

void test(std::function<void()> func, const char* title)
{
    std::cout << "testing " << title << std::endl;
    
#ifndef _WIN32
    int Events[NUM_EVENTS] = { PAPI_L1_DCM, PAPI_L2_TCM };
    long_long values[NUM_EVENTS] = {0, 0};

    if (PAPI_start_counters(Events, NUM_EVENTS) != PAPI_OK)
      handle_error(1);

    func();

    if (PAPI_stop_counters(values, NUM_EVENTS) != PAPI_OK)
      handle_error(1);

    std::cout << "L1 Data misses: " << values[0] << std::endl;
    std::cout << "L2 Total misses: " << values[1] << std::endl;

    flush_cache();
    Events = { PAPI_LST_INS, PAPI_L1_ICM };
    values = {0, 0};

    if (PAPI_start_counters(Events, NUM_EVENTS) != PAPI_OK)
      handle_error(1);

    func();

    if (PAPI_stop_counters(values, NUM_EVENTS) != PAPI_OK)
      handle_error(1);

    std::cout << "Total load/stores: " << values[0] << std::endl;
    std::cout << "L1 Instruction misses: " << values[1] << std::endl;
#endif

    flush_cache();
    long int start = GetTimeInMilliseconds();
    func();
    long int end = GetTimeInMilliseconds();
    std::cout << "Op took " << end - start << " ms" << std::endl << std::endl;
}

int main(int argc, char* argv[])
{
    int block_size = 56;
    std::istream* original_input_stream = NULL;
    std::istream* modified_input_stream = NULL;
    std::ostream* output_stream = NULL;
    bool ranged = false;

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
        } else if (arg.find("range", 1) != arg.npos) {
            ranged = true;
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
    
    int x_len = strlen(x);
    int y_len = strlen(y);
    std::cout << "Comparing strings of size " << x_len << " and " << y_len << std::endl;

    /*{
        Table<int> table(strlen(x), strlen(y));
        std::cout << "Custom table class:" << std::endl;
        long int start = GetTimeInMilliseconds();
        LCS_compute_table(x, y, table);
        long int end = GetTimeInMilliseconds();
        std::cout << "Op took " << end - start << " ms" << std::endl;

        int lcs_len = LCS_length(table);
        std::cout << "LCS Length: " <<  lcs_len << std::endl;
    }*/

    {
        int* table = LCS_create_table(x_len, y_len);
        test(std::bind(LCS_compute_table_ij<char>, x, x_len, y, y_len, table),
             "Loop Order ij:");

        /*int lcs_len = LCS_length(table, x_len+1, y_len+1);
        std::cout << "LCS Length: " <<  lcs_len << std::endl;*/
        

        std::ostringstream out;
        LCS_read(table, x, y, out);
        std::string str = out.str();
        std::cout << "LCS Sequence:" <<std::endl;
        for (std::string::reverse_iterator it = str.rbegin(); it != str.rend(); ++it)
        std::cout << *it;

        //LCS_print_table(C, x, y);
        delete[] table;
    }

    {
        int* table = LCS_create_table(x_len, y_len);
        test(std::bind(LCS_compute_table_ji<char>, x, x_len, y, y_len, table),
             "Loop Order ji:");

        /*int lcs_len = LCS_length(table, x_len+1, y_len+1);
        std::cout << "LCS Length: " <<  lcs_len << std::endl;*/
        delete[] table;
    }

    for (int i = ranged ? 1 : block_size; i <= block_size; ++i) {
        std::cout << std::endl << "Block Size: " << i << std::endl;
    {
        int* table = LCS_create_table(x_len, y_len);
        test(std::bind(LCS_compute_table_jiji<char>, x, x_len, y, y_len, table, block_size),
             "Block Order jiji:");

        /*int lcs_len = LCS_length(table, x_len+1, y_len+1);
        std::cout << "LCS Length: " <<  lcs_len << std::endl;*/
        delete[] table;
    }

    {
        int* table = LCS_create_table(x_len, y_len);
        test(std::bind(LCS_compute_table_ijij<char>, x, x_len, y, y_len, table, block_size),
             "Block Order ijij:");

        /*int lcs_len = LCS_length(table, x_len+1, y_len+1);
        std::cout << "LCS Length: " <<  lcs_len << std::endl;*/
        delete[] table;
    }

    {
        int* table = LCS_create_table(x_len, y_len);
        test(std::bind(LCS_compute_table_jij<char>, x, x_len, y, y_len, table, block_size),
             "Block Order jij:");

        /*int lcs_len = LCS_length(table, x_len+1, y_len+1);
        std::cout << "LCS Length: " <<  lcs_len << std::endl;*/
        delete[] table;
    }
    }

    //delete[] table;
}
