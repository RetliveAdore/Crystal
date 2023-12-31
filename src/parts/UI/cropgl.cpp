﻿#include <cropgl.hpp>
#include <math.h>
#include <crerrors.h>
#include <stdio.h>
#include <openglAPIs.h>

#define CRGL_RATIO 100.0f

typedef struct entityNode
{
    CRUIENTITY* pEty;
    CRUIENTITY Ety;
    CRUINT32 VAO;
    CRUINT32 VBO;
    CRUINT8* arrayBuffer;  //使用arrayBuffer将顶点缓存起来就不用每次都去算了
    CRUINT64 buffersize;
}CRUIENTITYNODE, *PCRUIENTITYNODE;

//一些常数
static const CRUINT32 n1 = 80;
static const CRUINT32 n2 = 8;
static const CRUINT32 nc = 3;

#ifdef CR_WINDOWS
static HMODULE glInst = NULL;

static void* crGetProcAddress(const char* name)
{
    PROC ret = wglGetProcAddress(name);
    if (ret == NULL)
    {
        ret = GetProcAddress(glInst, name);
    }
    if (!ret)
        printf("Failed load: %s\n", name);
    return ret;
}

CRBOOL CROpenGLInit()
{
    if (!(glInst = LoadLibraryA("opengl32.dll")))
        return CRFALSE;
    return CRTRUE;
}

void CROpenGLUninit()
{
    FreeLibrary(glInst);
}

#elif defined CR_LINUX
#include <dlfcn.h>
void* glInst;
static void* crGetProcAddress(const char* name)
{
    void* ret = (void*)glXGetProcAddress((const GLubyte*)name);
    if (ret == NULL)
    {
        ret = (void*)dlsym(glInst, name);
    }
    if (!ret)
        printf("Failed load: %s\n", name);
    return ret;
}
#include <stdio.h>
CRBOOL CROpenGLInit()
{
    glInst = dlopen("libOpenGL.so", RTLD_LAZY);
    if (!glInst)
        return CRFALSE;
    return CRTRUE;
}

void CROpenGLUninit()
{
    dlclose(glInst);
}

#endif

void _load_apis_(ccl_gl* pThis)
{
    pThis->glClearColor = (PGLCLEARCOLOR)crGetProcAddress("glClearColor");
    pThis->glClear = (PGLCLEAR)crGetProcAddress("glClear");
    pThis->glLoadIdentity = (PGLLOADIDENTITY)crGetProcAddress("glLoadIdentity");
    pThis->glViewport = (PGLVIEWPORT)crGetProcAddress("glViewport");
    pThis->glOrtho = (PGLORTHO)crGetProcAddress("glOrtho");
    pThis->glDisable = (PGLDISABLE)crGetProcAddress("glDisable");
    pThis->glEnable = (PGLENABLE)crGetProcAddress("glEnable");
    pThis->glBlendFunc = (PGLBLENDFUNC)crGetProcAddress("glBlendFunc");
    pThis->glGetString = (PGLGETSTRING)crGetProcAddress("glGetString");
    pThis->glBegin = (PGLBEGIN)crGetProcAddress("glBegin");
    pThis->glEnd = (PGLEND)crGetProcAddress("glEnd");
    pThis->glColor3f = (PGLCOLOR3F)crGetProcAddress("glColor3f");
    pThis->glColor4f = (PGLCOLOR4F)crGetProcAddress("glColor4f");
    pThis->glVertex3f = (PGLVERTEX3F)crGetProcAddress("glVertex3f");
}

void ccl_gl::DrawRect(float x1, float y1, float x2, float y2, float level, float stroke, CRCOLORF* pColor)
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
    stroke /= 2;

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

