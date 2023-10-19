#include "Demo.h"
#include <parts/Crbasic.h>
#include <stdio.h>

int Demo1(int argc, char** argv)
{
	CRCODE err = 0;
	CRBasicInit();
	printf("计时器启动...\n");
	CRTIMER timer = CRTimer();
	if (!CRTimerMark(timer))
		printf("报错：%s\n", CRGetError(0));
	CRSleep(1000);
	double tm = CRTimerMark(timer);
	printf("休眠了%.7f秒\n", tm);
	if (err = CRTimerClose(timer))
	{
		printf("报错:%s\n", CRGetError(err));
		return 1;
	}
	return 0;
}