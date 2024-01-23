#include "Demo.h"
#include <parts/CrUI.h>
#include <parts/Crbasic.h>
#include <parts/Crbinary.h>
#include <stdio.h>
#include <malloc.h>

int Demo2(int argc, char** argv)
{
	CRCODE code;
	if (code = CRBinaryInit())
		printf("error: %s\n", CRGetError(code));
	if (code = CRUIInit())
		printf("error: %s\n", CRGetError(code));

	CRSTRUCTURE bmp1 = CRDynamic();
	CRBMPINF inf;
	code = CRLoadBmp("./resource/uuz.bmp", bmp1, &inf);
	if (code)
		printf("error: %s\n", CRGetError(code));

	printf("宽: %d\n", inf.width);
	printf("高: %d\n", inf.height);
	printf("大小: %d\n", inf.sizeImage);

	CRUINT32 w = inf.width * 3 / 4;
	CRUINT32 h = inf.height * 3 / 4;
	CRWINDOW picWindow = CRCreateWindow("show picture", CRWINDOW_USEDEFAULT, CRWINDOW_USEDEFAULT, w, h);

	CRBITMAPINF bmi;
	bmi.w = inf.width;
	bmi.h = inf.height;
	bmi.pixels = CRDynCopy(bmp1, NULL);
	bmi.uvRect.left = 0.0f;
	bmi.uvRect.top = 0.0f;
	bmi.uvRect.right = 1.0f;
	bmi.uvRect.bottom = 1.0f;
	CRFreeStructure(bmp1, NULL);

	CRUIENTITY show;
	show.texture = &bmi;
	show.color.r = 1.0f;
	show.color.g = 1.0f;
	show.color.b = 1.0f;
	show.color.a = 1.0f;
	show.enableVision = CRTRUE;
	show.enableEvent = CRFALSE;
	show.level = 0;
	show.style_s.shape = CRUISHAPE_RECT;
	show.style_s.type = CRUISTYLE_FILLED;
	show.sizeBox.left = 0;
	show.sizeBox.top = 0;
	show.sizeBox.right = w;
	show.sizeBox.bottom = h;
	CRWindowEntityAdd(picWindow, &show);

	while (CRUIOnQuit()) CRSleep(1);

	CRDynFreeCopy(bmi.pixels);

Failed:
	CRBinaryUninit();
	CRUIUnInit();
	return 0;
}