void ccl_gl::DrawElipse(float x, float y, float rx, float ry, float level, float stroke, CRCOLORF* pColor)
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
    stroke /= 2;

    glColor4f(pColor->r, pColor->g, pColor->b, pColor->a);
    glBegin(GL_QUADS);
    float lr_x = rx + stroke, sr_x = rx - stroke;
    float lr_y = ry + stroke, sr_y = ry - stroke;
    float tx_1 = x, tx_2 = x;
    float ty_1 = lr_y + y, ty_2 = sr_y + y;
    for (int i = 0; i <= n; i++)
    {
        glVertex3f(tx_1, ty_1, level);
        glVertex3f(tx_2, ty_2, level);
        tx_1 = sin(i * M_PI * 2 / n) * sr_x + x;
        tx_2 = sin(i * M_PI * 2 / n) * lr_x + x;
        ty_1 = cos(i * M_PI * 2 / n) * sr_y + y;
        ty_2 = cos(i * M_PI * 2 / n) * lr_y + y;
        glVertex3f(tx_2, ty_2, level);
        glVertex3f(tx_1, ty_1, level);
    }
    glEnd();
}

void ccl_gl::FillRect(float x1, float y1, float x2, float y2, float level, CRCOLORF* pColor)
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

void ccl_gl::FillElipse(float x, float y, float rx, float ry, float level, CRCOLORF* pColor)
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

void ccl_gl::DrawPoint(float x, float y, float level, float stroke, CRCOLORF* pColor)
{
    if (stroke < 0)
        stroke = -stroke;
    FillElipse(x, y, stroke / nc, stroke / nc, level, pColor);
}

void ccl_gl::DrawLine(float x1, float y1, float x2, float y2, float level, float stroke, CRCOLORF* pColor)
{
    if (stroke < 0)
        stroke = -stroke;
    level = -level;
    stroke /= 2;
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

void ccl_gl::InitGL()
{
    //glShadeModel(GL_SMOOTH);
    //需要透明度的时候不要启用depth，否则某些情况下画面会被裁掉
    //不过如果按照从后往前的顺序来，就不会有问题
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_DEPTH);
    //启用透明度通道
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //
    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_POLYGON_SMOOTH);

    //清屏黑
    glClearColor(0.0, 0.0, 0.0, 1.0);
    printf("OpenGL Version: %s\n", glGetString(GL_VERSION));
}

void _free_entity_pool_(CRLVOID data)
{
    delete (PCRUIENTITYNODE)data;
}

void _free_levels_(CRLVOID data)
{
    CRFreeStructure((CRSTRUCTURE)data, _free_entity_pool_);
}

#ifdef CR_LINUX
#include <string.h>
//查询线宽范围
//static float range[2];

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

#endif

#ifdef CR_WINDOWS
ccl_gl::ccl_gl(HDC hDc)
#elif defined CR_LINUX
ccl_gl::ccl_gl(Display* pDisplay, XVisualInfo* vi, Window win)
#endif
{
#ifdef CR_WINDOWS
    _hDc = hDc;
    SetupPixelFormat(_hDc);
    _hRc = wglCreateContext(_hDc);
    wglMakeCurrent(_hDc, _hRc);
#elif defined CR_LINUX
    dpy = pDisplay;
    w = win;
    context = glXCreateContext(dpy, vi, nullptr, GL_TRUE);
    glXMakeCurrent(dpy, w, context);
#endif
    _load_apis_(this);
    /*
     * 创建环境完毕
     */
    InitGL();
    //
    available = CRLinear();
    toremove = CRLinear();
    emptylevel = CRLinear();
    levels = CRTree();
    quadTree = CRQuadtree(5000, 5000, 4);  //暂且使用这个大小，肯定会遇到不够的时候的
}

ccl_gl::~ccl_gl()
{
#ifdef CR_WINDOWS
    wglMakeCurrent(_hDc, NULL);
    wglDeleteContext(_hRc);
#elif defined CR_LINUX
    glXMakeCurrent(dpy, None, NULL);
    glXDestroyContext(dpy, context);
#endif
    CRFreeStructure(available, NULL);
    CRFreeStructure(toremove, NULL);
    CRFreeStructure(emptylevel, NULL);
    CRFreeStructure(levels, _free_levels_);
    CRFreeTreextra(quadTree, NULL);
}

void ccl_gl::_fill_port_(float r, float g, float b)
{
    glColor3f(r, g, b);
    glBegin(GL_QUADS);
    glVertex3f(1.0f, 1.0f, 0);
    glVertex3f(1.0f, -1.0f, 0);
    glVertex3f(-1.0f, -1.0f, 0);
    glVertex3f(-1.0f, 1.0f, 0);
    glEnd();
}

