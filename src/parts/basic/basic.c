#include <Crystal.h>
#include <parts/Crbasic.h>
#include <stdio.h>
CRLVOID CurrentID = (CRLVOID)1;
CRSTRUCTURE threadTree = NULL;
CRSTRUCTURE availableIDs = NULL;

#ifdef CR_WINDOWS
#include <Windows.h>
#pragma comment(lib, "winmm.lib")
LARGE_INTEGER frequency = { 0 };
LARGE_INTEGER count = { 0 };

#elif defined CR_LINUX
#endif

CRBOOL crInitedBasic = CRFALSE;

const char* errsBasic[] =
{
	"Fine\0",
	"please init Crbasic first\0",
	"run out of memory\0",
	"invalid item\0"
};
const char* errNowBasic = NULL;

CRAPI const char* CRErrorBasic(CRCODE errcode)
{
	if (!errNowBasic)
		errNowBasic = errsBasic[0];
	const char* ret = errNowBasic;
	if (errcode && errcode < sizeof(errsBasic) / sizeof(const char*))
		ret = errsBasic[errcode];
	else if (!errcode)
		ret = errNowBasic;
	else
		ret = "invalid errcode!";
	errNowBasic = errsBasic[0];
	return ret;
}

CRAPI CRCODE CRBasicInit()
{
	if (crInitedBasic)
		return 0;
#ifdef CR_WINDOWS  //几乎不会出错，除非是xp系统
	if (!QueryPerformanceFrequency(&frequency)) return -1;
#endif
	threadTree = CRTree();
	if (!threadTree)
		return CRERR_OUTOFMEM;
	availableIDs = CRLinear();
	if (!availableIDs)
	{
		CRFreeStructure(threadTree, NULL);
		threadTree = NULL;
		return CRERR_OUTOFMEM;
	}
	crInitedBasic = CRTRUE;
	return 0;
}

CRAPI void CRBasicUninit()
{
	if (!crInitedBasic)
		return;
	CRFreeStructure(threadTree, NULL);
	threadTree = NULL;
	CRFreeStructure(availableIDs, NULL);
	availableIDs = NULL;
	CurrentID = (CRLVOID)1;
	crInitedBasic = CRFALSE;
}