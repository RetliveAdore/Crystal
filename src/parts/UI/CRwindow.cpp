#include <Crystal.h>
#include <parts/Crbasic.h>
#include <parts/CrUI.h>
#include <crerrors.h>
#include <cropgl.hpp>

// 全局变量
static CRSTRUCTURE windowPool = nullptr;	  // tree
static CRSTRUCTURE availableID = nullptr; // queue
static CRUINT64 CurrentID = 1;
static CRBOOL inited = CRFALSE;
static CRBOOL wndclass = CRFALSE;
// 什么也不做，仅用于防止空指针
CRCODE _do_nothing_(PCRUIMSG msg) { return 0; }

// 试着将主循环也纳入线程托管，用户负责添加内容就好
void _window_thread_(CRLVOID data, CRTHREAD idThis);
void _paint_thread_(CRLVOID data, CRTHREAD idThis);

// 用于被继承的主类
class CRWindow
{
public:
	//一些具有共通性的函数
	void SetMinmax(CRUINT32 minx, CRUINT32 miny, CRUINT32 maxx, CRUINT32 maxy);
	void follow(CRBOOL ifFollow);
	void setcbk(CRWindowCallback func, CRUINT8 cbkID);
	void SetID(CRWINDOW id);
	CRWINDOW GetID();
	CRBOOL follow_stat();

	/*
	 * 都是一些纯虚函数，要在下面实现多态
	 */

	virtual void quit() = 0;
	virtual void clear() = 0;
	virtual void Text(const char *text) = 0; // 如果要设置窗口标题的话，使用该函数
	virtual void Move(CRINT32 x, CRINT32 y, CRUINT32 w, CRUINT32 h) = 0;
	virtual CRCODE Entity(CRUIENTITY* pEntity) = 0;

public:
	CRWindow() {}
	CRWindow(const char *title,
			   CRUINT32 w, CRUINT32 h,
			   CRINT32 x, CRINT32 y)
	{
	}
	// 禁止拷贝
	CRWindow &operator=(const CRWindow &) = delete;
	CRWindow(const CRWindow &) = delete;
	~CRWindow() {}
	// 公开变量，可以迅速设置是否限制大小
	CRBOOL sizeLimit = CRFALSE;
	CRWindowCallback funcs[CALLBACK_FUNCS_NUM] =
		{
			_do_nothing_,
			_do_nothing_,
			_do_nothing_,
			_do_nothing_,
			_do_nothing_,
			_do_nothing_,
			_do_nothing_,
			_do_nothing_,
		};

protected:
	CRWINDOW idThis = 0;
	CRUINT32 _maxx = 0, _maxy = 0;
	CRUINT32 _minx = 0, _miny = 0;

	//用于决定是否拖动窗口
	CRBOOL move = CRFALSE;
};

void CRWindow::SetMinmax(CRUINT32 minx, CRUINT32 miny, CRUINT32 maxx, CRUINT32 maxy)
{
	if (maxx < minx)
		maxx = minx;
	if (maxy < miny)
		maxy = miny;
	_minx = minx, _miny = miny;
	_maxx = maxx, _maxy = maxy;
}

void CRWindow::follow(CRBOOL ifFollow)
{
	move = ifFollow;
}

CRBOOL CRWindow::follow_stat()
{
	return move;
}

void CRWindow::setcbk(CRWindowCallback func, CRUINT8 cbkID)
{
	funcs[cbkID] = func;
}

CRWINDOW CRWindow::GetID()
{
	return idThis;
}

void CRWindow::SetID(CRWINDOW id)
{
	idThis = id;
}

//
////////////////////////////////////////////////////////////
#ifdef CR_WINDOWS // Windows partition
#include <Windows.h>
#include <windowsx.h>

void _move_thread_(CRLVOID data, CRTHREAD idThis);

typedef struct cr_windowthread_inf
{
	HWND hWnd;
	void *pThis;
	CRUINT32 fps;
	const char *title;
	CRUINT32 w, h;
	CRINT32 x, y;
	CRUINT32 dx, dy;
	CRBOOL onQuit;
	ccl_gl* pgl;  // 负责绘制的类
	CRTHREAD windowThread;
	CRTHREAD paintThread;
	CRTHREAD moveThread;
	//用于标题栏按钮判断
	CRBOOL preClose;
} CRWINDOWINF;

class CRWindowWindows : public CRWindow
{
public:
	virtual void quit();
	virtual void clear();
	virtual void Text(const char *text);
	virtual void Move(CRINT32 x, CRINT32 y, CRUINT32 w, CRUINT32 h);
	virtual CRCODE Entity(CRUIENTITY* pEntity);

public:
	CRWindowWindows(const char *title,
					  CRUINT32 w, CRUINT32 h,
					  CRINT32 x, CRINT32 y);
	~CRWindowWindows();
	// 禁止拷贝
	CRWindowWindows &operator=(const CRWindowWindows &) = delete;
	CRWindowWindows(const CRWindowWindows &) = delete;

private:
	CRWINDOWINF inf;

