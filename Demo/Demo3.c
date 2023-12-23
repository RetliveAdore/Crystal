#include "Demo.h"
#include <parts/CrUI.h>
#include <parts/Crbasic.h>
#include <parts/CrTreeExtra.h>

CRTREEXTRA quad;
CRSTRUCTURE dynP;

CRCODE MouseEvent(PCRUIMSG msg)
{
	if (msg->status & CRUI_STAT_DOWN)
	{
		printf("mouse button down: %d, %d\n", msg->x, msg->y);
		CRPOINTU p = {msg->x, msg->y};
		CRQuadtreeSearch(quad, p, dynP);
		CRLVOID key;
		CRDynSeekPtr(dynP, &key, 0);
		CRDynSetup(dynP, NULL, 0);
		printf("key: %d\n", key);
		CRQuadtreeRemove(quad, key);
	}
	return 0;
}

int Demo3(int argc, char** argv)
{
	quad = CRQuadtree(600, 400, 4);
	dynP = CRDynamicPtr();

	CRRECTU UI_1 = {0, 0, 100, 100};
	CRQuadtreePushin(quad, UI_1, (CRLVOID)51);
	CRRECTU UI_2 = { 100, 0, 200, 100 };
	CRQuadtreePushin(quad, UI_2, (CRLVOID)52);
	CRRECTU UI_3 = { 100, 100, 200, 200 };
	CRQuadtreePushin(quad, UI_3, (CRLVOID)53);
	CRRECTU UI_4 = { 0, 100, 100, 200 };
	CRQuadtreePushin(quad, UI_4, (CRLVOID)54);
	CRRECTU UI_5 = { 200, 100, 300, 200 };
	CRQuadtreePushin(quad, UI_5, (CRLVOID)55);

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

	CRFreeTreextra(&quad, NULL);
	CRFreeStructure(dynP, NULL);

	CRUIUnInit();
	return 0;
}