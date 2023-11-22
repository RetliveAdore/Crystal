#include <cropgl.hpp>
#include <math.h>

//一些常数
static const CRUINT32 ratio = 4;
static const CRUINT32 n1 = 80;
static const CRUINT32 n2 = 8;
static const CRUINT32 nc = 3;

void DrawRect(float x1, float y1, float x2, float y2, float level, float stroke, CRCOLORF* pColor)
{
    float tmp;
    if (stroke < 0)
        stroke = -stroke;
    if (x1 > x2)
    {
        tmp = x2;
        x2 = x1;
        x1 = tmp;
    }
    if (y1 > y2)
    {
        tmp = y2;
        y2 = y1;
        y1 = tmp;
    }
    level = -level;
    stroke /= 2 * ratio;

    glColor4f(pColor->r, pColor->g, pColor->b, pColor->a);
    glBegin(GL_QUADS);
    glVertex3f(x1 + stroke, y1 + stroke, level);
    glVertex3f(x1 - stroke, y1 - stroke, level);
    glVertex3f(x1 - stroke, y2 + stroke, level);
    glVertex3f(x1 + stroke, y2 - stroke, level);

    glVertex3f(x1 + stroke, y1 + stroke, level);
    glVertex3f(x1 - stroke, y1 - stroke, level);
    glVertex3f(x2 + stroke, y1 - stroke, level);
    glVertex3f(x2 - stroke, y1 + stroke, level);

    glVertex3f(x2 + stroke, y2 + stroke, level);
    glVertex3f(x2 - stroke, y2 - stroke, level);
    glVertex3f(x1 + stroke, y2 - stroke, level);
    glVertex3f(x1 - stroke, y2 + stroke, level);

    glVertex3f(x2 + stroke, y2 + stroke, level);
    glVertex3f(x2 - stroke, y2 - stroke, level);
    glVertex3f(x2 - stroke, y1 + stroke, level);
    glVertex3f(x2 + stroke, y1 - stroke, level);
    glEnd();
}

void DrawElipse(float x, float y, float rx, float ry, float level, float stroke, CRCOLORF* pColor)
{
    if (rx < 0)
        rx = -rx;
    if (ry < 0)
        ry = -ry;
    if (stroke < 0)
        stroke = -stroke;
    level = -level;
    int n = rx * nc + ry * nc;
    if (n > n1)
        n = n1;
    else if (n < n2)
        n = n2;

    glColor4f(pColor->r, pColor->g, pColor->b, pColor->a);
    glBegin(GL_QUADS);
    float lr_x = rx + stroke / ratio, sr_x = rx - stroke / ratio;
    float lr_y = ry + stroke / ratio, sr_y = ry - stroke / ratio;
    float tx_1 = x, tx_2 = x;
    float ty_1 = lr_y + y, ty_2 = sr_y + y;
    for (int i = 0; i <= n; i++)
    {
        glVertex3f(tx_1, ty_1, level);
        glVertex3f(tx_2, ty_2, level);
        tx_1 = sin(i * M_PI * 2 / n) * sr_x + x;
        tx_2 = sin(i * M_PI * 2 / n) * lr_x + x;
        ty_1 = cos(i * M_PI * 2 / n) * sr_y + ry;
        ty_2 = cos(i * M_PI * 2 / n) * lr_y + ry;
        glVertex3f(tx_2, ty_2, level);
        glVertex3f(tx_1, ty_1, level);
    }
    glEnd();
}

void FillRect(float x1, float y1, float x2, float y2, float level, CRCOLORF* pColor)
{
    level = -level;
    glColor4f(pColor->r, pColor->g, pColor->b, pColor->a);
    glBegin(GL_QUADS);
    glVertex3f(x1, y1, level);
    glVertex3f(x1, y2, level);
    glVertex3f(x2, y2, level);
    glVertex3f(x2, y1, level);
    glEnd();
}

