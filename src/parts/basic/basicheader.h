#ifndef _INCLUDE_BASICHEADER_H_
#define _INCLUDE_BASICHEADER_H_

#include <Crystal.h>
#include <parts/Crbasic.h>
#include <malloc.h>
#include <crerrors.h>

#define SERVER_MAX_ACCEPT 20

#ifdef CR_WINDOWS
#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")
#include <Windows.h>
#pragma comment(lib, "winmm.lib")

#elif defined CR_LINUX
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include <fcntl.h>
#define closesocket(soc) close(soc)
#endif

#endif  //include
