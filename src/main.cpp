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
#include "ArrayTable.h"
#include "Source.h"
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

void test(const std::function<void()>& func, const char* title)
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

bool keep_char(const char& c) { return c != 0; }

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

    int lcs_len;

    Source<char> x(size);
    original_input_stream->read(x, size);
    x.shrink_to_fit(keep_char);
    //BlockedData<char> x(*original_input_stream, block_size);

    Source<char> y(size);
    modified_input_stream->read(y, size);
    y.shrink_to_fit(keep_char);
    //BlockedData<char> y(*modified_input_stream, block_size);

    std::cout << "Comparing strings of size " << x.length() << " and " << y.length() << std::endl;

    {
        ArrayTable<int> table(x.length() + 1, y.length() + 1);
        test(std::bind(LCS_compute_table_ij<Source<char> >, std::ref(x), std::ref(y), std::ref(table)),
             "Loop Order ij:");

        lcs_len = LCS_length(table);
        std::cout << "LCS Length: " <<  lcs_len << std::endl;

        /*LCS_read(x, y, table, std::cout);
        LCS_print_table(x, y, table, std::cout);*/
    }

    {
        ArrayTable<int> table(x.length() + 1, y.length() + 1);
        test(std::bind(LCS_compute_table_ji<Source<char> >, std::ref(x), std::ref(y), std::ref(table)),
             "Loop Order ji:");

        assert(LCS_length(table) == lcs_len);
    }

    for (int b = ranged ? 1 : block_size; b <= block_size; ++b) {
        std::cout << std::endl << "Block Size: " << b << std::endl;
        {
            ArrayTable<int> table(x.length() + 1, y.length() + 1);
            test(std::bind(LCS_compute_table_jiji<Source<char> >, std::ref(x), std::ref(y), std::ref(table), b),
                 "Block Order jiji:");

            assert(LCS_length(table) == lcs_len);
        }
        {
            ArrayTable<int> table(x.length() + 1, y.length() + 1);
            test(std::bind(LCS_compute_table_ijij<Source<char> >, std::ref(x), std::ref(y), std::ref(table), b),
                 "Block Order ijij:");

            assert(LCS_length(table) == lcs_len);
        }
        {
            ArrayTable<int> table(x.length() + 1, y.length() + 1);
            test(std::bind(LCS_compute_table_jij<Source<char> >, std::ref(x), std::ref(y), std::ref(table), b),
                 "Block Order jij:");

            assert(LCS_length(table) == lcs_len);
        }
    }
}
