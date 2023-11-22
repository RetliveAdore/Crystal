#include "Demo.h"
#include <parts/CrUI.h>
#include <parts/Crbasic.h>

CRCODE MouseEvent(PCRUIMSG msg)
{
	if (msg->status & CRUI_STAT_DOWN)
		printf("mouse button down: %d, %d\n", msg->x, msg->y);
}

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

	if (code = CRSetWindowCbk(window1, MouseEvent, CRUI_MOUSE_CB))
		printf("error: %s\n", CRGetError(0));

	while (CRUIOnQuit()) CRSleep(1);

	CRUIUnInit();
	return 0;
}