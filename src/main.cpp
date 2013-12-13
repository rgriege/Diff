#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <functional>
#include <cassert>
#ifndef _WIN32
#include <papi.h>
#define NUM_EVENTS 2
#endif

#include "timer.h"
#include "BlockedData.h"
#include "ArrayTable.h"
#include "IndirectRowTable.h"
#include "Source.h"
#include "Lcs.h"
#include "Lcs_gpq.h"

enum test_type {
    CHAR_TEST,
    WORD_TEST,
    LINE_TEST,
    SENT_TEST
};

std::istream* original_input_stream = NULL;
std::istream* modified_input_stream = NULL;
std::ostream* output_stream = NULL;
char* tests = NULL;
unsigned block_size = 56;
bool block_ranged = false;
unsigned num_threads = 1;
bool thread_ranged = false;
test_type type = LINE_TEST;
char delim = '\n';
unsigned size = 0;
bool print_table = false;
bool print_sequence = false;

#ifndef _WIN32
void handle_error(int i)
{
    printf("Encountered error %d\n", i);
    assert(0 > 1);
}
#endif

void transfer_input(std::istream& in, std::ostream& out)
{
    unsigned len = 0;
    char buffer[100];
    do {
        in.getline(buffer, 100);
        out << buffer;
        len += 100;
    } while (strlen(buffer) == 99);
    size = std::max(size, len);
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

void show_help()
{
    std::cout << "Use:" << std::endl;
    std::cout << "  -original=" << std::endl;
    std::cout << "  -modified=" << std::endl;
    std::cout << "  -output=" << std::endl;
    std::cout << "  -tests=[a-f]" << std::endl;
    std::cout << "    a : Loop Order ij" << std::endl;
    std::cout << "    b : Loop Order ji" << std::endl;
    std::cout << "    c : Block Order jiji" << std::endl;
    std::cout << "    d : Block Order ijij" << std::endl;
    std::cout << "    e : Block Order jij" << std::endl;
    std::cout << "    f : Threaded Queue:" << std::endl;
    std::cout << "  -block=[size]" << std::endl;
    std::cout << "  -block_range" << std::endl;
    std::cout << "  -threads=[num]" << std::endl;
    std::cout << "  -thread_range" << std::endl;
    std::cout << "  -type=[char|word|line|sent]" << std::endl;
    std::cout << "  -print_table" << std::endl;
    std::cout << "  -print_sequence" << std::endl;
    std::cout << "  -help" << std::endl;
}

void test(const std::function<void()>& func, const char* title)
{
    *output_stream << "testing " << title << std::endl;
    
#ifndef _WIN32
    {
        int Events[NUM_EVENTS] = { PAPI_L1_DCM, PAPI_L2_TCM };
        long_long values[NUM_EVENTS] = {0, 0};

        if (PAPI_start_counters(Events, NUM_EVENTS) != PAPI_OK)
            handle_error(1);

        func();

        if (PAPI_stop_counters(values, NUM_EVENTS) != PAPI_OK)
            handle_error(1);

        *output_stream << "L1 Data misses: " << values[0] << std::endl;
        *output_stream << "L2 Total misses: " << values[1] << std::endl;

        flush_cache();
    }
    {
        int Events[NUM_EVENTS] = { PAPI_LST_INS, PAPI_L1_ICM };
        long_long values[NUM_EVENTS] = {0, 0};

        if (PAPI_start_counters(Events, NUM_EVENTS) != PAPI_OK)
            handle_error(1);

        func();

        if (PAPI_stop_counters(values, NUM_EVENTS) != PAPI_OK)
            handle_error(1);

        *output_stream << "Total load/stores: " << values[0] << std::endl;
        *output_stream << "L1 Instruction misses: " << values[1] << std::endl;

        flush_cache();
    }
#endif

    long int start = GetTimeInMilliseconds();
    func();
    long int end = GetTimeInMilliseconds();
    *output_stream << "Diff took " << end - start << " ms" << std::endl << std::endl;
}

void read(std::istream& in, Source<char>& source)
{
    in.read(source, source.size());
    source.shrink_to_fit();
}

void read(std::istream& in, Source<std::string>& source)
{
    char buffer[200];
    size_t i = 0;
    do {
        in.getline(buffer, 200, delim);
        source[i] = buffer;
        ++i;
    } while (!in.eof());
    source.shrink_to_fit();
}

void read(std::istream& in, std::vector<char>& source)
{
    char buffer[200];
    do {
        in.getline(buffer, 200);
        source.insert(source.end(), buffer, buffer + in.gcount());
    } while (!in.eof());
    source.shrink_to_fit();
}

void read(std::istream& in, std::vector<std::string>& source)
{
    char buffer[200];
    do {
        in.getline(buffer, 200, delim);
        source.push_back(buffer);
    } while (!in.eof());
    source.shrink_to_fit();
}

template <class Container, class Table>
void run_tests()
{
    long int start = GetTimeInMilliseconds();
    Container x; //(size);
    read(*original_input_stream, x);

    Container y; //(size);
    read(*modified_input_stream, y);
    long int end = GetTimeInMilliseconds();

    *output_stream << "Read took " << (end - start) << " ms" << std::endl;

    int lcs_len;

    *output_stream << "Comparing arrays of size " << x.size() << " and " << y.size() << std::endl << std::endl;

    for (unsigned i = 0; i < strlen(tests); ++i) {
        Table table(x.size() + 1, y.size() + 1);
        switch (tests[i]) {
        case 'a':
            test(std::bind(LCS_compute_table_ij<Container, Table>, std::ref(x), std::ref(y), std::ref(table)), "Loop Order ij:");
            break;
        case 'b':
            test(std::bind(LCS_compute_table_ji<Container, Table>, std::ref(x), std::ref(y), std::ref(table)), "Loop Order ji:");
            break;
        case 'c':
            for (unsigned b = block_ranged ? 64 : block_size; b <= block_size; b *= 2) {
                *output_stream << std::endl << "Block Size: " << b << std::endl;
                test(std::bind(LCS_compute_table_jiji<Container, Table>, std::ref(x), std::ref(y), std::ref(table), b), "Block Order jiji:");
            }
            break;
        case 'd':
            for (unsigned b = block_ranged ? 64 : block_size; b <= block_size; b *= 2) {
                *output_stream << std::endl << "Block Size: " << b << std::endl;
                test(std::bind(LCS_compute_table_ijij<Container, Table>, std::ref(x), std::ref(y), std::ref(table), b), "Block Order ijij:");
            }
            break;
        case 'e':
            for (unsigned b = block_ranged ? 64 : block_size; b <= block_size; b *= 2) {
                *output_stream << std::endl << "Block Size: " << b << std::endl;
                test(std::bind(LCS_compute_table_jij<Container, Table>, std::ref(x), std::ref(y), std::ref(table), b), "Block Order jij:");
            }
            break;
        case 'f':
            for (unsigned b = block_ranged ? 64 : block_size; b <= block_size; b *= 2) {
                *output_stream << std::endl << "Block Size: " << b << std::endl;
                for (unsigned t = thread_ranged ? 1 : num_threads; t <= num_threads; ++t) {
                    *output_stream << "Threads: " << t << std::endl;
                    test(std::bind(LCS_compute_table_gq<Container, Table>, std::ref(x), std::ref(y), std::ref(table), t, b, b), "Threaded Queue:");
                }
            }
            break;
        }
        if (print_sequence)
            LCS_read(x, y, table, *output_stream);
        if (print_table)
            LCS_print_table(x, y, table, *output_stream);
        if (i == 0) {
            lcs_len = LCS_length(table);
            *output_stream << "LCS length: " << lcs_len << std::endl;
        } else if (LCS_length(table) != lcs_len) {
            *output_stream << "failed" << std::endl;
        }
    }
}

int main(int argc, char* argv[])
{
    // parse args
    for (int i = 1; i < argc; ++i) {
        std::string arg(argv[i]);
        if (arg.length() < 2 || arg[0] != '-') {
            std::cout << "Invalid arg " << argv[i] << std::endl;
            return EXIT_FAILURE;
        } else if (arg.find("original=", 1) != arg.npos) {
            original_input_stream = new std::ifstream(arg.substr(10).c_str());
            std::ifstream temp(arg.substr(10).c_str(), std::ios_base::ate);
            size = std::max(size, unsigned(temp.tellg()));
        } else if (arg.find("modified=", 1) != arg.npos) {
            modified_input_stream = new std::ifstream(arg.substr(10).c_str());
            std::ifstream temp(arg.substr(10).c_str(), std::ios_base::ate);
            size = std::max(size, unsigned(temp.tellg()));
        } else if (arg.find("output=", 1) != arg.npos) {
            output_stream = new std::ofstream(arg.substr(8).c_str());
        } else if (arg.find("tests=", 1) != arg.npos) {
            tests = new char[arg.substr(7).length()+1];
            strcpy(tests, arg.substr(7).c_str());
        } else if (arg.find("block=", 1) != arg.npos) {
            block_size = atoi(arg.substr(7).c_str());
        } else if (arg.find("block_range", 1) != arg.npos) {
            block_ranged = true;
        } else if (arg.find("threads=", 1) != arg.npos) {
            num_threads = atoi(arg.substr(9).c_str());
        } else if (arg.find("thread_range", 1) != arg.npos) {
            thread_ranged = true;
        } else if (arg.find("type=", 1) != arg.npos) {
            type = arg.substr(6) == "char" ? CHAR_TEST : arg.substr(6) == "word" ? WORD_TEST : arg.substr(6) == "sent" ? SENT_TEST : LINE_TEST;
        } else if (arg.find("print_table", 1) != arg.npos) {
            print_table = true;
        } else if (arg.find("print_sequence", 1) != arg.npos) {
            print_sequence = true;
        } else if (arg.find("help", 1) != arg.npos) {
            show_help();
            return EXIT_SUCCESS;
        } else {
            std::cout << "Invalid arg " << argv[i] << std::endl;
            return EXIT_FAILURE;
        }
    }

    if (!tests) {
        tests = new char[1];
        tests[0] = 0;
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

    std::cout << std::endl;

    switch(type) {
    case CHAR_TEST:
        *output_stream << "Diffing by character" << std::endl;
        run_tests<std::vector<char>, ArrayTable<int> >();
        break;
    case WORD_TEST:
        *output_stream << "Diffing by word" << std::endl;
        delim = ' ';
        run_tests<std::vector<std::string>, ArrayTable<int> >();
        break;
    case LINE_TEST:
        *output_stream << "Diffing by line" << std::endl;
        delim = '\n';
        run_tests<std::vector<std::string>, ArrayTable<int> >();
        break;
    case SENT_TEST:
        *output_stream << "Diffing by sentence" << std::endl;
        delim = '.';
        run_tests<std::vector<std::string>, ArrayTable<int> >();
        break;
    }

    delete original_input_stream;
    delete modified_input_stream;
    delete[] tests;

#ifdef _DEBUG
    std::cout << "Press Enter to exit...";
    std::cin.ignore(1);
#endif
}
