#include "Demo.h"
#include <parts/CrUI.h>
#include <parts/Crbasic.h>
#include <parts/CrTreeExtra.h>
#include <malloc.h>
#include <stdlib.h>
#include <time.h>

CRUIENTITY entity1;
CRUIENTITY entity2;
CRUIENTITY entity3;
CRUIENTITY entity4;

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
			entity1.sizeBox.top += 10;
			entity1.sizeBox.bottom += 10;
			entity1.moved = CRTRUE;
		}
		else if (id == 2)
		{
			entity2.style_s.shape = CRUISHAPE_RECT;
			entity2.update = CRTRUE;
			entity2.moved = CRTRUE;
		}
		else if (id == 3)
		{
			entity3.color.r = 0.5f;
			entity3.color.g = 0.5f;
			entity3.color.b = 0.5f;
			entity3.update = CRTRUE;
		}
		else if (id == 4)
		{
			entity4.color.r = 0.5f;
			entity4.color.g = 0.5f;
			entity4.color.b = 0.5f;
			entity4.update = CRTRUE;
		}
	}
	else if (msg->status & CRUI_STAT_UP)
	{
		if (id == 1)
		{
			entity1.sizeBox.top -= 10;
			entity1.sizeBox.bottom -= 10;
			entity1.moved = CRTRUE;
		}
		else if (id == 2)
		{
			entity2.style_s.shape = CRUISHAPE_ELIPSE;
			entity2.update = CRTRUE;
			entity2.moved = CRTRUE;
		}
		else if (id == 3)
		{
			entity3.color.r = 1.0f;
			entity3.color.g = 1.0f;
			entity3.color.b = 1.0f;
			entity3.update = CRTRUE;
		}
		else if (id == 4)
		{
			entity4.color.r = 1.0f;
			entity4.color.g = 1.0f;
			entity4.color.b = 1.0f;
			entity4.update = CRTRUE;
		}
	}
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
	CRWINDOW window1 = CRCreateWindow("Demo1", CRWINDOW_USEDEFAULT, CRWINDOW_USEDEFAULT, 900, 600);
	if (!window1)
		printf("error: %s\n", CRGetError(0));

	if (code = CRSetWindowCbk(window1, MouseEvent, CRUI_MOUSE_CB))
		printf("error: %s\n", CRGetError(code));
	if (code = CRSetWindowCbk(window1, EntityEvent, CRUI_ENTITY_CB))
		printf("error: %s\n", CRGetError(code));

	CRBITMAPINF bmp1;
	bmp1.w = 128;
	bmp1.h = 128;
	bmp1.pixels = malloc(128 * 128 * sizeof(CRCOLORU));
	bmp1.uvRect.left = 0.0f;
	bmp1.uvRect.top = 1.0f;
	bmp1.uvRect.right = 1.0f;
	bmp1.uvRect.bottom = 0.0f;
	srand(time(0));
	CRUINT8 seed = 0;
	for (int i = 0; i < 128; i++)
	{
		for (int j = 0; j < 128; j++)
		{
			seed = rand() % 130;
			bmp1.pixels[i * 128 + j].r = seed + 125;
			bmp1.pixels[i * 128 + j].g = seed + 115;
			bmp1.pixels[i * 128 + j].b = 100;
			bmp1.pixels[i * 128 + j].a = 255;
		}
	}

	entity1.texture = NULL;
	entity1.color.r = 1.0f;
	entity1.color.g = 1.0f;
	entity1.color.b = 1.0f;
	entity1.color.a = 1.0f;
	entity1.id = 1;
	entity1.style_s.shape = CRUISHAPE_RECT;
	entity1.style_s.type = CRUISTYLE_COUNTOUR;
	entity1.stroke = 5.0f;
	entity1.sizeBox.left = 0;
	entity1.sizeBox.top = 0;
	entity1.sizeBox.right = 100;
	entity1.sizeBox.bottom = 100;
	entity1.enableVision = CRTRUE;
	entity1.enableEvent = CRTRUE;
	entity1.level = 2;
	if (code = CRWindowEntityAdd(window1, &entity1))
		printf("error: %s\n", CRGetError(code));

	entity2.texture = &bmp1;
	entity2.color.r = 1.0f;
	entity2.color.g = 1.0f;
	entity2.color.b = 1.0f;
	entity2.color.a = 1.0f;
	entity2.id = 2;
	entity2.style_s.shape = CRUISHAPE_ELIPSE;
	entity2.style_s.type = CRUISTYLE_COUNTOUR;
	entity2.stroke = 5.0f;
	entity2.sizeBox.left = 100;
	entity2.sizeBox.top = 0;
	entity2.sizeBox.right = 200;
	entity2.sizeBox.bottom = 100;
	entity2.enableVision = CRTRUE;
	entity2.enableEvent = CRTRUE;
	entity2.level = 2;
	if (code = CRWindowEntityAdd(window1, &entity2))
		printf("error: %s\n", CRGetError(code));

	entity3.texture = &bmp1;
	entity3.color.r = 1.0f;
	entity3.color.g = 1.0f;
	entity3.color.b = 1.0f;
	entity3.color.a = 1.0f;
	entity3.id = 3;
	entity3.style_s.shape = CRUISHAPE_ELIPSE;
	entity3.style_s.type = CRUISTYLE_FILLED;
	entity3.stroke = 5.0f;
	entity3.sizeBox.left = 200;
	entity3.sizeBox.top = 0;
	entity3.sizeBox.right = 300;
	entity3.sizeBox.bottom = 100;
	entity3.enableVision = CRTRUE;
	entity3.enableEvent = CRTRUE;
	entity3.level = 2;
	if (code = CRWindowEntityAdd(window1, &entity3))
		printf("error: %s\n", CRGetError(code));

	entity4.texture = &bmp1;
	entity4.color.r = 1.0f;
	entity4.color.g = 1.0f;
	entity4.color.b = 1.0f;
	entity4.color.a = 1.0f;
	entity4.id = 4;
	entity4.style_s.shape = CRUISHAPE_RECT;
	entity4.style_s.type = CRUISTYLE_FILLED;
	entity4.stroke = 5.0f;
	entity4.sizeBox.left = 300;
	entity4.sizeBox.top = 0;
	entity4.sizeBox.right = 400;
	entity4.sizeBox.bottom = 100;
	entity4.enableVision = CRTRUE;
	entity4.enableEvent = CRTRUE;
	entity4.level = 2;
	if (code = CRWindowEntityAdd(window1, &entity4))
		printf("error: %s\n", CRGetError(code));
	
	while (CRUIOnQuit()) CRSleep(1);
	free(bmp1.pixels);

	CRUIUnInit();
	return 0;
}