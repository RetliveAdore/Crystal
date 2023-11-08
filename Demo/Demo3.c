#include "Demo.h"
#include <parts/CrUI.h>
#include <parts/Crbasic.h>

int Demo3(int argc, char** argv)
{
	CRCODE code = 0;
	if (code = CRUIInit())
	{
		printf("error: %s\n", CRGetError(code));
		return 1;
	}
	CRWINDOW window1 = CRCreateWindow("Demo1", CRWINDOW_USEDEFAULT, CRWINDOW_USEDEFAULT, 600, 400);
	if (!window1)
		printf("error: %s\n", CRGetError(0));
	CRSleep(10000);
	CRCloseWindow(window1);

	while (CRUIOnQuit()) CRSleep(1);

	CRUIUnInit();
	return 0;
}