void FillElipse(float x, float y, float rx, float ry, float level, CRCOLORF* pColor)
{
    if (rx < 0)
        rx = -rx;
    if (ry < 0)
        ry = -ry;
    level = -level;
    int n = rx * nc + ry * nc;
    if (n > n1)
        n = n1;
    else if (n < n2)
        n = n2;

    glColor4f(pColor->r, pColor->g, pColor->b, pColor->a);
    glBegin(GL_POLYGON);
    for (int i = 0; i < n; i++)
    {
        glVertex3f(sin(i * M_PI * 2 / n) * rx + x, cos(i * M_PI * 2 / n) * ry + y, level);
    }
    glEnd();
}

void DrawPoint(float x, float y, float level, float stroke, CRCOLORF* pColor)
{
    if (stroke < 0)
        stroke = -stroke;
    FillElipse(x, y, stroke / nc, stroke / nc, level, pColor);
}

void DrawLine(float x1, float y1, float x2, float y2, float level, float stroke, CRCOLORF* pColor)
{
    if (stroke < 0)
        stroke = -stroke;
    level = -level;
    stroke /= 2 * ratio;
    float r = -(M_PI_2 - atan((y2 - y1) / (x2 - x1)));
    float dx = cos(r) * stroke, dy = sin(r) * stroke;
    glColor4f(pColor->r, pColor->g, pColor->b, pColor->a);
    glBegin(GL_QUADS);
    glVertex3f(x1 + dx, y1 + dy, level);
    glVertex3f(x1 - dx, y1 - dy, level);
    glVertex3f(x2 - dx, y2 - dy, level);
    glVertex3f(x2 + dx, y2 + dy, level);
    glEnd();
}

void DrawDemo()
{
    CRCOLORF color;
    color.r = 0.3f;
    color.g = 0.5f;
    color.b = 0.2f;
    color.a = 0.5f;
    DrawElipse(20.0f, 30.0f, 30.0f, 20.0f, 1.0f, 3.0f, &color);
    color.r = 0.7f;
    color.g = 0.2f;
    color.b = 0.5f;
    color.a = 0.2;
    FillElipse(0.0f, 0.0f, 50.0f, 50.0f, 3.0f, &color);
    color.r = 0.5f;
    color.g = 0.9f;
    color.b = 1.0f;
    color.a = 0.3;
    DrawPoint(40.0f, 30.0f, 1.0f, 10.0f, &color);
    color.r = 1.0f;
    color.g = 0.0f;
    color.b = 0.0f;
    color.a = 1.0;
    DrawRect(100.0f, -100.0f, -100.0f, 100.0f, 3.0f, 5.0f, &color);
    color.r = 1.0f;
    color.g = 1.0f;
    color.b = 1.0f;
    color.a = 0.4;
    FillRect(40.0f, -20.0f, 10.0f, 30.0f, 0.0f, &color);
    color.r = 0.2f;
    color.g = 0.4f;
    DrawLine(-10.0, -70.0, 50.0, 80.0, -1.0, 8, &color);
}

void InitGL()
{
    glShadeModel(GL_SMOOTH);
    //需要透明度的时候不要启用depth，否则某些情况下画面会被裁掉
    //不过如果按照从后往前的顺序来，就不会有问题
    glDisable(GL_DEPTH_TEST);
    //glDisable(GL_DEPTH);
    //启用透明度通道
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //
    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_POLYGON_SMOOTH);

    //清屏黑
    glClearColor(0.0, 0.0, 0.0, 1.0);
}

#ifdef CR_LINUX

//查询线宽范围（PS:虚拟机里面只支持1-1的线宽）
//static float range[2];

ccl_gl::ccl_gl(Display *pDisplay, XVisualInfo *vi, Window win)
{
    dpy = pDisplay;
    w = win;
    context = glXCreateContext(dpy, vi, nullptr, GL_TRUE);
    glXMakeCurrent(dpy, w, context);
    /*
     * 创建环境完毕
     */
    InitGL();

    //查询
    //glGetFloatv(GL_LINE_WIDTH_RANGE, range);
}

