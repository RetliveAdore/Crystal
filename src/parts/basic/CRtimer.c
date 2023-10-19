#include <Crystal.h>
#include <parts/Crbasic.h>

#ifdef CR_WINDOWS
#include <Windows.h>
extern LARGE_INTEGER frequency;
extern LARGE_INTEGER count;

#elif defined CR_LINUX
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
static time_t crTime = 0;
static struct tm* ptm = NULL;
static struct timeval ti = { 0 };
static struct timespec ts = { 0 };
#endif

extern const char* errsBasic[];
extern const char* errNowBasic;
extern CRBOOL crInitedBasic;

#define TIMER_MAGIC 0x5b

typedef struct
{
	CRUINT8 magic;
	double last;
}CRTIMERINNER, *PCRTIMERINNER;

CRAPI void CRSleep(CRUINT64 ms)
{
#ifdef CR_WINDOWS
	timeBeginPeriod(1);
	SleepEx((DWORD)ms, TRUE);
	timeEndPeriod(1);
#elif defined CR_LINUX
	usleep(ms * 1000);
#endif
}

CRAPI CRTIMER CRTimer()
{
	PCRTIMERINNER timer = malloc(sizeof(CRTIMERINNER));
	if (timer)
	{
		timer->magic = TIMER_MAGIC;
		return timer;
	}
	CRThrowError(CRERR_OUTOFMEM, NULL);
	return NULL;
}

CRAPI CRCODE CRTimerClose(CRTIMER timer)
{
	PCRTIMERINNER pInner = (PCRTIMERINNER)timer;
	if (!pInner || pInner->magic != TIMER_MAGIC)
		return CRERR_INVALID;
	free(pInner);
	return 0;
}

CRAPI double CRTimerPeek(CRTIMER timer)
{
	if (!crInitedBasic)
	{
		CRThrowError(CRERR_UNINITED, NULL);
		return 0.0f;
	}
#ifdef CR_WINDOWS
	QueryPerformanceCounter(&count);
#elif defined CR_LINUX
	clock_gettime(CLOCK_MONOTONIC, &ts);
#endif
	PCRTIMERINNER pInner = (PCRTIMERINNER)timer;
	if (!pInner || pInner->magic != TIMER_MAGIC)
	{
		CRThrowError(CRERR_INVALID, NULL);
		return 0.0f;
	}
#ifdef CR_WINDOWS
	return (double)(count.QuadPart) / (double)(frequency.QuadPart) - pInner->last;
#elif defined CR_LINUX
	return (double)(ts.tv_sec) + (double)(ts.tv_nsec) / 1000000000 - pInner->last;
#endif
}

CRAPI double CRTimerMark(CRTIMER timer)
{
	if (!crInitedBasic)
	{
		CRThrowError(CRERR_UNINITED, NULL);
		return 0.0f;
	}
#ifdef CR_WINDOWS
	QueryPerformanceCounter(&count);
#elif defined CR_LINUX
	clock_gettime(CLOCK_MONOTONIC, &ts);
#endif
	PCRTIMERINNER pInner = (PCRTIMERINNER)timer;
	if (!pInner || pInner->magic != TIMER_MAGIC)
	{
		CRThrowError(CRERR_INVALID, NULL);
		return 0.0f;
	}
	double now = 0;
#ifdef CR_WINDOWS
	now = (double)(count.QuadPart) / (double)(frequency.QuadPart);
#elif defined CR_LINUX
	now = (double)(ts.tv_sec) + (double)(ts.tv_nsec) / 1000000000;
#endif
	double back = now - pInner->last;
	pInner->last = now;
	return back;
}