void ccl_gl::_draw_titlebar_(CRINT32 _w, CRINT32 _h)
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
    _fill_port_(0.7, 0.6, 1.0);
    //
}

//sizebox要单独处理
void _copy_entity_(CRUIENTITY* dst, CRUIENTITY* src)
{
    dst->style = src->style;
    dst->stroke = src->stroke;
    dst->color = src->color;
    dst->key = src->key;
    dst->userdata = src->userdata;
}

void _paint_entities_(CRLVOID data, CRLVOID user, CRUINT64 key)
{
    PCRUIENTITYNODE node = (PCRUIENTITYNODE)data;
    ccl_gl* pgl = (ccl_gl*)user;
    if (node->pEty->invalid)
    {
        //此时就需要准备移除这个结点了
        CRLinPut(pgl->toremove, (CRLVOID)key, 0);
        CRUIENTITY* p = node->pEty;
        delete node;
        p->invalid = CRFALSE; //此时告诉用户，已经处理妥善，可以释放用户的pEty的内存了
        return; 
    }
    if (node->pEty->update)
    {
        _copy_entity_(&(node->Ety), node->pEty);
        node->pEty->update = CRFALSE;
    }
    if (node->pEty->enableEvent)
    {
        if (!node->Ety.enableEvent)
        {
            node->Ety.enableEvent = CRTRUE;
            CRQuadtreePushin(pgl->quadTree, node->Ety.sizeBox, (CRLVOID)node->Ety.id);
        }
        else if (node->pEty->moved)  //涉及到区域检索树的变换
        {
            node->pEty->moved = CRFALSE;
            node->Ety.sizeBox = node->pEty->sizeBox;
            CRQuadtreeRemove(pgl->quadTree, (CRLVOID)node->Ety.id);
            CRQuadtreePushin(pgl->quadTree, node->Ety.sizeBox, (CRLVOID)node->Ety.id);
        }
    }
    else if (node->Ety.enableEvent)
    {
        node->Ety.enableEvent = CRFALSE;
        CRQuadtreeRemove(pgl->quadTree, (CRLVOID)node->Ety.key);
    }
    if (!node->pEty->enableVision)
        return;
    switch (node->Ety.style_s.shape)
    {
    case CRUISHAPE_RECT:
    {
        if (node->Ety.style_s.type == CRUISTYLE_FILLED)
            pgl->FillRect(
                (node->Ety.sizeBox.left + 0.5) * pgl->ratio - pgl->dx, -(node->Ety.sizeBox.top * pgl->ratio - pgl->dy),
                (node->Ety.sizeBox.right + 0.5) * pgl->ratio - pgl->dx, -(node->Ety.sizeBox.bottom * pgl->ratio - pgl->dy),
                0, &node->Ety.color);
        else if (node->Ety.style_s.type == CRUISTYLE_COUNTOUR)
            pgl->DrawRect(
                (node->Ety.sizeBox.left + 0.5) * pgl->ratio - pgl->dx, -(node->Ety.sizeBox.top * pgl->ratio - pgl->dy),
                (node->Ety.sizeBox.right + 0.5) * pgl->ratio - pgl->dx, -(node->Ety.sizeBox.bottom * pgl->ratio - pgl->dy),
                0, node->Ety.stroke * pgl->ratio, &node->Ety.color);
        break;
    }
    case CRUISHAPE_ELIPSE:
    {
        float w = (float)(node->Ety.sizeBox.right - node->Ety.sizeBox.left) * pgl->ratio;
        float h = (float)(node->Ety.sizeBox.bottom - node->Ety.sizeBox.top) * pgl->ratio;
        float x = node->Ety.sizeBox.left / 2 + w * pgl->ratio - pgl->dx;
        float y = -(node->Ety.sizeBox.top / 2 + h * pgl->ratio - pgl->dy);
        w /= 2;
        h /= 2;
        if (node->Ety.style_s.type == CRUISTYLE_FILLED)
            pgl->FillElipse(x, y, w, h, 0, &node->Ety.color);
        else if (node->Ety.style_s.type == CRUISTYLE_COUNTOUR)
            pgl->DrawElipse(x, y, w, h, 0, node->Ety.stroke * pgl->ratio, &node->Ety.color);
        break;
    }
    default:
        break;
    }
}

