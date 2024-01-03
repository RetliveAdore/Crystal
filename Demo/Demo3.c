#include "Demo.h"
#include <parts/CrUI.h>
#include <parts/Crbasic.h>
#include <parts/CrTreeExtra.h>

CRCODE MouseEvent(PCRUIMSG msg)
{
	if (msg->status & CRUI_STAT_DOWN)
		printf("mouse button down: %d, %d\n", msg->x, msg->y);
	return 0;
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
		printf("error: %s\n", CRGetError(code));

	CRUIENTITY entity1;
	entity1.color.r = 1.0f;
	entity1.color.g = 1.0f;
	entity1.color.b = 1.0f;
	entity1.color.a = 1.0f;
	entity1.id = 1;
	entity1.style_s.shape = CRUISHAPE_RECT;
	entity1.style_s.type = CRUISTYLE_COUNTOUR;
	entity1.stroke = 2.0f;
	entity1.sizeBox.left = 0;
	entity1.sizeBox.top = 0;
	entity1.sizeBox.right = 100;
	entity1.sizeBox.bottom = 100;
	entity1.enableVision = CRTRUE;
	if (code = CRWindowEntityAdd(window1, &entity1))
		printf("error: %s\n", CRGetError(code));

	CRUIENTITY entity2;
	entity2.color.r = 1.0f;
	entity2.color.g = 1.0f;
	entity2.color.b = 1.0f;
	entity2.color.a = 1.0f;
	entity2.id = 1;
	entity2.style_s.shape = CRUISHAPE_ELIPSE;
	entity2.style_s.type = CRUISTYLE_FILLED;
	entity2.stroke = 2.0f;
	entity2.sizeBox.left = 0;
	entity2.sizeBox.top = 0;
	entity2.sizeBox.right = 100;
	entity2.sizeBox.bottom = 100;
	entity2.enableVision = CRTRUE;
	if (code = CRWindowEntityAdd(window1, &entity2))
		printf("error: %s\n", CRGetError(code));

	while (CRUIOnQuit()) CRSleep(1);

	CRUIUnInit();
	return 0;
}