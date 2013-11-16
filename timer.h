#ifdef _WIN32
#include <Windows.h>
#else
#include <sys/time.h>
#endif

long int GetTimeInMilliseconds() {
#ifdef _WIN32
    SYSTEMTIME st;
    GetLocalTime(&st);
    return st.wSecond * 1000 + st.wMilliseconds;
#else
    timeval tim;
    gettimeofday(&tim, NULL);
    return tim.tv_sec * 1000 + tim.tv_usec/1000;
#endif
}

int GetTimeInSeconds() {
#ifdef _WIN32
    SYSTEMTIME st;
    GetLocalTime(&st);
	return st.wMinute * 60 + st.wSecond;
#else
    timeval tim;
    gettimeofday(&tim, NULL);
    return tim.tv_sec;
#endif
}