	friend LRESULT AfterProc(HWND, UINT, WPARAM, LPARAM, CRWindowWindows *);
	friend LRESULT WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
};

CRWindowWindows::CRWindowWindows(
	const char* title,
	CRUINT32 w, CRUINT32 h,
	CRINT32 x, CRINT32 y)
{
	inf.pThis = this;
	inf.onQuit = CRFALSE;
	inf.preClose = CRFALSE;
	inf.fps = 60;
	inf.title = title;

	CRUINT32 s_w = GetSystemMetrics(SM_CXSCREEN);
	CRUINT32 s_h = GetSystemMetrics(SM_CYSCREEN);

	if (w == CRWINDOW_USEDEFAULT)
		w = (s_w > s_h ? s_w : s_h) / 2;
	if (h == CRWINDOW_USEDEFAULT)
		h = (s_w > s_h ? s_w : s_h) * 3 / 4;
	if (x == CRWINDOW_USEDEFAULT)
		x = (s_w - w) / 2;
	if (y == CRWINDOW_USEDEFAULT)
		y = (s_h - h) / 2;

	inf.w = w;
	inf.h = h + CRUI_TITLEBAR_PIXEL;
	inf.x = x;
	inf.y = y;
	inf.pgl = nullptr;

	inf.windowThread = CRThread(_window_thread_, &inf);
	inf.paintThread = CRThread(_paint_thread_, &inf);
	inf.moveThread = CRThread(_move_thread_, &inf);
}

CRWindowWindows::~CRWindowWindows()
{}

void CRWindowWindows::quit()
{
	inf.onQuit = CRTRUE;
}

void CRWindowWindows::clear()
{
	PostMessage(inf.hWnd, WM_CLOSE, 0, 0);
}

void CRWindowWindows::Text(const char *text)
{
	SetWindowText(inf.hWnd, text);
}

void CRWindowWindows::Move(CRINT32 x, CRINT32 y, CRUINT32 w, CRUINT32 h)
{
	MoveWindow(inf.hWnd, x, y, w, h, TRUE);
}

CRCODE CRWindowWindows::Entity(CRUIENTITY* pEntity)
{
	while (!inf.pgl) CRSleep(1);
	return inf.pgl->AddEntity(pEntity);
}

static MSG w_msg = {0};

void msgloop()
{
	if (GetMessage(&w_msg, NULL, 0, 0))
	{
		TranslateMessage(&w_msg);
		DispatchMessage(&w_msg);
	}
}

void _window_thread_(CRLVOID data, CRTHREAD idThis)
{
	CRWINDOWINF* inf = (CRWINDOWINF*)data;
	// 创建这个窗口的部分
	inf->hWnd = CreateWindow(TEXT("crwndclass"), inf->title,
							 WS_POPUP, inf->x, inf->y, inf->w, inf->h,
							 NULL, NULL, GetModuleHandle(NULL),
							 inf->pThis);
	UpdateWindow(inf->hWnd);
	ShowWindow(inf->hWnd, SW_SHOWDEFAULT);

	// 维护这个窗口的部分
	while (!inf->onQuit)
	{
		if (GetMessage(&w_msg, NULL, 0, 0))
		{
			TranslateMessage(&w_msg);
			DispatchMessage(&w_msg);
		}
	}
	CRWINDOW id = ((CRWindow*)inf->pThis)->GetID();
	CRTreeGet(windowPool, NULL, id);
	CRLinPut(availableID, (CRLVOID)id, 0);
	delete inf->pgl;
	delete inf->pThis;
}

void _paint_thread_(CRLVOID data, CRTHREAD idThis)
{
	CRWINDOWINF* inf = (CRWINDOWINF*)data;
	CRTIMER timer = CRTimer();
	CRTimerMark(timer);
	while (!inf->onQuit)
	{
		if (CRTimerPeek(timer) >= (float)1 / (float)(inf->fps))
		{
			CRTimerMark(timer);
			SendMessage(inf->hWnd, WM_PAINT, 0, 0);
		}
		CRSleep(1);
	}
	CRTimerClose(timer);
}

void _move_thread_(CRLVOID data, CRTHREAD idThis)
{
	CRWINDOWINF* inf = (CRWINDOWINF*)data;
	CRWindow* pWindow = (CRWindow*)inf->pThis;
	CRTIMER timer = CRTimer();
	while (!inf->onQuit)
	{
		if (pWindow->follow_stat())
		{
			POINT p;
			RECT r;
			GetCursorPos(&p);
			GetClientRect(inf->hWnd, &r);
			MoveWindow(inf->hWnd, p.x - inf->dx, p.y - inf->dy, r.right, r.bottom, TRUE);
		}
		CRSleep(1);
	}
}

