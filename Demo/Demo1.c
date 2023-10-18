#include "Demo.h"
#include <parts/Crbasic.h>
#include <stdio.h>

int Demo1(int argc, char** argv)
{
	CRBasicInit();
	printf("timer begin...\n");
	CRTIMER timer = CRTimer();
	CRTimerMark(timer);
	CRSleep(1000);
	double tm = CRTimerMark(timer);
	printf("during %.7f seconds\n", tm);
	if (CRTimerClose(timer))
		return 1;
	return 0;
}