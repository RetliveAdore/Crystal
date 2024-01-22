#define CRENTRY_CRT
#include <Crystal.h>
#include <parts/CrUI.h>
#include <parts/Crbasic.h>
#include <parts/Crbinary.h>
#include <parts/CrAudio.h>

#define WINDOW_H 400
#define WINDOW_W 600

CRUIENTITY button1;
CRBOOL b1Pressed = CRFALSE;
CRUIENTITY barLeft;
CRUIENTITY barRight;
CRUIENTITY back1;
float progress = 0.0f;  //0~1的浮点数，用于表示进度
CRUIENTITY button2;
CRBOOL pause = CRTRUE;
CRBOOL b2Pressed = CRFALSE;
CRBOOL update = CRFALSE;

CRWWINFO streamInf;
CRSTRUCTURE audioStream = 0;
CRUINT32 streamOffset = 0;
CRUINT32 frameSize;
CRUINT32 frameCounts;
CRAUDIOPLAY player = 0;
CRBOOL audioEnd = CRFALSE;

CRCODE playerEntityEvent(PCRUIMSG msg)
{
	CRUINT64 id;
	CRDynSeekPtr(msg->lp, (CRLVOID*)&id, 0);
	if (msg->status & CRUI_STAT_DOWN)
	{
		if (id == 1)
			b1Pressed = CRTRUE;
		if (id == 2)
		{
			b2Pressed = CRTRUE;
			button2.color.r = 0.3f;
			button2.color.g = 0.3f;
			button2.color.b = 0.3f;
			button2.update = CRTRUE;
		}
	}
	if (msg->status & CRUI_STAT_UP)
	{
		if (id == 2 && b2Pressed)
		{
			b2Pressed = CRFALSE;
			if (pause)
			{
				pause = CRFALSE;
				CRAudioResume(player);
				button2.color.r = 0.2f;
				button2.color.g = 0.6f;
				button2.color.b = 0.2f;
				button2.update = CRTRUE;
			}
			else
			{
				pause = CRTRUE;
				CRAudioPause(player);
				button2.color.r = 0.6f;
				button2.color.g = 0.2f;
				button2.color.b = 0.2f;
				button2.update = CRTRUE;
			}
		}
	}
	return 0;
}

void playerStream(CRUINT8* buffer, CRUINT32 frames, CRUINT32 size)
{
	CRUINT32 offsetCopy = streamOffset;
	for (int i = 0; i < size; i++) CRDynSeek(audioStream, (CRUINT8*)&buffer[i], offsetCopy + i);
	streamOffset += size;
	if (streamOffset > CRStructureSize(audioStream))
		audioEnd = CRTRUE;
}

CRCODE playerMouseEvent(PCRUIMSG msg)
{
	if (msg->status & CRUI_STAT_MOVE)
	{
		if (b1Pressed)
		{
			if (msg->x <= 35) //限位（贴合进度条）
				msg->x = 35;
			if (msg->x >= WINDOW_W - 35)
				msg->x = WINDOW_W - 35;
			button1.sizeBox.left = msg->x - 5;
			button1.sizeBox.right = msg->x + 5;
			button1.moved = CRTRUE;
			barLeft.sizeBox.right = msg->x;
			barRight.sizeBox.left = msg->x;
			barLeft.moved = CRTRUE;
			barRight.moved = CRTRUE;
		}
	}
	else if (msg->status & CRUI_STAT_UP)
	{
		if (b1Pressed)
		{
			b1Pressed = CRFALSE;
			if (msg->x <= 35) //限位（贴合进度条）
				msg->x = 35;
			if (msg->x >= WINDOW_W - 35)
				msg->x = WINDOW_W - 35;
			progress = (float)(msg->x - 35) / (float)(WINDOW_W - 70);
			streamOffset = frameSize * (CRUINT32)(frameCounts * progress);
		}
		update = CRTRUE;
	}
	return 0;
}

