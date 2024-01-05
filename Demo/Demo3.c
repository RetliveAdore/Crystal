#include "Demo.h"
#include <parts/CrUI.h>
#include <parts/Crbasic.h>
#include <parts/CrTreeExtra.h>

CRUIENTITY entity1;
CRUIENTITY entity2;

CRCODE MouseEvent(PCRUIMSG msg)
{
	return 0;
}

CRCODE EntityEvent(PCRUIMSG msg)
{
	CRUINT64 id;
	CRDynSeekPtr(msg->lp, (CRLVOID*)&id, 0);
	if (msg->status & CRUI_STAT_DOWN)
	{
		if (id == 1)
		{
			entity1.color.r = 0.5f;
			entity1.color.g = 0.9f;
			entity1.color.b = 0.4f;
			entity1.update = CRTRUE;
		}
		else if (id == 2)
		{
			entity2.color.r = 0.7f;
			entity2.color.g = 0.4f;
			entity2.color.b = 0.9f;
			entity2.update = CRTRUE;
		}
	}
	else if (msg->status & CRUI_STAT_UP)
	{
		if (id == 1)
		{
			entity1.color.r = 0.7f;
			entity1.color.g = 1.0f;
			entity1.color.b = 0.6f;
			entity1.update = CRTRUE;
		}
		else if (id == 2)
		{
			entity2.color.r = 0.8f;
			entity2.color.g = 0.6f;
			entity2.color.b = 1.0f;
			entity2.update = CRTRUE;
		}
	}
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
	if (code = CRSetWindowCbk(window1, EntityEvent, CRUI_ENTITY_CB))
		printf("error: %s\n", CRGetError(code));

	entity1.color.r = 0.7f;
	entity1.color.g = 1.0f;
	entity1.color.b = 0.6f;
	entity1.color.a = 1.0f;
	entity1.id = 1;
	entity1.style_s.shape = CRUISHAPE_RECT;
	entity1.style_s.type = CRUISTYLE_COUNTOUR;
	entity1.stroke = 2.0f;
	entity1.sizeBox.left = 100;
	entity1.sizeBox.top = 0;
	entity1.sizeBox.right = 200;
	entity1.sizeBox.bottom = 100;
	entity1.enableVision = CRTRUE;
	entity1.enableEvent = CRTRUE;
	entity1.level = 2;
	if (code = CRWindowEntityAdd(window1, &entity1))
		printf("error: %s\n", CRGetError(code));

	entity2.color.r = 0.8f;
	entity2.color.g = 0.6f;
	entity2.color.b = 1.0f;
	entity2.color.a = 1.0f;
	entity2.id = 2;
	entity2.style_s.shape = CRUISHAPE_ELIPSE;
	entity2.style_s.type = CRUISTYLE_FILLED;
	entity2.stroke = 2.0f;
	entity2.sizeBox.left = 0;
	entity2.sizeBox.top = 0;
	entity2.sizeBox.right = 100;
	entity2.sizeBox.bottom = 100;
	entity2.enableVision = CRTRUE;
	entity2.enableEvent = CRTRUE;
	entity2.level = 1;
	if (code = CRWindowEntityAdd(window1, &entity2))
		printf("error: %s\n", CRGetError(code));

	while (CRUIOnQuit()) CRSleep(1);

	CRUIUnInit();
	return 0;
}