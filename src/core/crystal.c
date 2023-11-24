#include <Crystal.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef CR_WINDOWS
#include <Windows.h>
CRITICAL_SECTION cs;

#elif defined CR_LINUX
#include <pthread.h>
//直接使出那一招，简单方便又快捷
extern void InitializeCriticalSection(pthread_mutex_t* mt);
extern void DeleteCriticalSection(pthread_mutex_t* mt);
extern void EnterCriticalSection(pthread_mutex_t* mt);
extern void LeaveCriticalSection(pthread_mutex_t* mt);
pthread_mutex_t cs;
#endif

CRSTRUCTURE bin = NULL;
static CRSTRUCTURE errTree = NULL;

static const char* errs[] =
{
	"Fine\0",
	"please init the module first\0",
	"invalid input\0",
	"your item not found\0",
	"out of menory\0",
	"item conflict\0"
};
static const char* errNow = NULL;

//编译时写死在库中，可以用于验证真正的二进制版本
const CRVERSION ver =
{
	.v16[0] = CRV_MAJOR,
	.v16[1] = CRV_MINOR,
	.v16[2] = CRV_BUILD,
	.v16[3] = CRV_REVIS
};
//版本控制

static void _clear_callback_(CRLVOID data)
{
	((TrashBinFunc)data)();
}

static void _cr_uninit_(void)
{
	CRFreeStructure(errTree, NULL);
	errTree = NULL;
}

static void _on_close_(void)
{
	CRFreeStructure(bin, _clear_callback_);
	bin = NULL;
	_cr_uninit_();
	DeleteCriticalSection(&cs);
}

CRAPI CRCODE CRInit()
{
	errNow = errs[0];
	bin = CRLinear();
	if (!bin)
	{
		errNow = errs[CRERR_OUTOFMEM];
		return -1;
	}
	errTree = CRTree();
	if (!errTree)
	{
		CRFreeStructure(bin, NULL);
		bin = NULL;
		errNow = errs[CRERR_OUTOFMEM];
		return -1;
	}
	InitializeCriticalSection(&cs);
	atexit(_on_close_);
	return 0;
}

CRAPI const CRVERSION* CRVer()
{
	printf("Crystal Version: %d.%d.%d.%d\n", CRV_MAJOR, CRV_MINOR, CRV_BUILD, CRV_REVIS);
	return &ver;
}

CRAPI CRCODE CRAddtoTrashBin(TrashBinFunc WipeYourAss)
{
	if (!bin)
		return CRERR_UNINITED;
	if (!WipeYourAss)
		return CRERR_INVALID;
	CRCODE code = 0;
	CRLinPut(bin, WipeYourAss, 0);
	return 0;
}

CRAPI CRBOOL CRThrowError(CRCODE errcode, const char* description)
{
	EnterCriticalSection(&cs);
	if (!errTree)
	{
		errNow = errs[CRERR_UNINITED];
		LeaveCriticalSection(&cs);
		return CRFALSE;
	}
	errNow = description;
	if (errcode > 0 && errcode <= 1000 && errcode < CRERR_MAXCODE)
		errNow = errs[errcode];
	else
		CRTreePut(errTree, (void*)description, (CRUINT64)errcode);
	LeaveCriticalSection(&cs);
	return CRTRUE;
}

CRAPI const char* CRGetError(CRCODE errcode)
{
	EnterCriticalSection(&cs);
	if (!errNow)
		errNow = errs[0];
	const char* back = errs[0];
	if (!errcode)
	{
		back = errNow;
		errNow = errs[0];
	}
	else if (errcode <= 1000)
	{
		if (errcode && errcode < sizeof(errs) / sizeof(const char*))
			back = errs[errcode];
	}
	else
	{
		if (CRTreeGet(errTree, (void*)&back, (CRUINT64)errcode))
			back = errs[0];
	}
	LeaveCriticalSection(&cs);
	return back;
}