void _paint_levels_(CRLVOID data, CRLVOID user, CRUINT64 key)
{
    CRSTRUCTURE entityPool = (CRSTRUCTURE)data;
    CRStructureForEach(entityPool, _paint_entities_, user);
    //然后是检查有没有需要移除的
    //如该层级的实体池空了，就移除该层级
    CRUINT64 keyEty;
    ccl_gl* pThis = (ccl_gl*)user;
    while (!CRLinGet(pThis->toremove, (CRLVOID*)&keyEty, 0))
        CRTreeGet(entityPool, NULL, keyEty);
    if (!CRStructureSize(entityPool))
    {
        CRFreeStructure(entityPool, NULL);
        CRLinPut(pThis->emptylevel, (CRLVOID)key, 0);
    }
}

CRCODE ccl_gl::AddEntity(CRUIENTITY* pEntity)
{
    pEntity->update = CRFALSE;
    pEntity->moved = CRFALSE;
    pEntity->invalid = CRFALSE;
    PCRUIENTITYNODE node = new CRUIENTITYNODE;
    //
    memcpy(&(node->Ety), pEntity, sizeof(CRUIENTITY));
    node->pEty = pEntity;
    //
    CRSTRUCTURE entityPool;  //tree
    if (CRTreeSeek(levels, &entityPool, node->Ety.level) == CRERR_NOTFOUND)
    {
        entityPool = CRTree();
        if (!entityPool)
        {
            delete node;
            return CRERR_OUTOFMEM;
        }
        CRTreePut(levels, entityPool, node->Ety.level);
    }
    CRUINT64 id;
    if (CRLinGet(available, (CRLVOID*)&id, 0))
        id = CurrentID++;
    CRTreePut(entityPool, node, id);
    if (CRQuadTreeCheck(quadTree, (CRLVOID)node->Ety.id))
    {
        CRThrowError(CRERR_CRUI_ENTITYCFLI, CRDES_CRUI_ENTITYCFLI);
        return CRERR_CRUI_ENTITYCFLI;
    }
    if (node->Ety.enableEvent)
        return CRQuadtreePushin(quadTree, node->Ety.sizeBox, (CRLVOID)node->Ety.id);
    return 0;
}

void ccl_gl::SeekEntity(CRPOINTU point, CRSTRUCTURE dyn)
{
    CRUINT64 id = 0;
    CRQuadtreeSearch(quadTree, point, dyn);
}

void ccl_gl::Ratio()
{
    ratio = CRGL_RATIO / (float)(_w < _h ? _w : _h) * 2;
    if (_w > _h)
    {
        dx = _w * ratio / 2;
        dy = CRGL_RATIO;
    }
    else
    {
        dx = CRGL_RATIO;
        dy = _h * ratio / 2;
    }
}

void ccl_gl::PaintAll()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //
    _draw_titlebar_(_w, _h);
    Resize(_w, _h);
    //
    //DrawDemo();
    CRStructureForEach(levels, _paint_levels_, this);
    CRUINT64 key;
    while (!CRLinGet(emptylevel, (CRLVOID*)&key, 0))
        CRTreeGet(levels, NULL, key);
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
        glOrtho(-CRGL_RATIO * x / y, CRGL_RATIO * x / y, -CRGL_RATIO, CRGL_RATIO, CRGL_RATIO / 10, -CRGL_RATIO * 100);
    }
    else
    {
        glOrtho(-CRGL_RATIO, CRGL_RATIO, -CRGL_RATIO * y / x, CRGL_RATIO * y / x, CRGL_RATIO / 10, -CRGL_RATIO * 100);
    }
    Ratio();
}

void _delete_vao_(CRLVOID data)
{
    
}

cr_vaovbo_pool::cr_vaovbo_pool()
{
    vaoPool = CRLinear();
    vboPool = CRLinear();
}

cr_vaovbo_pool::~cr_vaovbo_pool()
{
    CRFreeStructure(vaoPool, _delete_vao_);
}
