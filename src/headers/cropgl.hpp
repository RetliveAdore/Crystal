#ifndef _INCLUDE_CCLGL_HPP_
#define _INCLUDE_CCLGL_HPP_

#include <Crystal.h>
#include <parts/CrUI.h>
#include <parts/CrTreeExtra.h>

#ifdef CR_LINUX
#include <GL/glx.h>
#include <GL/glu.h>

//这些方法是从CCL里面来的，就不改名了（反正都是我写的
class ccl_gl
{
public:
    void AddEntity(CRUIENTITY* pEntity);
    void Ratio();
    void PaintAll();
    void Resize(CRUINT32 x, CRUINT32 y);
public:
    ccl_gl(Display* pDisplay, XVisualInfo* vi, Window win);
    ~ccl_gl();
    // 禁止拷贝
    ccl_gl& operator=(const ccl_gl&) = delete;
    ccl_gl(const ccl_gl&) = delete;
private:
    Display* dpy;
    Window w;

    //OpenGL的环境
    GLXContext context;

    CRINT32 _w = 0, _h = 0;
    CRUINT32 CurrentID = 1;

    //用于坐标系变换——从OpenGL坐标变换为窗口坐标
    float ratio = 0, dx = 0, dy = 0;

    //
    CRSTRUCTURE available;  //linear
    CRSTRUCTURE toremove;  //线性表

    //这里会用比较繁杂的数据结构嵌套来存储图像实体的先后关系及层级关系
    //层级数字越小越先绘制，同层级可能会随机绘制
    CRSTRUCTURE levels;  //键值树，存储需要绘制的UI实体元素
    //levels存储的是层级，在每个层级中存储对应的items，层级按照从小到大依次渲染
    //每个层级内部的元素也有编号（编号可重合，但重合编号但是元素将随机顺序渲染）按照从小到大依次渲染

    CRTREEXTRA quadTree;
    friend void _paint_entities_(CRLVOID);
};

#elif defined CR_WINDOWS //
#include <Windows.h>
#include <gl/GLU.h>
#include <corecrt_math_defines.h>
#pragma comment(lib, "opengl32.lib")

class ccl_gl
{
public:
    void AddEntity(CRUIENTITY* pEntity);
    void Ratio();
    void PaintAll();
    void Resize(CRUINT32 x, CRUINT32 y);
public:
    ccl_gl(HDC hDc);
    ~ccl_gl();
    // 禁止拷贝
    ccl_gl& operator=(const ccl_gl&) = delete;
    ccl_gl(const ccl_gl&) = delete;
private:
    HDC _hDc;
    //OpenGL的环境
    HGLRC _hRc;

    CRINT32 _w = 0, _h = 0;
    CRUINT32 CurrentID = 1;

    //用于坐标系变换——从OpenGL坐标变换为窗口坐标
    float ratio = 0, dx = 0, dy = 0;

    //
    CRSTRUCTURE available;  //linear
    CRSTRUCTURE toremove;  //线性表

    //这里会用比较繁杂的数据结构嵌套来存储图像实体的先后关系及层级关系
    //层级数字越小越先绘制，同层级可能会随机绘制
    CRSTRUCTURE levels;  //键值树，存储需要绘制的UI实体元素
    //levels存储的是层级，在每个层级中存储对应的items，层级按照从小到大依次渲染
    //每个层级内部的元素也有编号（编号可重合，但重合编号但是元素将随机顺序渲染）按照从小到大依次渲染

    CRTREEXTRA quadTree;
    friend void _paint_entities_(CRLVOID);
};

#endif

#endif