LRESULT AfterProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, CRWindowWindows *pThis)
{
	CRUIMSG cbinfo;
	cbinfo.window = pThis->GetID();
	cbinfo.x = GET_X_LPARAM(lParam);
	cbinfo.y = GET_Y_LPARAM(lParam) - CRUI_TITLEBAR_PIXEL;
	cbinfo.keycode = wParam & 0xff;
	cbinfo.status = CRUI_STAT_OTHER;
	CRPOINTU point;
	if (msg == WM_CLOSE)
	{
		if (!pThis->funcs[CRUI_QUIT_CB](&cbinfo))
		{
			DestroyWindow(hWnd);
			return 0;
		}
	}
	else if (msg == WM_DESTROY)
	{
		pThis->quit();
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	// else
	switch (msg)
	{
	case WM_PAINT:
	{
		// 绘制过程并不直接向外暴露，
		// 用户通过添加实体对象并改其属性来控制画面
		cbinfo.x = pThis->inf.w;
		cbinfo.y = pThis->inf.h;
		LRESULT ret = DefWindowProc(hWnd, msg, wParam, lParam);

		pThis->inf.pgl->PaintAll();
		pThis->funcs[CRUI_PAINT_CB](&cbinfo);
		return ret;
	}
	case WM_MOUSEMOVE:
	{
		pThis->inf.x = cbinfo.x;
		pThis->inf.y = cbinfo.y;
		if (cbinfo.y > 0)
		{
			cbinfo.status = CRUI_STAT_MOVE;
			pThis->funcs[CRUI_MOUSE_CB](&cbinfo);
		}
		return 0;
	}
	case WM_SETCURSOR:
	{
		if (pThis->inf.y < 0 && pThis->inf.x < CRUI_TITLEBAR_PIXEL)
			SetCursor(LoadCursor(NULL, IDC_HAND));
		else
			SetCursor(LoadCursor(NULL, IDC_ARROW));
		return 0;
	}
	case WM_LBUTTONDOWN:
	{
		if (cbinfo.h < 0)
		{
			if (cbinfo.x > CRUI_TITLEBAR_PIXEL * 3)
			{
				pThis->follow(CRTRUE);
				pThis->inf.dx = cbinfo.x;
				pThis->inf.dy = cbinfo.y + CRUI_TITLEBAR_PIXEL;
			}
			else if (cbinfo.x > CRUI_TITLEBAR_PIXEL * 2)
			{}
			else if (cbinfo.x > CRUI_TITLEBAR_PIXEL)
			{}
			else
				pThis->inf.preClose = CRTRUE;
		}
		else
		{
			cbinfo.status = CRUI_STAT_DOWN | CRUI_STAT_LEFT;
			pThis->funcs[CRUI_MOUSE_CB](&cbinfo);
			//
			cbinfo.status = CRUI_STAT_DOWN | CRUI_STAT_LEFT;
			point.x = cbinfo.x;
			point.y = cbinfo.y;
			cbinfo.lp = CRDynamicPtr();
			pThis->inf.pgl->SeekEntity(point, cbinfo.lp);
			if (CRStructureSize(cbinfo.lp))
				pThis->funcs[CRUI_ENTITY_CB](&cbinfo);
			CRFreeStructure(cbinfo.lp, NULL);
		}
		return 0;
	}
	case WM_LBUTTONUP:
	{
		if (cbinfo.h < 0)
		{
			if (cbinfo.x > CRUI_TITLEBAR_PIXEL * 3)
			{
			}
			else if (cbinfo.x > CRUI_TITLEBAR_PIXEL * 2)
			{
			}
			else if (cbinfo.x > CRUI_TITLEBAR_PIXEL)
			{
			}
			else if (pThis->inf.preClose)
				pThis->clear();
		}
		else
		{
			cbinfo.status = CRUI_STAT_UP | CRUI_STAT_LEFT;
			pThis->funcs[CRUI_MOUSE_CB](&cbinfo);
			//
			//
			cbinfo.status = CRUI_STAT_UP | CRUI_STAT_LEFT;
			point.x = cbinfo.x;
			point.y = cbinfo.y;
			cbinfo.lp = CRDynamicPtr();
			pThis->inf.pgl->SeekEntity(point, cbinfo.lp);
			if (CRStructureSize(cbinfo.lp))
				pThis->funcs[CRUI_ENTITY_CB](&cbinfo);
			CRFreeStructure(cbinfo.lp, NULL);
		}
		pThis->follow(CRFALSE);
		pThis->inf.preClose = CRFALSE;
		return 0;
	}
	case WM_RBUTTONDOWN:
	{
		cbinfo.status = CRUI_STAT_DOWN | CRUI_STAT_RIGHT;
		if (cbinfo.y > 0)
		{
			pThis->funcs[CRUI_MOUSE_CB](&cbinfo);
			//
			cbinfo.status = CRUI_STAT_DOWN | CRUI_STAT_RIGHT;
			point.x = cbinfo.x;
			point.y = cbinfo.y;
			cbinfo.lp = CRDynamicPtr();
			pThis->inf.pgl->SeekEntity(point, cbinfo.lp);
			if (CRStructureSize(cbinfo.lp))
				pThis->funcs[CRUI_ENTITY_CB](&cbinfo);
			CRFreeStructure(cbinfo.lp, NULL);
		}
		return 0;
	}
	case WM_RBUTTONUP:
	{
		cbinfo.status = CRUI_STAT_UP | CRUI_STAT_RIGHT;
		if (cbinfo.y > 0)
		{
			pThis->funcs[CRUI_MOUSE_CB](&cbinfo);
			//
			cbinfo.status = CRUI_STAT_UP | CRUI_STAT_RIGHT;
			point.x = cbinfo.x;
			point.y = cbinfo.y;
			cbinfo.lp = CRDynamicPtr();
			pThis->inf.pgl->SeekEntity(point, cbinfo.lp);
			if (CRStructureSize(cbinfo.lp))
				pThis->funcs[CRUI_ENTITY_CB](&cbinfo);
			CRFreeStructure(cbinfo.lp, NULL);
		}
		return 0;
	}
	case WM_KEYDOWN:
	{
		cbinfo.status = CRUI_STAT_DOWN;
		if (!pThis->funcs[CRUI_KEY_CB](&cbinfo))
			return DefWindowProc(hWnd, msg, wParam, lParam);
		return 0;
	}
	case WM_KEYUP:
	{
		cbinfo.status = CRUI_STAT_UP;
		if (!pThis->funcs[CRUI_KEY_CB](&cbinfo))
			return DefWindowProc(hWnd, msg, wParam, lParam);
		return 0;
	}
	case WM_CHAR:
	{
		if (!pThis->funcs[CRUI_KEY_CB](&cbinfo))
			return DefWindowProc(hWnd, msg, wParam, lParam);
		return 0;
	}
	case WM_SETFOCUS:
	{
		cbinfo.status = CRUI_STAT_DOWN;
		if (!pThis->funcs[CRUI_FOCUS_CB](&cbinfo))
			return DefWindowProc(hWnd, msg, wParam, lParam);
		return 0;
	}
	case WM_KILLFOCUS:
	{
		cbinfo.status = CRUI_STAT_UP;
		if (!pThis->funcs[CRUI_FOCUS_CB](&cbinfo))
			return DefWindowProc(hWnd, msg, wParam, lParam);
		return 0;
	}
	case WM_MOVE:
	{
		if (!pThis->funcs[CRUI_MOVE_CB](&cbinfo))
			return DefWindowProc(hWnd, msg, wParam, lParam);
		return 0;
	}
	case WM_SIZE:
	{
		pThis->inf.w = cbinfo.w;
		pThis->inf.h = cbinfo.h;
		pThis->inf.pgl->Resize(cbinfo.x, cbinfo.y);
		pThis->inf.pgl->Ratio();
		if (!pThis->funcs[CRUI_SIZE_CB](&cbinfo))
			return DefWindowProc(hWnd, msg, wParam, lParam);
		return 0;
	}
	case WM_GETMINMAXINFO:
	{
		if (pThis->sizeLimit)
		{
			MINMAXINFO *mminfo = (MINMAXINFO *)lParam;
			mminfo->ptMaxTrackSize.x = pThis->_maxx;
			mminfo->ptMaxTrackSize.y = pThis->_maxy;
			mminfo->ptMinTrackSize.x = pThis->_minx;
			mminfo->ptMinTrackSize.y = pThis->_miny;
		}
		return 0;
	}
	default:
		break;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

LRESULT WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_CREATE)
	{
		LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(lpcs->lpCreateParams));
		CRWindowWindows* pThis = reinterpret_cast<CRWindowWindows *>(lpcs->lpCreateParams);
		pThis->inf.pgl = new ccl_gl(GetDC(hWnd));
		return 0;
	}
	else
	{
		CRWindow *pThis = reinterpret_cast<CRWindow *>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
		if (pThis)
		{
			return AfterProc(hWnd, msg, wParam, lParam, (CRWindowWindows *)pThis);
		}
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

void regist_class()
{
	if (!wndclass)
	{
		WNDCLASSEX wcex = { 0 };
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = CS_VREDRAW | CS_HREDRAW;
		wcex.lpfnWndProc = WndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = GetModuleHandle(NULL);
		wcex.hIcon = NULL;
		wcex.hCursor = NULL;
		wcex.hbrBackground = NULL;
		wcex.lpszMenuName = NULL;
		wcex.lpszClassName = TEXT("crwndclass");
		wcex.hIconSm = NULL;
		RegisterClassEx(&wcex);
		wndclass = CRTRUE;
	}
}

CRAPI CRCODE CRUIInit()
{
	CRCODE code = CRBasicInit();
	if (code)
		return code;
	if (!CROpenGLInit())
	{
		CRThrowError(CRERR_CROPENGL_LOAD, CRDEF_CROPENGL_LOAD);
		return CRERR_CROPENGL_LOAD;
	}
	if (!inited)
	{
		windowPool = CRTree();
		availableID = CRLinear();
		CurrentID = 1;
		regist_class();
		inited = CRTRUE;
	}
	return 0;
}

void _clear_callback_(void *data)
{
	((CRWindow*)data)->quit();
}

CRAPI void CRUIUnInit()
{
	CRFreeStructure(windowPool, _clear_callback_);
	CRFreeStructure(availableID, NULL);
	windowPool = NULL;
	availableID = NULL;
	inited = CRFALSE;
	CROpenGLUninit();
	CRBasicUninit();
}

CRAPI CRWINDOW CRCreateWindow(const char* title, CRUINT32 x, CRUINT32 y, CRUINT32 w, CRUINT32 h)
{
	CRWINDOW id;
	CRLinGet(availableID, (CRLVOID*)&id, 0);
	if (!id)
		id = CurrentID++;
	CRWindow* pWindow = new CRWindowWindows(title, w, h, x, y);
	if (!pWindow)
	{
		CRThrowError(CRERR_OUTOFMEM, NULL);
		return 0;
	}
	pWindow->SetID(id);
	CRTreePut(windowPool, pWindow, id);
	return id;
}

#elif defined CR_LINUX
#include <X11/Xlib.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <X11/cursorfont.h>

Display *pDisplay;
Window rootWindow;
//等什么时候有时间了分到新文件里面去
#define CR_X_CURSOR_DEFAULT 0
#define CR_X_CURSOR_HAND    1
Cursor cursors[4];

typedef struct cr_windowthread_inf
{
	void *pThis;
	CRUINT32 fps;
	const char *title;
	CRUINT32 w, h;
	CRINT32 x, y;
	CRUINT32 dx, dy;
	CRWindowCallback *cbk;
	CRBOOL onQuit;
	XVisualInfo *vi;
	Window win;
	ccl_gl *pgl;
	CRBOOL paint;
	Atom protocols_quit;
	CRTHREAD windowThread = 0;
	CRTHREAD paintThread = 0;
	//
	CRBOOL preClose;
} CRWINDOWINF;

class CRWindowLinux : public CRWindow
{
public:
	CRWindowLinux(const char *title,
					CRUINT32 w, CRUINT32 h,
					CRINT32 x, CRINT32 y);
	~CRWindowLinux();
	// 禁止拷贝
	CRWindowLinux &operator=(const CRWindowLinux &) = delete;
	CRWindowLinux(const CRWindowLinux &) = delete;

	virtual void quit();
	virtual void clear();
	virtual void Text(const char *text);
	virtual void Move(CRINT32 x, CRINT32 y, CRUINT32 w, CRUINT32 h);
	virtual CRCODE Entity(CRUIENTITY* pEntity);

private:
	CRWINDOWINF inf;

	friend void windowserv_ccl(void *req);
};

CRWindowLinux::CRWindowLinux(const char *title,
							CRUINT32 w, CRUINT32 h,
							CRINT32 x, CRINT32 y)
{
	inf.onQuit = CRFALSE;
	inf.fps = 30;
	inf.cbk = funcs;
	inf.title = title;

	int screenNum = DefaultScreen(pDisplay);
	CRUINT32 s_w = DisplayWidth(pDisplay, screenNum);
	CRUINT32 s_h = DisplayHeight(pDisplay, screenNum);

	if (w == CRWINDOW_USEDEFAULT)
		w = (s_w > s_h ? s_w : s_h) / 2;
	if (h == CRWINDOW_USEDEFAULT)
		h = (s_w > s_h ? s_w : s_h) * 3 / 4;
	if (x == CRWINDOW_USEDEFAULT)
		x = (s_w - w) / 2;
	if (y == CRWINDOW_USEDEFAULT)
		y = (s_h - h) / 2;

	inf.w = w;
	inf.h = h + CRUI_TITLEBAR_PIXEL;
	inf.x = x;
	inf.y = y;
	inf.win = 0;
	inf.vi = nullptr;
	inf.pThis = this;
	inf.pgl = nullptr;
	inf.paint = CRFALSE;
	inf.windowThread = CRThread(_window_thread_, &inf);
	inf.paintThread = CRThread(_paint_thread_, &inf);
}

CRWindowLinux::~CRWindowLinux()
{}

void CRWindowLinux::quit()
{
	inf.onQuit = CRTRUE;
}

void CRWindowLinux::clear()
{
	//发消息利用原本的方法来自然销毁是最完美的解法
	XEvent event;
	event.type = ClientMessage;
	event.xany.window = inf.win;
	event.xclient.data.l[0] = inf.protocols_quit;
	XPutBackEvent(pDisplay, &event);
}

void CRWindowLinux::Text(const char *text)
{
}

void CRWindowLinux::Move(CRINT32 x, CRINT32 y, CRUINT32 w, CRUINT32 h)
{
}

CRCODE CRWindowLinux::Entity(CRUIENTITY* pEntity)
{
	while (!inf.pgl) CRSleep(1);
	return inf.pgl->AddEntity(pEntity);
}

void ProcessMsg(CRWINDOWINF *inf)
{
	CRUIMSG cbinfo;
	XEvent event;
	CRWindowLinux *pWindow = (CRWindowLinux *)(inf->pThis);
	cbinfo.status = CRUI_STAT_OTHER;
	cbinfo.window = pWindow->GetID();
	CRPOINTU point;
	while (!inf->onQuit)
	{
		XNextEvent(pDisplay, &event);
		if (event.xany.window != inf->win)
		{
			XPutBackEvent(pDisplay, &event);
			CRSleep(1);
			continue;
		}
		switch (event.type)
		{
		case Expose: // “显示”事件
		{
			if (event.xexpose.count == 0)
			{
				inf->paint = CRTRUE;
			}
			break;
		}
		case ConfigureNotify: // 窗口改变大小
		{
			inf->w = event.xconfigure.width;
			inf->h = event.xconfigure.height - CRUI_TITLEBAR_PIXEL;
			cbinfo.x = event.xconfigure.width;
			cbinfo.y = event.xconfigure.height;
			inf->paint = CRTRUE;
			break;
		}
		case MotionNotify:
		{
			if (event.xbutton.x < CRUI_TITLEBAR_PIXEL && event.xbutton.y < CRUI_TITLEBAR_PIXEL)
				XDefineCursor(pDisplay, inf->win, cursors[CR_X_CURSOR_HAND]);
			else
			{
				XDefineCursor(pDisplay, inf->win, cursors[CR_X_CURSOR_DEFAULT]);
				//
				if (pWindow->follow_stat())
					XMoveWindow(pDisplay, inf->win, event.xmotion.x_root - inf->dx, event.xmotion.y_root - inf->dy);
				else
				{
					cbinfo.status = CRUI_STAT_MOVE;
					cbinfo.x = event.xbutton.x;
					cbinfo.y = event.xbutton.y - CRUI_TITLEBAR_PIXEL;
					inf->cbk[CRUI_MOUSE_CB](&cbinfo);
				}
			}
			break;
		}
		case ButtonPress:
		{
			
			cbinfo.x = event.xbutton.x;
			cbinfo.y = event.xbutton.y - CRUI_TITLEBAR_PIXEL;
			if (cbinfo.h > 0)
			{
				cbinfo.status = CRUI_STAT_DOWN;
				if (event.xbutton.button == 1)
					cbinfo.status |= CRUI_STAT_LEFT;
				else if (event.xbutton.button == 3)
					cbinfo.status |= CRUI_STAT_RIGHT;
				inf->cbk[CRUI_MOUSE_CB](&cbinfo);
				//
				cbinfo.status = CRUI_STAT_DOWN | CRUI_STAT_RIGHT;
				point.x = cbinfo.x;
				point.y = cbinfo.y;
				cbinfo.lp = CRDynamicPtr();
				inf->pgl->SeekEntity(point, cbinfo.lp);
				if (CRStructureSize(cbinfo.lp))
					inf->cbk[CRUI_ENTITY_CB](&cbinfo);
				CRFreeStructure(cbinfo.lp, NULL);
			}
			else if (event.xbutton.button == 1)
			{
				if (cbinfo.x > CRUI_TITLEBAR_PIXEL * 3)
				{
					inf->dx = event.xbutton.x;
					inf->dy = event.xbutton.y;
					pWindow->follow(CRTRUE);
				}
				else if (cbinfo.x > CRUI_TITLEBAR_PIXEL * 2)
				{
				}
				else if (cbinfo.x > CRUI_TITLEBAR_PIXEL)
				{
				}
				else
					inf->preClose = CRTRUE;
			}
			break;
		}
		case ButtonRelease:
		{
			cbinfo.x = event.xbutton.x;
			cbinfo.y = event.xbutton.y - CRUI_TITLEBAR_PIXEL;
			if (cbinfo.y > 0)
			{
				cbinfo.status = CRUI_STAT_UP;
				inf->preClose = CRFALSE;
				if (event.xbutton.button == 1)
					cbinfo.status |= CRUI_STAT_LEFT;
				else if (event.xbutton.button == 3)
					cbinfo.status |= CRUI_STAT_RIGHT;
				inf->cbk[CRUI_MOUSE_CB](&cbinfo);
				//
				cbinfo.status = CRUI_STAT_UP | CRUI_STAT_RIGHT;
				point.x = cbinfo.x;
				point.y = cbinfo.y;
				cbinfo.lp = CRDynamicPtr();
				inf->pgl->SeekEntity(point, cbinfo.lp);
				if (CRStructureSize(cbinfo.lp))
					inf->cbk[CRUI_ENTITY_CB](&cbinfo);
				CRFreeStructure(cbinfo.lp, NULL);
			}
			else if (event.xbutton.button == 1)
			{
				if (cbinfo.x > CRUI_TITLEBAR_PIXEL * 3)
				{
				}
				else if (cbinfo.x > CRUI_TITLEBAR_PIXEL * 2)
				{
				}
				else if (cbinfo.x > CRUI_TITLEBAR_PIXEL)
				{
				}
				else if (inf->preClose)
					pWindow->clear();
				pWindow->follow(CRFALSE);
			}
			break;
		}
		case KeyPress:
		{
			cbinfo.status = CRUI_STAT_DOWN;
			cbinfo.keycode = event.xkey.keycode;
			inf->cbk[CRUI_KEY_CB](&cbinfo);
			break;
		}
		case KeyRelease:
		{
			cbinfo.status = CRUI_STAT_UP;
			cbinfo.keycode = event.xkey.keycode;
			inf->cbk[CRUI_KEY_CB](&cbinfo);
			break;
		}
		case DestroyNotify:
			break;
		case ClientMessage:
		{
			if (event.xclient.data.l[0] == inf->protocols_quit)
			{
				if (!inf->cbk[CRUI_QUIT_CB](&cbinfo))
				{
					pWindow->quit();
					// 释放
					XSelectInput(pDisplay, inf->win, NoEventMask);
					CRWaitThread(inf->paintThread);
					XDestroyWindow(pDisplay, inf->win);
				}
			}
			break;
		}
		default:
		{
			if (inf->onQuit)
			{
				// 释放
				XSelectInput(pDisplay, inf->win, NoEventMask);
				XDestroyWindow(pDisplay, inf->win);
				break;
			}
			CRSleep(1);
			break;
		}
		}
	}
	XFlush(pDisplay);
}

/*
* 无边框窗口需要的东西
*
* 这些是用来创建无边框窗口使用的
* 内部的一些数据结构
*/

#define MWM_HINTS_FUNCTIONS (1L << 0)
#define MWM_HINTS_DECORATIONS  (1L << 1)

#define MWM_FUNC_ALL (1L << 0)
#define MWM_FUNC_RESIZE (1L << 1)
#define MWM_FUNC_MOVE (1L << 2)
#define MWM_FUNC_MINIMIZE (1L << 3)
#define MWM_FUNC_MAXIMIZE (1L << 4)
#define MWM_FUNC_CLOSE (1L << 5)

#define PROP_MWM_HINTS_ELEMENTS 5
typedef struct _mwmhints
{
    uint32_t flags;
    uint32_t functions;
    uint32_t decorations;
    int32_t  input_mode;
    uint32_t status;
}MWMHints;
//
//

void _window_thread_(CRLVOID data, CRTHREAD idThis)
{
	GLint att[] = {
		GLX_RGBA,
		GLX_DOUBLEBUFFER,
		GLX_DEPTH_SIZE, 24,
		GLX_SAMPLE_BUFFERS,
		None
		};
	Colormap cmap;
	XSetWindowAttributes swa;
	GLXContext glc;
	XWindowAttributes gwa;
	XEvent xev;

	CRWINDOWINF* inf = (CRWINDOWINF*)data;

	inf->vi = glXChooseVisual(pDisplay, 0, att);
	cmap = XCreateColormap(pDisplay, rootWindow, inf->vi->visual, AllocNone);

	swa.colormap = cmap;
	//选择关心的事件
	swa.event_mask = ExposureMask
	| KeyPressMask | ButtonPressMask
	| KeyReleaseMask | ButtonReleaseMask
	| PointerMotionMask
	| StructureNotifyMask;
	inf->win = XCreateWindow(pDisplay, rootWindow, inf->x, inf->y, inf->w, inf->h, 0, inf->vi->depth, InputOutput, inf->vi->visual, CWColormap | CWEventMask, &swa);
	XStoreName(pDisplay, inf->win, inf->title);
	//
	//无边框窗口的制作：
	MWMHints mwmhints;
	Atom prop;
	memset(&mwmhints,0, sizeof(MWMHints));
	prop = XInternAtom(pDisplay, "_MOTIF_WM_HINTS", False);
	mwmhints.flags = MWM_HINTS_DECORATIONS;
	mwmhints.decorations = 0;
	XChangeProperty(
	    pDisplay, inf->win, prop, prop, 32,
	    PropModeReplace, (unsigned char*)&mwmhints,
	    PROP_MWM_HINTS_ELEMENTS
	);
	XMapWindow(pDisplay, inf->win);

	XMoveResizeWindow(pDisplay, inf->win, inf->x, inf->y, inf->w, inf->h);

	//捕获退出事件
	inf->protocols_quit = XInternAtom(pDisplay, "WM_DELETE_WINDOW", True);
	XSetWMProtocols(pDisplay, inf->win, &(inf->protocols_quit), 1);

	/**/
	ProcessMsg(inf);
	/**/
}

void _paint_thread_(CRLVOID data, CRTHREAD idThis)
{
	const char *CCLstr = "CCL XWindow";
	char size[32];
	CRWINDOWINF *inf = (CRWINDOWINF *)data;
	while (!inf->win)
		CRSleep(1);

	//一定要确保创建和绘制都在同一个线程
	inf->pgl = new ccl_gl(pDisplay, inf->vi, inf->win);
	inf->pgl->Resize(inf->w, inf->h);
	inf->pgl->Ratio();

	CRTIMER timer = CRTimer();
	CRTimerMark(timer);
	while (!inf->onQuit)
	{
		if (CRTimerPeek(timer) >= (float)1 / (float)(inf->fps) || inf->paint)
		{
			CRTimerMark(timer);
			inf->paint = CRFALSE;
			inf->pgl->Resize(inf->w, inf->h);
			inf->pgl->PaintAll();
			XFlush(pDisplay);
		}
		CRSleep(1);
	}
	CRTimerClose(timer);
	CRWindow* pWindow = (CRWindow*)(inf->pThis);
	delete inf->pgl;
	CRLinPut(availableID, (CRLVOID)(pWindow->GetID()), 0);
	CRTreeGet(windowPool, NULL, pWindow->GetID());
	delete pWindow;
}

CRAPI CRCODE CRUIInit()
{
	CRCODE code = CRBasicInit();
	if (code)
		return code;
	if (!CROpenGLInit())
	{
		CRThrowError(CRERR_CROPENGL_LOAD, CRDEF_CROPENGL_LOAD);
		return CRERR_CROPENGL_LOAD;
	}
	if (!inited)
	{
		windowPool = CRTree();
		availableID = CRLinear();
		pDisplay = XOpenDisplay(NULL);
		rootWindow = DefaultRootWindow(pDisplay);
		CurrentID = 1;
		//光标形状
		cursors[CR_X_CURSOR_DEFAULT] = XCreateFontCursor(pDisplay, XC_left_ptr);
		cursors[CR_X_CURSOR_HAND] = XCreateFontCursor(pDisplay, XC_hand2);
		inited = CRTRUE;
	}
	return 0;
}

void _clear_callback_(void *data)
{
	((CRWindow*)data)->clear();
}

CRAPI void CRUIUnInit()
{
	CRFreeStructure(windowPool, _clear_callback_);
	CRFreeStructure(availableID, NULL);
	windowPool = NULL;
	availableID = NULL;
	XCloseDisplay(pDisplay);
	rootWindow = 0;
	inited = CRFALSE;
	CROpenGLUninit();
	CRBasicUninit();
}

CRAPI CRWINDOW CRCreateWindow(const char* title, CRUINT32 x, CRUINT32 y, CRUINT32 w, CRUINT32 h)
{
	CRWINDOW id;
	CRLinGet(availableID, (CRLVOID*)&id, 0);
	if (!id)
		id = CurrentID++;
	CRWindow* pWindow = new CRWindowLinux(title, w, h, x, y);
	if (!pWindow)
	{
		CRThrowError(CRERR_OUTOFMEM, NULL);
		return 0;
	}
	pWindow->SetID(id);
	CRTreePut(windowPool, pWindow, id);
	return id;
}

#endif

CRAPI CRCODE CRUIOnQuit()
{
	return CRStructureSize(windowPool);
}

CRAPI CRCODE CRCloseWindow(CRWINDOW window)
{
	CRWindow* pWindow = NULL;
	CRTreeSeek(windowPool, (CRLVOID*)&pWindow, window);
	if (!pWindow)
		return CRERR_INVALID;
	pWindow->clear();
	return 0;
}

CRAPI CRCODE CRSetWindowCbk(CRWINDOW window, CRWindowCallback func, CRUINT8 cbkID)
{
	CRWindow* pWindow = NULL;
	if (cbkID >= CALLBACK_FUNCS_NUM || !func)
		return CRERR_INVALID;
	CRTreeSeek(windowPool, (CRLVOID*)&pWindow, window);
	if (!pWindow)
		return CRERR_INVALID;
	pWindow->setcbk(func, cbkID);
	return 0;
}

CRAPI CRCODE CRWindowEntityAdd(CRWINDOW window, CRUIENTITY* pEntity)
{
	CRWindow* pWindow = NULL;
	CRTreeSeek(windowPool, (CRLVOID*)&pWindow, window);
	if (!pWindow || !pEntity)
		return CRERR_INVALID;
	return pWindow->Entity(pEntity);
}