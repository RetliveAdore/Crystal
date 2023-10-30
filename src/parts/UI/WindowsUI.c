#include <Crystal.h>
#include <parts/CrUI.h>
#include <parts/Crbasic.h>
#include <crerrors.h>

#ifdef CR_WINDOWS
#include <Windows.h>

const char classname[] = TEXT("CrystalWindowClass");

static CRSTRUCTURE windowPool = NULL;
static CRSTRUCTURE availableID = NULL;
static CRUINT64 currentID = 1;
static CRBOOL crUiInited = CRFALSE;
static CRBOOL crWindowClass = CRFALSE;

CRCODE DoNothing(PCRUIMSG msg) { return 0; }

typedef struct
{
	CRWINDOW window;
	HWND hWnd;
	//
	CRBOOL sizeLimit;
	CRUINT32 maxx, maxy;
	CRUINT32 minx, miny;
	//
	CRTHREAD windowThread;  //负责处理窗口事件的线程
	CRTHREAD paintThread;   //负责绘制渲染的线程
	CRWindowCallback funcs[CALLBACK_FUNCS_NUM];
	//控制窗口线程的退出
	CRBOOL onQuit;
	//创建窗口需要的信息
	char* title;
	CRUINT32 x, y, w, h;
}CRWINDOWINNER, *PCRWINDOWINNER;

LRESULT ProcessProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, PCRWINDOWINNER window)
{
	CRUIMSG crMsg;
	crMsg.window = window->window;
	crMsg.status = CRUI_STAT_OTHER;
	if (msg == WM_CLOSE)
	{
		if (!window->funcs[CRUI_QUIT_CB](&crMsg))
		{
			DestroyWindow(hWnd);
			return 0;
		}
	}
	else if (msg == WM_DESTROY)
	{
		window->onQuit = CRTRUE;
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

LRESULT WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_CREATE)
	{
		LPCREATESTRUCT lpcs = lParam;
		SetWindowLongPtr(hWnd, GWLP_USERDATA, lpcs->lpCreateParams);
	}
	else
	{
		PCRWINDOWINNER pInner = GetWindowLongPtr(hWnd, GWLP_USERDATA);
		if (pInner)
			return ProcessProc(hWnd, msg, wParam, lParam, pInner);
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

CRAPI CRCODE CRUIInit()
{
	if (crUiInited)
		return 0;
	if (!crWindowClass)
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
		wcex.lpszClassName = classname;
		wcex.hIconSm = NULL;
		if (!RegisterClassEx(&wcex))
		{
			CRThrowError(CRERR_WINDOW_REGISTERCLASS, CRDES_WINDOW_REGISTERCLASS);
			return CRERR_WINDOW_REGISTERCLASS;
		}
		crWindowClass = CRTRUE;
	}
	CRCODE code = CRBasicInit();
	if (code)
		return code;
	windowPool = CRTree();
	if (!windowPool)
	{
		CRThrowError(CRERR_WINDOW_CRCREATEPOOL, CRDES_WINDOW_CRCREATEPOOL);
		return CRERR_WINDOW_CRCREATEPOOL;
	}
	availableID = CRLinear();
	if (!availableID)
	{
		CRFreeStructure(windowPool, NULL);
		CRThrowError(CRERR_WINDOW_CRCREATEIDPOOL, CRDES_WINDOW_CRCREATEIDPOOL);
		return CRERR_WINDOW_CRCREATEIDPOOL;
	}
	crUiInited = CRTRUE;
	return 0;
}

CRAPI void CRUIUnInit()
{
	crUiInited = CRFALSE;
	CRFreeStructure(windowPool, NULL);
	CRFreeStructure(availableID, NULL);
	windowPool = NULL;
	availableID = NULL;
	CRBasicUninit();
}

CRAPI CRBOOL CRUIOnQuit()
{
	if (CRStructureSize(windowPool))
		return CRFALSE;
	return CRTRUE;
}

//

/*
* 负责窗体创建和维护的部分
*/

//渲染线程归窗口线程管，二者一一对应
void _window_thread_(CRLVOID userdata, CRTHREAD idThis)
{
	PCRWINDOWINNER pInner = userdata;
	//窗体必须由本线程创建本线程维护，否则无法获取消息
	pInner->hWnd = CreateWindowEx(
		WS_EX_APPWINDOW,
		classname,
		pInner->title,
		WS_POPUP | WS_VISIBLE | WS_BORDER,
		pInner->x, pInner->y,
		pInner->w, pInner->h,
		NULL,
		NULL,
		GetModuleHandle(NULL),
		pInner
	);
	UpdateWindow(pInner->hWnd);
	ShowWindow(pInner->hWnd, SW_SHOWDEFAULT);
	//开启消息循环
	MSG msg = { 0 };
	while (!pInner->onQuit)
	{
		if (GetMessage(&msg, NULL, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	CRTreeGet(windowPool, NULL, pInner->window);
	free(pInner);
}

CRAPI CRWINDOW CRCreateWindow(const char* title, CRUINT32 x, CRUINT32 y, CRUINT32 w, CRUINT32 h)
{
	if (!crUiInited)
	{
		CRThrowError(CRERR_UNINITED, NULL);
		return 0;
	}
	PCRWINDOWINNER pInner = calloc(1, sizeof(CRWINDOWINNER));
	if (!pInner)
	{
		CRThrowError(CRERR_OUTOFMEM, NULL);
		return 0;
	}
	int sx = GetSystemMetrics(SM_CXSCREEN);
	int sy = GetSystemMetrics(SM_CYSCREEN);
	int wd, hd;
	if (sx > sy)
	{
		wd = sy * 2 / 3;
		hd = sy / 2;
	}
	else
	{
		wd = sx * 2 / 3;
		hd = sx / 2;
	}
	if (w == CRWINDOW_USEDEFAULT)
		w = wd;
	if (h == CRWINDOW_USEDEFAULT)
		h = hd;
	if (x == CRWINDOW_USEDEFAULT)
		x = (sx - w) / 2;
	if (y == CRWINDOW_USEDEFAULT)
		y = (sy - h) / 2;
	for (int i = 0; i < CALLBACK_FUNCS_NUM; i++)
		pInner->funcs[i] = DoNothing;
	pInner->sizeLimit = CRFALSE;
	pInner->onQuit = CRFALSE;
	pInner->title = title;
	pInner->x = x;
	pInner->y = y;
	pInner->w = w;
	pInner->h = h;
	CRLinGet(availableID, &(pInner->window), 0);
	if (!pInner->window)
		pInner->window = currentID++;
	CRTreePut(windowPool, pInner, pInner->window);
	pInner->windowThread = CRThread(_window_thread_, pInner);
	if (!pInner->windowThread)
    {
        CRTreeGet(windowPool, NULL, pInner->window);
        free(pInner);
        return 0;
    }
	return pInner->window;
}

CRAPI CRCODE CRCloseWindow(CRWINDOW window)
{
	PCRWINDOWINNER pInner = NULL;
	CRTreeSeek(windowPool, &pInner, window);
	if (pInner)
	{
		CRTHREAD windowThread = pInner->windowThread;
		SendMessage(pInner->hWnd, WM_DESTROY, 0, 0);
		CRWaitThread(windowThread);
	}
	return 0;
}

#endif