int main(int argc, char** argv)
{
	if (CRInit())
		return -1;
	if (CRUIInit())
		return 1;
	if (CRBasicInit())
		return 1;
	if (CRAudioInit())
		return 1;

	//加载音频
	audioStream = CRDynamic();
	if (CRLoadWave("./resource/au2.wav", audioStream, &streamInf))
		return 2;

	streamOffset = 0;
	frameSize = (streamInf.BitsPerSample >> 3) * streamInf.NumChannels;
	frameCounts = CRStructureSize(audioStream) / frameSize;
	audioEnd = CRFALSE;

	player = CRAudioStream(&streamInf, playerStream);
	//
	CRWINDOW playerWindow = CRCreateWindow("player", CRWINDOW_USEDEFAULT, CRWINDOW_USEDEFAULT, WINDOW_W, WINDOW_H);
	if (!playerWindow)
		return 3;
	button1.texture = NULL;
	button1.color.r = 0.7f;
	button1.color.g = 0.3f;
	button1.color.b = 0.8f;
	button1.color.a = 1.0f;
	button1.id = 1;
	button1.style_s.shape = CRUISHAPE_ELIPSE;
	button1.style_s.type = CRUISTYLE_FILLED;
	button1.enableEvent = CRTRUE;
	button1.enableVision = CRTRUE;
	button1.sizeBox.left = 30;
	button1.sizeBox.right = 40;
	button1.sizeBox.top = WINDOW_H - 70;
	button1.sizeBox.bottom = WINDOW_H - 60;
	button1.level = 10;
	CRWindowEntityAdd(playerWindow, &button1);

	barLeft.texture = NULL;
	barLeft.color.r = 0.7f;
	barLeft.color.g = 0.65f;
	barLeft.color.b = 0.8f;
	barLeft.color.a = 1.0f;
	barLeft.id = 0;
	barLeft.style_s.shape = CRUISHAPE_RECT;
	barLeft.style_s.type = CRUISTYLE_FILLED;
	barLeft.enableEvent = CRFALSE;
	barLeft.enableVision = CRTRUE;
	barLeft.sizeBox.left = 30;
	barLeft.sizeBox.right = 30 + progress * (WINDOW_W - 60);
	barLeft.sizeBox.top = WINDOW_H - 67;
	barLeft.sizeBox.bottom = WINDOW_H - 63;
	barLeft.level = 1;
	CRWindowEntityAdd(playerWindow, &barLeft);

	barRight.texture = NULL;
	barRight.color.r = 0.85f;
	barRight.color.g = 0.85f;
	barRight.color.b = 0.8f;
	barRight.color.a = 1.0f;
	barRight.id = 0;
	barRight.style_s.shape = CRUISHAPE_RECT;
	barRight.style_s.type = CRUISTYLE_FILLED;
	barRight.enableEvent = CRFALSE;
	barRight.enableVision = CRTRUE;
	barRight.sizeBox.left = 30 + progress * (WINDOW_W - 60);
	barRight.sizeBox.right = WINDOW_W - 30;
	barRight.sizeBox.top = WINDOW_H - 67;
	barRight.sizeBox.bottom = WINDOW_H - 63;
	barRight.level = 1;
	CRWindowEntityAdd(playerWindow, &barRight);

	back1.texture = NULL;
	back1.color.r = 0.2f;
	back1.color.g = 0.2f;
	back1.color.b = 0.2f;
	back1.color.a = 1.0f;
	back1.id = 0;
	back1.style_s.shape = CRUISHAPE_RECT;
	back1.style_s.type = CRUISTYLE_FILLED;
	back1.enableEvent = CRFALSE;
	back1.enableVision = CRTRUE;
	back1.sizeBox.left = 0;
	back1.sizeBox.right = WINDOW_W;
	back1.sizeBox.top = WINDOW_H - 85;
	back1.sizeBox.bottom = WINDOW_H;
	back1.level = 0;
	CRWindowEntityAdd(playerWindow, &back1);

	button2.texture = NULL;
	button2.color.r = 0.6f;
	button2.color.g = 0.2f;
	button2.color.b = 0.2f;
	button2.color.a = 1.0f;
	button2.id = 2;
	button2.style_s.shape = CRUISHAPE_ELIPSE;
	button2.style_s.type = CRUISTYLE_FILLED;
	button2.enableEvent = CRTRUE;
	button2.enableVision = CRTRUE;
	button2.sizeBox.left = WINDOW_W / 2 - 20;
	button2.sizeBox.right = WINDOW_W / 2 + 20;
	button2.sizeBox.top = WINDOW_H - 50;
	button2.sizeBox.bottom = WINDOW_H - 10;
	button2.level = 10;
	CRWindowEntityAdd(playerWindow, &button2);

	CRSetWindowCbk(playerWindow, playerEntityEvent, CRUI_ENTITY_CB);
	CRSetWindowCbk(playerWindow, playerMouseEvent, CRUI_MOUSE_CB);

	while (CRUIOnQuit())
	{
		if (!pause && !b1Pressed)
		{
			progress = (float)streamOffset / (float)CRStructureSize(audioStream);
			CRUINT32 current = progress * (WINDOW_W - 70) + 35;
			button1.sizeBox.left = current - 5;
			button1.sizeBox.right = current + 5;
			barLeft.sizeBox.right = current;
			barRight.sizeBox.left = current;
			button1.moved = CRTRUE;
			barLeft.moved = CRTRUE;
			barRight.moved = CRTRUE;
		}
		if (audioEnd)
		{
			CRAudioPause(player);
			pause = CRTRUE;
			audioEnd = CRFALSE;
			progress = 0.0f;
			streamOffset = 0;
			update = CRTRUE;
		}
		if (update)
		{
			update = CRFALSE;
			if (pause)
			{
				button2.color.r = 0.6f;
				button2.color.g = 0.2f;
				button2.color.b = 0.2f;
				button2.update = CRTRUE;
			}
			else
			{
				button2.color.r = 0.2f;
				button2.color.g = 0.6f;
				button2.color.b = 0.2f;
				button2.update = CRTRUE;
			}
		}
		else CRSleep(1);
	}
	CRAudioClose(player);
	CRFreeStructure(audioStream, NULL);

	CRUIUnInit();
	CRBasicUninit();
	CRAudioUnInit();
	return 0;
}