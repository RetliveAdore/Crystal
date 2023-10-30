#include <Crystal.h>
#include <parts/CrUI.h>
#include <parts/Crbasic.h>
#include <crerrors.h>

#ifdef CR_LINUX
#include <X11/Xlib.h>
#include <malloc.h>
#include <GL/glx.h>
#include <GL/glu.h>
#include <string.h>

static Display* pDisplay = NULL;
static CRSTRUCTURE windowPool = NULL;
static CRSTRUCTURE availableID = NULL;
static CRUINT64 currentID = 1;
static CRBOOL crUiInited = CRFALSE;

CRCODE DoNothing(PCRUIMSG msg) { return 0; }

typedef struct
{
    CRWINDOW window;
    Window win;
    Atom protocolQuit;
    XVisualInfo* vi;
    //
    CRBOOL sizeLimit;
    CRUINT32 maxx, maxy;
    CRUINT32 minx, miny;
    //
    CRTHREAD windowThread;
    CRTHREAD paintThread;
    CRWindowCallback funcs[CALLBACK_FUNCS_NUM];
    //控制退出
    CRBOOL onQuit;
    //
    const char* title;
    CRUINT32 x, y, w, h;
}CRWINDOWINNER, *PCRWINDOWINNER;

//

/*
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

CRAPI CRCODE CRUIInit()
{
    if (crUiInited)
        return 0;
    if (!(pDisplay = XOpenDisplay(NULL)))
    {
        CRThrowError(CRERR_WINDOW_OPENDISPLAY, CRDES_WINDOW_OPENDISPLAY);
        return CRERR_WINDOW_OPENDISPLAY;
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
        XCloseDisplay(pDisplay);
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
    XCloseDisplay(pDisplay);
    pDisplay = NULL;
    windowPool = NULL;
    availableID = NULL;
}

CRAPI CRBOOL CRUIOnQuit()
{
	if (CRStructureSize(windowPool))
		return CRFALSE;
	return CRTRUE;
}

//

/*
* 负责创建窗体和维护的部分
*/

void ProcessMsg(PCRWINDOWINNER window)
{
    CRUIMSG crMsg;
    XEvent event;
    crMsg.window = window->window;
    while (!window->onQuit)
    {
        XNextEvent(pDisplay, &event);
        if (event.xany.window != window->win)
		{
			XPutBackEvent(pDisplay, &event);
			CRSleep(1);
			continue;
		}
        switch (event.type)
        {
        case ClientMessage:
        {
            if (event.xclient.data.l[0] == window->protocolQuit)
            {
                if (!window->funcs[CRUI_QUIT_CB](&crMsg))
                {
                    XSelectInput(pDisplay, window->win, NoEventMask);
                    XDestroyWindow(pDisplay, window->win);
                    window->onQuit = CRTRUE;
                }
            }
            break;
        }
        default:
        {
            CRSleep(1);
            break;
        }
        }
    }
    XFlush(pDisplay);
}

void _window_thread_(CRLVOID userdata, CRTHREAD idThis)
{
    PCRWINDOWINNER pInner = userdata;
    //这一段是为开启opengl做准备
    Window root = DefaultRootWindow(pDisplay);
    GLint att[] =
    {
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
    pInner->vi = glXChooseVisual(pDisplay, 0, att);
    if (!pInner->vi)
        goto End;
    cmap = XCreateColormap(pDisplay, root, pInner->vi->visual, AllocNone);
    //
    //
    swa.colormap = cmap;
    swa.event_mask = ExposureMask
	| KeyPressMask | ButtonPressMask
	| KeyReleaseMask | ButtonReleaseMask
	| PointerMotionMask
	| StructureNotifyMask;
    pInner->win = XCreateWindow(pDisplay,
        root,
        pInner->x, pInner->y, pInner->w, pInner->h,
        0,
        pInner->vi->depth,
        InputOutput,
        pInner->vi->visual,
        CWColormap | CWEventMask,
        &swa
    );
    XStoreName(pDisplay, pInner->win, pInner->title);
    //捕获退出事件
    pInner->protocolQuit = XInternAtom(pDisplay, "WM_DELETE_WINDOW", True);
    XSetWMProtocols(pDisplay, pInner->win, &(pInner->protocolQuit), 1);
    //无边框窗口的制作：
    MWMHints mwmhints;
    Atom prop;
    memset(&mwmhints,0, sizeof(MWMHints));
    prop = XInternAtom(pDisplay, "_MOTIF_WM_HINTS", False);
    mwmhints.flags = MWM_HINTS_DECORATIONS;
    mwmhints.decorations = 0;
    XChangeProperty(
        pDisplay, pInner->win, prop, prop, 32,
        PropModeReplace, (unsigned char*)&mwmhints,
        PROP_MWM_HINTS_ELEMENTS
    );
    //
    //
    XMapWindow(pDisplay, pInner->win);
    ProcessMsg(pInner);
End:
    CRTreeGet(windowPool, NULL, pInner->window);
    free(pInner);
}

void _paint_thread_(CRLVOID userdata, CRTHREAD idThis)
{
    
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
    int screenNum = DefaultScreen(pDisplay);
    int sx = DisplayWidth(pDisplay, screenNum);
    int sy = DisplayHeight(pDisplay, screenNum);
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
    CRLinGet(availableID, (CRLVOID*)&(pInner->window), 0);
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
	CRTreeSeek(windowPool, (CRLVOID*)&pInner, window);
	if (pInner)
	{
		CRTHREAD windowThread = pInner->windowThread;
		XEvent event;
        event.type = ClientMessage;
        event.xany.window = pInner->win;
        event.xclient.data.l[0] = pInner->protocolQuit;
        XPutBackEvent(pDisplay, &event);
		CRWaitThread(windowThread);
	}
	return 0;
}

#endif