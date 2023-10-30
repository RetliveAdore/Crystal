/*
* UI系统需要抹平很多平台差异，实现起来很冗杂，但是
* 一旦成功就会在日后开发流程中嫉妒舒适
* Crystal准备搞一套自己的视窗UI风格，
* 基于无边框窗口和渲染器
*/

#ifndef _INCLUDE_CRUI_H_
#define _INCLUDE_CRUI_H_

#include "../datastructure.h"

#define CRWINDOW_USEDEFAULT (CRUINT32)0xffffffff

//0是无效窗口
typedef CRUINT64 CRWINDOW;

typedef struct
{
	CRWINDOW window;
	union
	{
		CRUINT64 x;
		CRUINT64 w;
	};
	union
	{
		CRUINT64 y;
		CRUINT64 h;
	};
	CRUINT8 keycode;
	//CRUI_STAT_XX
	CRUINT8 status;
}CRUIMSG, *PCRUIMSG;
#define CRUI_STAT_OTHER 0x00
#define CRUI_STAT_UP    0x01
#define CRUI_STAT_DOWN  0x02
#define CRUI_STAT_MOVE  0x04
#define CRUI_STAT_LEFT  0x10
#define CRUI_STAT_NIDD  0x20
#define CRUI_STAT_RIGHT 0x30

#define CRUI_QUIT_CB  0
#define CRUI_PAINT_CB 1
#define CRUI_MOUSE_CB 2
#define CRUI_KEY_CB   3
#define CRUI_FOCUS_CB 4
#define CRUI_SIZE_CB  5
#define CRUI_MOVE_CB  6
#define CALLBACK_FUNCS_NUM 7
//这个被用于设置回调，方晶中的所有信息都被抽象为事件
typedef CRCODE (*CRWindowCallback)(PCRUIMSG msg);

//这个真得初始化了才能用
CRAPI CRCODE CRUIInit();
CRAPI void CRUIUnInit();
//当还有窗体在工作时返回CRFALSE，
CRAPI CRBOOL CRUIOnQuit();

//不要让单次的任务太复杂
//在方晶引擎中，每一个窗体都对应一个独立的线程
CRAPI CRWINDOW CRCreateWindow(const char* title, CRUINT32 x, CRUINT32 y, CRUINT32 w, CRUINT32 h);
CRAPI CRCODE CRCloseWindow(CRWINDOW window);

#endif  //include
