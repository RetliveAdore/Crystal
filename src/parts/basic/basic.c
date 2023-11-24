#include <Crystal.h>
#include <parts/Crbasic.h>
#include <crerrors.h>

CRLVOID CurrentIDthread = (CRLVOID)1;
CRSTRUCTURE threadTree = NULL;
CRSTRUCTURE availableIDthread = NULL;

CRLVOID CurrentIDsocket = (CRLVOID)1;
CRSTRUCTURE socketTree = NULL;
CRSTRUCTURE availableIDsocket = NULL;

#ifdef CR_WINDOWS
#include <Windows.h>
#pragma comment(lib, "winmm.lib")

#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")

LARGE_INTEGER frequency = { 0 };
LARGE_INTEGER count = { 0 };

#elif defined CR_LINUX
#endif

CRBOOL crInitedBasic = CRFALSE;

CRAPI CRCODE CRBasicInit()
{
	if (crInitedBasic)
		return 0;
#ifdef CR_WINDOWS  //几乎不会出错，除非是xp系统
	if (!QueryPerformanceFrequency(&frequency)) return -1;
#endif
	CurrentIDthread = 1;
	threadTree = CRTree();
	if (!threadTree)
		return CRERR_OUTOFMEM;
	availableIDthread = CRLinear();
	if (!availableIDthread)
	{
		CRFreeStructure(threadTree, NULL);
		threadTree = NULL;
		return CRERR_OUTOFMEM;
	}
	//
#ifdef CR_WINDOWS
	WSADATA wsadata;
	if (FAILED(WSAStartup(MAKEWORD(2, 2), &wsadata)))
	{
		CRThrowError(CRERR_BASIC_FAILEDINET, CRDES_BASIC_FAILEDINET);
		return CRERR_BASIC_FAILEDINET;
	}
#endif
	CurrentIDsocket = 1;
	socketTree = CRTree();
	if (!socketTree)
		return CRERR_OUTOFMEM;
	availableIDsocket = CRLinear();
	if (!availableIDsocket)
	{
		CRFreeStructure(socketTree, NULL);
		socketTree = NULL;
		return CRERR_OUTOFMEM;
	}
	//
	crInitedBasic = CRTRUE;
	return 0;
}

CRAPI void CRBasicUninit()
{
	if (!crInitedBasic)
		return;
	CRFreeStructure(threadTree, NULL);
	threadTree = NULL;
	CRFreeStructure(availableIDthread, NULL);
	availableIDthread = NULL;
	CurrentIDthread = (CRLVOID)1;
#ifdef CR_WINDOWS
	WSACleanup();
#endif
	crInitedBasic = CRFALSE;
}