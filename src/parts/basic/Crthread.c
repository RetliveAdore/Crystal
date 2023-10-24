#include <Crystal.h>
#include <parts/Crbasic.h>
#include <malloc.h>
#include <stdio.h>
extern CRUINT64 CurrentID;
extern CRSTRUCTURE threadTree;
extern CRSTRUCTURE availableIDs;

#ifdef CR_WINDOWS
#include <Windows.h>
#include <process.h>

typedef struct
{
	CRThreadFunction func;
	CRLVOID userData;
	CRTHREAD threadID;
	HWND thread;
	DWORD threadIDw;
}CRTHREADINNER, *PCRTHREADINNER;

typedef struct
{
	CRITICAL_SECTION cs;
	CRBOOL lock;
}CRLOCKINNER, *PCRLOCKINNER;

#elif defined CR_LINUX
#include <pthread.h>

typedef struct
{
	CRThreadFunction func;
	CRLVOID userData;
	CRTHREAD threadID;
	pthread_t thread;
}CRTHREADINNER, *PCRTHREADINNER;

typedef struct
{
	pthread_mutex_t cs;
	CRBOOL lock;
}CRLOCKINNER, *PCRLOCKINNER;
//直接使出那一招，简单方便又快捷
extern void InitializeCriticalSection(pthread_mutex_t* mt);
extern void DeleteCriticalSection(pthread_mutex_t* mt);
extern void EnterCriticalSection(pthread_mutex_t* mt);
extern void LeaveCriticalSection(pthread_mutex_t* mt);
#endif

void* _thread_inner_(void* lp)
{
	PCRTHREADINNER pInner = lp;
	pInner->func(pInner->userData, pInner->threadID);
	CRTreeGet(threadTree, NULL, (CRUINT64)(pInner->threadID));
	CRLinPut(availableIDs, pInner->threadID, 0);
	free(pInner);
	return 0;
}

CRAPI CRTHREAD CRThread(CRThreadFunction func, CRLVOID userData)
{
	if (!func)
	{
		CRThrowError(CRERR_INVALID, NULL);
		return 0;
	}
	if (!threadTree || !availableIDs)
	{
		CRThrowError(CRERR_UNINITED, NULL);
		return 0;
	}
	PCRTHREADINNER pInner = malloc(sizeof(CRTHREADINNER));
	if (!pInner)
	{
		CRThrowError(CRERR_OUTOFMEM, NULL);
		return 0;
	}
	if (CRLinGet(availableIDs, &(pInner->threadID), 0))
		pInner->threadID = (CRTHREAD)CurrentID++;
	pInner->func = func;
	pInner->userData = userData;
	CRTreePut(threadTree, pInner, (CRUINT64)(pInner->threadID));
#ifdef CR_WINDOWS
	pInner->thread = _beginthreadex(NULL, 0, _thread_inner_, pInner, 0, &(pInner->threadIDw));
	if (pInner->thread)
		CloseHandle(pInner->thread);
#elif defined CR_LINUX
	pthread_create(&(pInner->thread), NULL, _thread_inner_, pInner);
	if (pInner->thread)
		pthread_detach(pInner->thread);
#endif
	return pInner->threadID;
}

CRAPI CRCODE CRWaitThread(CRTHREAD thread)
{
	while (!CRTreeSeek(threadTree, NULL, (CRUINT64)thread)) CRSleep(1);
	return 0;
}

CRAPI CRLOCK CRLockCreate()
{
	PCRLOCKINNER lock = malloc(sizeof(CRLOCKINNER));
	if (lock)
	{
		InitializeCriticalSection(&(lock->cs));
		lock->lock = CRFALSE;
		return lock;
	}
	CRThrowError(CRERR_OUTOFMEM, NULL);
	return NULL;
}

CRAPI void CRLockRelease(CRLOCK lock)
{
	PCRLOCKINNER pInner = lock;
	if (!pInner)
	{
		CRThrowError(CRERR_INVALID, NULL);
		return;
	}
	while (pInner->lock) CRSleep(1);
	DeleteCriticalSection(&(pInner->cs));
	free(pInner);
}

CRAPI void CRLock(CRLOCK lock)
{
	PCRLOCKINNER pInner = lock;
	if (!pInner)
		return;
Block:
	while (pInner->lock) CRSleep(1);
	EnterCriticalSection(&(pInner->cs));
	if (pInner->lock)
	{
		LeaveCriticalSection(&(pInner->cs));
		goto Block;
	}
	pInner->lock = CRTRUE;
	LeaveCriticalSection(&(pInner->cs));
}

CRAPI void CRUnlock(CRLOCK lock)
{
	PCRLOCKINNER pInner = lock;
	if (pInner)
		pInner->lock = CRFALSE;
}