ccl_gl::~ccl_gl()
{
    glXMakeCurrent(dpy, None, NULL);
    glXDestroyContext(dpy, context);
}

#elif defined CR_WINDOWS

//像素格式
static PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA,
        32,
        0, 0, 0, 0, 0, 0,
        0,
        0,
        0,
        0, 0, 0, 0,
        16,
        0,
        0,
        PFD_MAIN_PLANE,
        0,
        0, 0, 0
};
/*
* 
*/

void SetupPixelFormat(HDC hDc)
{
    int nPixelFormat;
    nPixelFormat = ChoosePixelFormat(hDc, &pfd);
    SetPixelFormat(hDc, nPixelFormat, &pfd);
}

ccl_gl::ccl_gl(HDC hDc)
{
    _hDc = hDc;
    SetupPixelFormat(_hDc);
    _hRc = wglCreateContext(_hDc);
    wglMakeCurrent(_hDc, _hRc);
    /*
     * 创建环境完毕
     */
    InitGL();
}

ccl_gl::~ccl_gl()
{
    wglMakeCurrent(_hDc, NULL);
    wglDeleteContext(_hRc);
}

#endif

void _fill_port_(float r, float g, float b)
{
    glColor3f(r, g, b);
    glBegin(GL_QUADS);
    glVertex3f(1.0f, 1.0f, 0);
    glVertex3f(1.0f, -1.0f, 0);
    glVertex3f(-1.0f, -1.0f, 0);
    glVertex3f(-1.0f, 1.0f, 0);
    glEnd();
}

void _draw_titlebar_(CRINT32 _w, CRINT32 _h)
{
    glLoadIdentity();
    glOrtho(-1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f);
    //标题栏底色
    glViewport(0, _h, _w, CRUI_TITLEBAR_PIXEL);
    _fill_port_(0.2, 0.3, 0.35);
    //三个按钮
    glViewport(CRUI_TITLEBAR_PIXEL / 4, _h + CRUI_TITLEBAR_PIXEL / 4, CRUI_TITLEBAR_PIXEL / 2, CRUI_TITLEBAR_PIXEL / 2);
    _fill_port_(0.95, 0.45, 0.55);
    //
    glViewport(CRUI_TITLEBAR_PIXEL + CRUI_TITLEBAR_PIXEL / 4, _h + CRUI_TITLEBAR_PIXEL / 4, CRUI_TITLEBAR_PIXEL / 2, CRUI_TITLEBAR_PIXEL / 2);
    _fill_port_(0.55, 0.9, 0.55);
    //
    glViewport(CRUI_TITLEBAR_PIXEL * 2 + CRUI_TITLEBAR_PIXEL / 4, _h + CRUI_TITLEBAR_PIXEL / 4, CRUI_TITLEBAR_PIXEL / 2, CRUI_TITLEBAR_PIXEL / 2);
    _fill_port_(0.6, 0.65, 1.0);
    //
}

void ccl_gl::PaintAll()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //
    _draw_titlebar_(_w, _h);
    Resize(_w, _h);
    //
    DrawDemo();
#ifdef CR_WINDOWS
    SwapBuffers(_hDc);
#elif defined CR_LINUX
    glXSwapBuffers(dpy, w);
#endif
}

void ccl_gl::Resize(CRUINT32 x, CRUINT32 y)
{
    _w = x, _h = y;
    glViewport(0, 0, x, y);
    glLoadIdentity();
    if (x > y)
    {
        glOrtho(-100.0f * x / y, 100.0f * x / y, -100.0f, 100.0f, 10.0f, -1000.0f);
    }
    else
    {
        glOrtho(-100.0f, 100.0f, -100.0f * y / x, 100.0f * y / x, 10.0f, -1000.0f);
    }
}
