#ifndef _INCLUDE_CCLGL_HPP_
#define _INCLUDE_CCLGL_HPP_

#include <Crystal.h>
#include <parts/CrUI.h>

#ifdef CR_LINUX
#include <GL/glx.h>
#include <GL/glu.h>

class ccl_gl
{
public:
    ccl_gl(Display* pDisplay, XVisualInfo* vi, Window win);
    ~ccl_gl();
    // 禁止拷贝
    ccl_gl& operator=(const ccl_gl&) = delete;
    ccl_gl(const ccl_gl&) = delete;
public:
    void PaintAll();
    void Resize(CRUINT32 x, CRUINT32 y);
private:
    Display* dpy;
    Window w;

    //OpenGL的环境
    GLXContext context;

    CRUINT32 _w, _h;
    CRUINT32 CurrentID = 1;
    CRBOOL _lock;
};

#elif defined CR_WINDOWS //
#include <Windows.h>
#include <gl/GLU.h>
#include <corecrt_math_defines.h>
#pragma comment(lib, "opengl32.lib")

class ccl_gl
{
public:
    ccl_gl(HDC hDc);
    ~ccl_gl();
    // 禁止拷贝
    ccl_gl& operator=(const ccl_gl&) = delete;
    ccl_gl(const ccl_gl&) = delete;
public:
    void PaintAll();
    void Resize(CRUINT32 x, CRUINT32 y);
private:
    HDC _hDc;
    //OpenGL的环境
    HGLRC _hRc;

    CRUINT32 _w, _h;
    CRUINT32 CurrentID = 1;
    CRBOOL _lock;

    //
    CRSTRUCTURE available;  //queue
    CRSTRUCTURE toremove;

    //这里会用比较繁杂的数据结构嵌套来存储图像实体的先后关系及层级关系
    //层级数字越小越先绘制，同层级可能会随机绘制
    CRSTRUCTURE _tree;

    static void PaintItems(void* data, void* user, CRWINDOW idThis);

    friend void paint(void* data, void* user, CRWINDOW idThis);
};

#endif

#endif