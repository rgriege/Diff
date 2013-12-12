#ifndef __TEXT_COLOR_H__
#define __TEXT_COLOR_H__

#include <ostream>

enum Color {
    DEFAULT,
    RED,
    GREEN
};

#ifdef _WIN32

static unsigned short default_color = 0;

static HANDLE hstdout = GetStdHandle(STD_OUTPUT_HANDLE);

void store_default()
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hstdout, &csbi);
    default_color = csbi.wAttributes;
}

std::ostream& operator<<(std::ostream& out, Color c)
{
    unsigned short wAttributes;
    switch (c) {
    case DEFAULT:
        wAttributes = default_color;
        break;
    case RED:
        wAttributes = 12;
        break;
    case GREEN:
        wAttributes = 10;
        break;
    }
    SetConsoleTextAttribute(hstdout, wAttributes);
    return out;
}

#else // Linux

void store_default() {}

std::ostream& operator<<(std::ostream& out, Color c)
{
    switch(c) {
    case DEFAULT:
        out << "\033[0m\n";
        break;
    case RED:
        out << "\033[1;31m";
        break;
    case GREEN:
        out << "\033[1;32m";
        break;
    }
    return out;
}

#endif

#endif