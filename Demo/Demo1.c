#include "Demo.h"
#include <parts/Crbasic.h>
#include <stdio.h>

#define THREAD_NUM 100
#define REPEAT_NUM 100

static int num = 0, cal = 0;

CRLOCK lock = NULL;

void threadfunc1(CRLVOID data, CRTHREAD idThis)
{
	CRLock(lock);
	num += 1;
	CRUnlock(lock);
}

int Demo1(int argc, char** argv)
{
	CRCODE err = 0;
	CRBasicInit();

	lock = CRLockCreate();
	if (!lock)
		goto End;

	num = 0;

	printf("计时器启动...\n");
	CRTIMER timer = CRTimer();
	if (!CRTimerMark(timer))
		printf("报错：%s\n", CRGetError(0));

	CRTHREAD pool[THREAD_NUM];

	for (int j = 0; j < REPEAT_NUM; j++)
	{
		for (int i = 0; i < THREAD_NUM; i++)
			pool[i] = CRThread(threadfunc1, NULL);
		for (int i = 0; i < THREAD_NUM; i++)
			CRWaitThread(pool[i]);
	}

	printf("开启了%d个线程，每一个线程给num加一，结果：num = %d\n", THREAD_NUM * REPEAT_NUM, num);
	double tm = CRTimerMark(timer);
	printf("执行了%.7f秒\n", tm);
	if (err = CRTimerClose(timer))
	{
		printf("报错:%s\n", CRGetError(err));
		return 1;
	}

End:
	CRLockRelease(lock);
	CRBasicUninit();
	return 0;
}