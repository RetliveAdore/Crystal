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
#define CRUI_TITLEBAR_PIXEL 36

//0是无效窗口
typedef CRUINT64 CRWINDOW;

typedef struct
{
	float r;
	float g;
	float b;
	float a;
}CRCOLORF;

typedef struct
{
	CRUINT8 r;
	CRUINT8 g;
	CRUINT8 b;
	CRUINT8 a;
}CRCOLORU;

typedef struct
{
	CRWINDOW window;
	union
	{
		CRINT64 x;
		CRINT64 w;
	};
	union
	{
		CRINT64 y;
		CRINT64 h;
	};
	CRUINT8 keycode;
	//CRUI_STAT_XX
	CRUINT8 status;
	CRLVOID lp;
}CRUIMSG, *PCRUIMSG;
#define CRUI_STAT_OTHER 0x00
#define CRUI_STAT_UP    0x01
#define CRUI_STAT_DOWN  0x02
#define CRUI_STAT_MOVE  0x04
#define CRUI_STAT_LEFT  0x10
#define CRUI_STAT_NIDD  0x20
#define CRUI_STAT_RIGHT 0x30

#define CRUI_QUIT_CB   0
#define CRUI_PAINT_CB  1
#define CRUI_MOUSE_CB  2
#define CRUI_KEY_CB    3
#define CRUI_FOCUS_CB  4
#define CRUI_SIZE_CB   5
#define CRUI_MOVE_CB   6
#define CRUI_ENTITY_CB 7
#define CALLBACK_FUNCS_NUM 8
//这个被用于设置回调，引擎中的所有信息都被抽象为事件
//返回非0值可以打断默认操作
typedef CRCODE (*CRWindowCallback)(PCRUIMSG msg);

//实体，内部带有一套自动化实现，可开启事件响应
//通常建议用来制作UI
typedef struct
{
	union
	{
		CRUINT32 style;
		struct
		{
			CRUINT16 shape;
			CRUINT16 type;
		}style_s;
	};
	CRRECTU sizeBox;
	float stroke;
	CRCOLORF color;
	CRUINT64 level;
	CRUINT64 id;  //当产生控件消息时以key作为控件识别依据，ID一旦确定不可修改
	CRUINT64 key; //控件排序的依据（我有一个使用哈希表直接排序的方法，性能可能不算最优，但在各种情况下性能稳定）
	CRLVOID userdata;
	/*
	* 事件回调和渲染的开关时相互独立的
	* 比如说：可以作为一个普通的图像实体，无事件，仅绘制
	* 也可作为一个透明控件，不绘制，但是有事件回调（点击有反应）
	*/
	CRBOOL enableEvent;  //可以选择是否支持事件回调
	CRBOOL enableVision;  //可以选择是否渲染图像

	CRBOOL update;  //当update为CRTRUE时，从下一帧开始更新实体信息
	CRBOOL moved;  //仅当moved为CRTRUE时，对sizeBox的更改才生效
	CRBOOL invalid;  //当设置为CRTRUE时，实体无效化，将自动从实体池中移除。想要再次使用需要重新添加。
}CRUIENTITY;
#define CRUISHAPE_RECT   (CRUINT16)0x01
#define CRUISHAPE_ELIPSE (CRUINT16)0x02
//
#define CRUISTYLE_COUNTOUR (CRUINT16)0x00
#define CRUISTYLE_FILLED   (CRUINT16)0x01
#define CRUISTYLE_BITMAP   (CRUINT16)0x02
//

#ifdef __cplusplus
extern "C" {
#endif

//这个真得初始化了才能用
CRAPI CRCODE CRUIInit();
CRAPI void CRUIUnInit();
//当还有窗体在工作时返回数量，
CRAPI CRCODE CRUIOnQuit();

//不要让单次的任务太复杂
//在引擎中，每一个窗体都对应一个独立的线程
CRAPI CRWINDOW CRCreateWindow(const char* title, CRUINT32 x, CRUINT32 y, CRUINT32 w, CRUINT32 h);
CRAPI CRCODE CRCloseWindow(CRWINDOW window);

//引擎的UI反应是基于事件触发的，此API用于设置回调来响应事件
CRAPI CRCODE CRSetWindowCbk(CRWINDOW window, CRWindowCallback func, CRUINT8 cbkID);

CRAPI CRCODE CRWindowEntityAdd(CRWINDOW window, CRUIENTITY* pEntity);

#ifdef __cplusplus
}
#endif

#endif  //include
