#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#if defined(linux) || defined(__linux) || defined(__linux__)
    #define LINUX_SYS
    // printf("Linux \n");
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(WIN64) || defined(_WIN64) || defined(__WIN64__)
    #define WIN_SYS
    #if defined(WIN64) || defined(_WIN64) || defined(__WIN64__)
        // printf("Windows 64 bit\n");
    #else
        // printf("Windows 32 bit\n");
    #endif
#elif defined(__APPLE__)
    // printf("Apple OS\n");
#elif defined(__ANDROID__)
    // android
    // linux
#elif defined(__unix__ )// all unices not caught above
    // Unix
#elif defined(_POSIX_VERSION)
    // POSIX
#else
#   error "Unknown"
#endif


#include <unistd.h>


#ifdef WIN_SYS
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <string.h>
#include <tchar.h>

typedef __int64            int64;
typedef unsigned __int64   uint64;
typedef SOCKET             net_socket_fd;

#endif

#ifdef LINUX_SYS
#include <arpa/inet.h>
#include <sys/unistd.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <netinet/tcp.h>

#include <pthread.h>
#include <fcntl.h>

typedef int                net_socket_fd;
typedef int                BOOL;
typedef signed long long   int64;
typedef unsigned long long uint64;

#endif



#endif //__PLATFORM_H__