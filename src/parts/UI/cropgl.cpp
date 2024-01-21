#include <cropgl.hpp>
#include <math.h>
#include <crerrors.h>
#include <stdio.h>
#include <string.h>
#include <openglAPIs.h>
#include <DefaultShaders.h>

#define CRGL_RATIO 1.0f

inline float _abs_(float num)
{
    return num > 0 ? num : -num;
}

const CRUINT32 rectElement[24] =
{
    0, 1, 5, 0, 5, 4,
    1, 2, 6, 1, 6, 5,
    2, 3, 7, 2, 7, 6,
    3, 0, 4, 3, 4, 7
};

const CRUINT32 rectFilledElement[6] =
{
    0, 1, 2, 0, 2, 3
};

typedef struct entityNode
{
    CRUIENTITY* pEty;
    CRUIENTITY Ety;
    CRUINT32 VAO;
    CRUINT32 VBO;
    CRUINT32 EBO;  //顶点索引信息
    CRUINT64 Texture;  //只有Entity风格为Bitmap时会用到，其余时刻忽略
    float* arrayBuffer;  //使用arrayBuffer将顶点缓存起来就不用每次都去算了
    CRUINT32 arraysize;
    CRUINT32* elementBuffer;  //索引缓冲
    CRUINT32 elementsize;
    CRUINT32 elementcount;
    ccl_gl* pThis;
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

void ccl_gl::_load_apis_()
{
    glGetString = (PGLGETSTRING)crGetProcAddress("glGetString");
    glClearColor = (PGLCLEARCOLOR)crGetProcAddress("glClearColor");
    glClear = (PGLCLEAR)crGetProcAddress("glClear");
    glLoadIdentity = (PGLLOADIDENTITY)crGetProcAddress("glLoadIdentity");
    glViewport = (PGLVIEWPORT)crGetProcAddress("glViewport");
    glOrtho = (PGLORTHO)crGetProcAddress("glOrtho");
    glDisable = (PGLDISABLE)crGetProcAddress("glDisable");
    glEnable = (PGLENABLE)crGetProcAddress("glEnable");
    glBlendFunc = (PGLBLENDFUNC)crGetProcAddress("glBlendFunc");
    glBegin = (PGLBEGIN)crGetProcAddress("glBegin");
    glEnd = (PGLEND)crGetProcAddress("glEnd");
    glColor3f = (PGLCOLOR3F)crGetProcAddress("glColor3f");
    glColor4f = (PGLCOLOR4F)crGetProcAddress("glColor4f");
    glVertex3f = (PGLVERTEX3F)crGetProcAddress("glVertex3f");
    glBindVertexArray = (PGLBINDVERTEXARRAY)crGetProcAddress("glBindVertexArray");
    glBindBuffer = (PGLBINDBUFFER)crGetProcAddress("glBindBuffer");
    glBindTexture = (PGLBINDTEXTURE)crGetProcAddress("glBindTexture");
    glVertexAttribPointer = (PGLVERTEXATTRIBPOINTER)crGetProcAddress("glVertexAttribPointer");
    glBufferData = (PGLBUFFERDATA)crGetProcAddress("glBufferData");
    glTexImage2D = (PGLTEXIMAGE2D)crGetProcAddress("glTexImage2D");
    glCreateShader = (PGLCREATESHADER)crGetProcAddress("glCreateShader");
    glDeleteShader = (PGLDELETESHADER)crGetProcAddress("glDeleteShader");
    glShaderSource = (PGLSHADERSOURCE)crGetProcAddress("glShaderSource");
    glCompileShader = (PGLCOMPILESHADER)crGetProcAddress("glCompileShader");
    glCreateProgram = (PGLCREATEPROGRAM)crGetProcAddress("glCreateProgram");
    glAttachShader = (PGLATTACHSHADER)crGetProcAddress("glAttachShader");
    glDetachShader = (PGLDETACHSHADER)crGetProcAddress("glDetachShader");
    glLinkProgram = (PGLLINKPROGRAM)crGetProcAddress("glLinkProgram");
    glDeleteProgram = (PGLDELETEPROGRAM)crGetProcAddress("glDeleteProgram");
    glUseProgram = (PGLUSEPROGRAM)crGetProcAddress("glUseProgram");
    glEnableVertexAttribArray = (PGLENABLEVERTEXATTRIBARRAY)crGetProcAddress("glEnableVertexAttribArray");
    glDisableVertexAttribArray = (PGLDISABLEVERTEXATTRIBARRAY)crGetProcAddress("glDisableVertexAttribArray");
    glDrawArrays = (PGLDRAWARRAYS)crGetProcAddress("glDrawArrays");
    glDrawElements = (PGLDRAWELEMENTS)crGetProcAddress("glDrawElements");
    glPolygonMode = (PGLPOLYGONMODE)crGetProcAddress("glPolygonMode");
    glUniform4f = (PGLUNIFORM4F)crGetProcAddress("glUniform4f");
    glUniform2f = (PGLUNIFORM2F)crGetProcAddress("glUniform2f");
    glGetUniformLoaction = (PGLGETUNIFORMLOCATION)crGetProcAddress("glGetUniformLocation");
    glTexParameteri = (PGLTEXPARAMETERI)crGetProcAddress("glTexParameteri");
    glTexParameterfv = (PGLTEXPARAMETERFV)crGetProcAddress("glTexParameterfv");
    glGenerateMipmap = (PGLGENERATEMIPMAP)crGetProcAddress("glGenerateMipmap");
}

void ccl_gl::GenRect(CRLVOID inner, float x1, float y1, float x2, float y2, float stroke, CRCOLORF* pColor)
{
    PCRUIENTITYNODE node = (PCRUIENTITYNODE)inner;
    if (node->arrayBuffer) delete[] node->arrayBuffer;
    node->arrayBuffer = new float[40];  //x,y,z,u,v
    node->arraysize = sizeof(float) * 40;
    if (node->elementBuffer) delete[] node->elementBuffer;
    node->elementBuffer = new CRUINT32[24];
    node->elementsize = sizeof(CRUINT32) * 24;
    node->elementcount = 24;
    
    x1 = (x1 + 0.5) * ratio - dx;
    x2 = (x2 + 0.5) * ratio - dx;
    y1 = -(y1 * ratio - dy);
    y2 = -(y2 * ratio - dy);
    stroke = stroke * ratio;
    if (node->Ety.texture)
    {
        PCRBITMAPINF inf = (PCRBITMAPINF)(node->Ety.texture);
        float dxUV = inf->uvRect.right - inf->uvRect.left;
        float dyUV = inf->uvRect.bottom - inf->uvRect.top;
        float dxA = x2 - x1;
        float dyA = y2 - y1;
        float strokeUV = stroke * (_abs_(dxUV) + _abs_(dyUV)) / (_abs_(dxA) + _abs_(dyA));
        node->arrayBuffer[3] = inf->uvRect.left;
        node->arrayBuffer[4] = inf->uvRect.top;
        node->arrayBuffer[8] = inf->uvRect.left;
        node->arrayBuffer[9] = inf->uvRect.bottom;
        node->arrayBuffer[13] = inf->uvRect.right;
        node->arrayBuffer[14] = inf->uvRect.bottom;
        node->arrayBuffer[18] = inf->uvRect.right;
        node->arrayBuffer[19] = inf->uvRect.top;
        //
        node->arrayBuffer[23] = inf->uvRect.left + strokeUV;
        node->arrayBuffer[24] = inf->uvRect.top - strokeUV;
        node->arrayBuffer[28] = inf->uvRect.left + strokeUV;
        node->arrayBuffer[29] = inf->uvRect.bottom + strokeUV;
        node->arrayBuffer[33] = inf->uvRect.right - strokeUV;
        node->arrayBuffer[34] = inf->uvRect.bottom + strokeUV;
        node->arrayBuffer[38] = inf->uvRect.right - strokeUV;
        node->arrayBuffer[39] = inf->uvRect.top - strokeUV;
    }
    else for (int i = 0; i < 40; i++) node->arrayBuffer[i] = 0.0f;
    node->arrayBuffer[0] = x1;
    node->arrayBuffer[1] = y1;
    node->arrayBuffer[2] = 0.0f;
    node->arrayBuffer[5] = x1;
    node->arrayBuffer[6] = y2;
    node->arrayBuffer[7] = 0.0f;
    node->arrayBuffer[10] = x2;
    node->arrayBuffer[11] = y2;
    node->arrayBuffer[12] = 0.0f;
    node->arrayBuffer[15] = x2;
    node->arrayBuffer[16] = y1;
    node->arrayBuffer[17] = 0.0f;
    //
    node->arrayBuffer[20] = x1 + stroke;
    node->arrayBuffer[21] = y1 - stroke;
    node->arrayBuffer[22] = 0.0f;
    node->arrayBuffer[25] = x1 + stroke;
    node->arrayBuffer[26] = y2 + stroke;
    node->arrayBuffer[27] = 0.0f;
    node->arrayBuffer[30] = x2 - stroke;
    node->arrayBuffer[31] = y2 + stroke;
    node->arrayBuffer[32] = 0.0f;
    node->arrayBuffer[35] = x2 - stroke;
    node->arrayBuffer[36] = y1 - stroke;
    node->arrayBuffer[37] = 0.0f;
    //
    memcpy(node->elementBuffer, rectElement, node->elementsize);
}

void ccl_gl::GenFilledRect(CRLVOID inner, float x1, float y1, float x2, float y2, CRCOLORF* pColor)
{
    PCRUIENTITYNODE node = (PCRUIENTITYNODE)inner;
    if (node->arrayBuffer) delete[] node->arrayBuffer;
    node->arrayBuffer = new float[20];  //x,y,z
    node->arraysize = sizeof(float) * 20;
    if (node->elementBuffer) delete[] node->elementBuffer;
    node->elementBuffer = new CRUINT32[6];
    node->elementsize = sizeof(CRUINT32) * 6;
    node->elementcount = 6;
    x1 = (x1 + 0.5) * ratio - dx;
    x2 = (x2 + 0.5) * ratio - dx;
    y1 = -(y1 * ratio - dy);
    y2 = -(y2 * ratio - dy);
    if (node->Ety.texture)
    {
        PCRBITMAPINF inf = (PCRBITMAPINF)(node->Ety.texture);
        node->arrayBuffer[3] = inf->uvRect.left;
        node->arrayBuffer[4] = inf->uvRect.top;
        node->arrayBuffer[8] = inf->uvRect.left;
        node->arrayBuffer[9] = inf->uvRect.bottom;
        node->arrayBuffer[13] = inf->uvRect.right;
        node->arrayBuffer[14] = inf->uvRect.bottom;
        node->arrayBuffer[18] = inf->uvRect.right;
        node->arrayBuffer[19] = inf->uvRect.top;
    }
    else for (int i = 0; i < 20; i++) node->arrayBuffer[i] = 0.0f;
    //
    node->arrayBuffer[0] = x1;
    node->arrayBuffer[1] = y1;
    node->arrayBuffer[2] = 0.0f;
    node->arrayBuffer[5] = x1;
    node->arrayBuffer[6] = y2;
    node->arrayBuffer[7] = 0.0f;
    node->arrayBuffer[10] = x2;
    node->arrayBuffer[11] = y2;
    node->arrayBuffer[12] = 0.0f;
    node->arrayBuffer[15] = x2;
    node->arrayBuffer[16] = y1;
    node->arrayBuffer[17] = 0.0f;
    //
    memcpy(node->elementBuffer, rectFilledElement, node->elementsize);
}

void ccl_gl::GenElipse(CRLVOID inner, float x, float y, float rx, float ry, float stroke, CRCOLORF* pColor)
{
    PCRUIENTITYNODE node = (PCRUIENTITYNODE)inner;
    if (node->arrayBuffer) delete[] node->arrayBuffer;
    node->arrayBuffer = new float[320];  //x,y,z
    node->arraysize = sizeof(float) * 320;
    //总共64个三角形
    if (node->elementBuffer) delete[] node->elementBuffer;
    node->elementBuffer = new CRUINT32[192];
    node->elementsize = sizeof(CRUINT32) * 192;
    node->elementcount = 192;
    x = x * ratio - dx;
    y = -(y * ratio - dy);
    rx *= ratio;
    ry *= -ratio;
    stroke = stroke * ratio;
    float l_x = rx;
    float l_y = ry;
    float s_x = rx - stroke;
    float s_y = ry + stroke;
    if (node->Ety.texture)
    {
        PCRBITMAPINF inf = (PCRBITMAPINF)(node->Ety.texture);
        float rxUV = (inf->uvRect.right - inf->uvRect.left) / 2;
        float ryUV = (inf->uvRect.bottom - inf->uvRect.top) / 2;
        float xUV = inf->uvRect.left + rxUV;
        float yUV = inf->uvRect.top + ryUV;
        float strokeUV = stroke * (_abs_(rxUV) + _abs_(ryUV)) / (_abs_(rx) + _abs_(ry));
        float x_sUV = rxUV - strokeUV;
        float y_sUV = ryUV + strokeUV;
        for (int i = 0; i < 32; i++)
        {
            node->arrayBuffer[i * 10 + 3] = sin(i * M_PI * 2 / 32) * rxUV + xUV;
            node->arrayBuffer[i * 10 + 4] = cos(i * M_PI * 2 / 32) * ryUV + yUV;
            node->arrayBuffer[i * 10 + 8] = sin(i * M_PI * 2 / 32) * x_sUV + xUV;
            node->arrayBuffer[i * 10 + 9] = cos(i * M_PI * 2 / 32) * y_sUV + yUV;
        }
    }
    else for (int i = 0; i < 320; i++) node->arrayBuffer[i] = 0.0f;
    for (int i = 0; i < 32; i++)  //48个三角形
    {
        node->arrayBuffer[i * 10] = sin(i * M_PI * 2 / 32) * l_x + x;
        node->arrayBuffer[i * 10 + 1] = cos(i * M_PI * 2 / 32) * l_y + y;
        node->arrayBuffer[i * 10 + 2] = 0.0f;
        node->arrayBuffer[i * 10 + 5] = sin(i * M_PI * 2 / 32) * s_x + x;
        node->arrayBuffer[i * 10 + 6] = cos(i * M_PI * 2 / 32) * s_y + y;
        node->arrayBuffer[i * 10 + 7] = 0.0f;
    }
    for (int i = 0; i < 32; i++)
    {
        node->elementBuffer[i * 6] = i * 2;
        node->elementBuffer[i * 6 + 1] = i * 2 + 3;
        node->elementBuffer[i * 6 + 2] = i * 2 + 1;
        node->elementBuffer[i * 6 + 3] = i * 2;
        node->elementBuffer[i * 6 + 4] = i * 2 + 2;
        node->elementBuffer[i * 6 + 5] = i * 2 + 3;
    }
    node->elementBuffer[187] = 1;
    node->elementBuffer[190] = 0;
    node->elementBuffer[191] = 1;
}

void ccl_gl::GenFilledElipse(CRLVOID inner, float x, float y, float rx, float ry, CRCOLORF* pColor)
{
    PCRUIENTITYNODE node = (PCRUIENTITYNODE)inner;
    if (node->arrayBuffer) delete[] node->arrayBuffer;
    node->arrayBuffer = new float[165];  //x,y,z
    node->arraysize = sizeof(float) * 165;
    //总共32个三角形，每一象限8个
    if (node->elementBuffer) delete[] node->elementBuffer;
    node->elementBuffer = new CRUINT32[96];
    node->elementsize = sizeof(CRUINT32) * 96;
    node->elementcount = 96;
    x = x * ratio - dx;
    y = -(y * ratio - dy);
    rx *= ratio;
    ry *= ratio;
    if (node->Ety.texture)
    {
        PCRBITMAPINF inf = (PCRBITMAPINF)(node->Ety.texture);
        float rxUV = (inf->uvRect.right - inf->uvRect.left) / 2;
        float ryUV = (inf->uvRect.bottom - inf->uvRect.top) / 2;
        float xUV = inf->uvRect.left + rxUV;
        float yUV = inf->uvRect.top + ryUV;
        node->arrayBuffer[3] = xUV;
        node->arrayBuffer[4] = yUV;
        for (int i = 1; i < 33; i++)
        {
            node->arrayBuffer[i * 5 + 3] = sin(i * M_PI * 2 / 32) * rxUV + xUV;
            node->arrayBuffer[i * 5 + 4] = cos(i * M_PI * 2 / 32) * rxUV + xUV;
        }
    }
    else for (int i = 0; i < 165; i++) node->arrayBuffer[i] = 0.0f;
    node->arrayBuffer[0] = x;
    node->arrayBuffer[1] = y;
    node->arrayBuffer[2] = 0.0f;
    for (int i = 1; i < 33; i++)  //24个三角形
    {
        node->arrayBuffer[i * 5] = sin(i * M_PI * 2 / 32) * rx + x;
        node->arrayBuffer[i * 5 + 1] = cos(i * M_PI * 2 / 32) * ry + y;
        node->arrayBuffer[i * 5 + 2] = 0.0f;
    }
    for (int i = 0; i < 32; i++)
    {
        node->elementBuffer[i * 3] = 0;
        node->elementBuffer[i * 3 + 1] = i + 1;
        node->elementBuffer[i * 3 + 2] = i + 2;
    }
    node->elementBuffer[95] = 1;
}

void ccl_gl::GenNode(CRLVOID inner)
{
    PCRUIENTITYNODE node = (PCRUIENTITYNODE)inner;
    //生成顶点数组
    switch (node->Ety.style_s.shape)
    {
    case CRUISHAPE_RECT:
    {
        if (node->Ety.style_s.type == CRUISTYLE_FILLED)
        {
            GenFilledRect(node, node->Ety.sizeBox.left, node->Ety.sizeBox.top, node->Ety.sizeBox.right, node->Ety.sizeBox.bottom,
                &node->Ety.color);
        }
        else
        {
            GenRect(node, node->Ety.sizeBox.left, node->Ety.sizeBox.top, node->Ety.sizeBox.right, node->Ety.sizeBox.bottom,
                node->Ety.stroke, &node->Ety.color);
        }
        break;
    }
    case CRUISHAPE_ELIPSE:
    {
        float rx = (node->Ety.sizeBox.right - node->Ety.sizeBox.left) / 2;
        float ry = (node->Ety.sizeBox.bottom - node->Ety.sizeBox.top) / 2;
        float x = rx + node->Ety.sizeBox.left;
        float y = ry + node->Ety.sizeBox.top;
        if (node->Ety.style_s.type == CRUISTYLE_FILLED)
        {
            GenFilledElipse(node, x, y, rx, ry, &node->Ety.color);
        }
        else
        {
            GenElipse(node, x, y, rx, ry, node->Ety.stroke, &node->Ety.color);
        }
        break;
    }
    default:
        break;
    }
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

    //清屏黑
    glClearColor(0.0, 0.0, 0.0, 1.0);
    printf("OpenGL Version: %s\n", glGetString(GL_VERSION));
}

void _free_entity_pool_(CRLVOID data)
{
    PCRUIENTITYNODE node = (PCRUIENTITYNODE)data;
    node->pThis->pool->ReleaseVAO(node->VAO);
    node->pThis->pool->ReleaseBuffer(node->VBO);
    node->pThis->pool->ReleaseBuffer(node->EBO);
    delete node->arrayBuffer;
    delete node->elementBuffer;
    delete node;
}

void _free_levels_(CRLVOID data)
{
    CRFreeStructure((CRSTRUCTURE)data, _free_entity_pool_);
}

void _free_textures_(CRLVOID data, CRLVOID user, CRUINT64 id)
{
    ccl_gl* pgl = (ccl_gl*)user;
    pgl->pool->ReleaseTexture((CRUINT32)(CRUINT64)data);
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
    _load_apis_();
    pool = new cr_vertex_buffer_pool();
    //
    publicTexture = pool->GetTexture();
    glBindTexture(GL_TEXTURE_2D, publicTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &whiteColor);
    //
    VertexShader = glCreateShader(GL_VERTEX_SHADER);
    FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(VertexShader, 1, &CrVertexShaderSource1, NULL);
    glShaderSource(FragmentShader, 1, &CRFragmentShaderSource1, NULL);
    glCompileShader(VertexShader);
    glCompileShader(FragmentShader);  //所有的UI实体绘制都使用相同的简单shader

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, VertexShader);
    glAttachShader(shaderProgram, FragmentShader);
    glLinkProgram(shaderProgram);

    colorLocation = glGetUniformLoaction(shaderProgram, "paintColor");
    aspLocation = glGetUniformLoaction(shaderProgram, "asp");
    /*
     * 创建环境完毕
     */
    InitGL();
    //
    available = CRLinear();
    toremove = CRLinear();
    emptylevel = CRLinear();
    levels = CRTree();
    existTexture = CRTree();
    existTextureToken = CRTree();
    quadTree = CRQuadtree(QUAD_RANGE, QUAD_RANGE, 4);  //暂且使用这个大小，肯定会遇到不够的时候的
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
    //
    CRFreeStructure(available, NULL);
    CRFreeStructure(toremove, NULL);
    CRFreeStructure(emptylevel, NULL);
    CRFreeStructure(levels, _free_levels_);
    //手动预释放纹理（尚未实际删除，在deletepool之后就真正意义上删除了）
    CRStructureForEach(existTexture, _free_textures_, this);
    CRFreeStructure(existTexture, NULL);
    CRFreeStructure(existTextureToken, NULL);
    //
    CRFreeTreextra(quadTree, NULL);
    //
    glDetachShader(shaderProgram, VertexShader);
    glDetachShader(shaderProgram, FragmentShader);  //解绑然后分别释放
    glDeleteProgram(shaderProgram);
    glDeleteShader(VertexShader);
    glDeleteShader(FragmentShader);
    delete pool;
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
    //dst->texture = src->texture;
}

void _paint_entities_(CRLVOID data, CRLVOID user, CRUINT64 key)
{
    PCRUIENTITYNODE node = (PCRUIENTITYNODE)data;
    ccl_gl* pgl = (ccl_gl*)user;
    //
    if (node->pEty->invalid)
    {
        //此时就需要准备移除这个结点了
        CRLinPut(pgl->toremove, (CRLVOID)key, 0);
        CRUIENTITY* p = node->pEty;
        //判断一下纹理
        CRUINT64 token = 0;
        if (CRTreeSeek(pgl->existTextureToken, (CRLVOID*)&token, (CRUINT64)node->Ety.texture))
        {  //找到了token
            token--;
            if (token > 0) CRTreePut(pgl->existTextureToken, (CRLVOID)token, (CRUINT64)node->Ety.texture);
            else  //token归零，需要回收texture ID
            {
                CRTreeGet(pgl->existTexture, NULL, (CRUINT64)node->Ety.texture);
                pgl->pool->ReleaseTexture(node->Texture);
            }
        }
        //
        delete node;
        p->invalid = CRFALSE; //此时告诉用户，已经处理妥善，可以释放用户的pEty的内存了
        return; 
    }
    if (node->pEty->update)
    {
        _copy_entity_(&(node->Ety), node->pEty);
        //纹理发生变化要单独处理

        //
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
            //node->pEty->moved = CRFALSE;
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
    //这是初始化VBO和EBO唯一的办法，没有办法跨线程生成VBO之类的
    //考虑转战vulkan了
    //OpenGL在这些方面就是一坨大便
    if (!node->VAO)
    {
        node->VAO = pgl->pool->GetVAO();
        pgl->glBindVertexArray(node->VAO);
        node->VBO = pgl->pool->GetBuffer();
        pgl->glBindBuffer(GL_ARRAY_BUFFER, node->VBO);
        //VAO、VBO创建完毕
        pgl->glBufferData(GL_ARRAY_BUFFER, node->arraysize, node->arrayBuffer, GL_STATIC_DRAW);
        pgl->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        pgl->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        pgl->glEnableVertexAttribArray(0);
        pgl->glEnableVertexAttribArray(1);
        //创建EBO
        node->EBO = pgl->pool->GetBuffer();
        pgl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, node->EBO);
        pgl->glBufferData(GL_ELEMENT_ARRAY_BUFFER, node->elementsize, node->elementBuffer, GL_STATIC_DRAW);
        //创建Texture
        if (node->Ety.texture)
        {
            if (CRTreeSeek(pgl->existTexture, (CRLVOID*)&node->Texture, (CRUINT64)node->Ety.texture))
            {  //需要生成的情况
                node->Texture = pgl->pool->GetTexture();
                pgl->glBindTexture(GL_TEXTURE_2D, node->Texture);
                PCRBITMAPINF inf = (PCRBITMAPINF)node->Ety.texture;
                pgl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, inf->w, inf->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, inf->pixels);
                CRTreePut(pgl->existTexture, (CRLVOID)node->Texture, (CRUINT64)node->Ety.texture);
                CRTreePut(pgl->existTextureToken, (CRLVOID)1, (CRUINT64)node->Ety.texture);
            }
            else
            {
                CRUINT64 token = 0;
                CRTreeSeek(pgl->existTextureToken, (CRLVOID*)&token, (CRUINT64)node->Ety.texture);
                token++;
                CRTreePut(pgl->existTextureToken, (CRLVOID)token, (CRUINT64)node->Ety.texture);
            }
        }
        else node->Texture = pgl->publicTexture;
        pgl->glGenerateMipmap(GL_TEXTURE_2D);
        //此时这些数据已经进入显存了，可以释放掉节省内存
        //贴图数据因为是用户提供的（不是内部生成的），所以说是否释放交给用户处理
        //而且跨库跨线程也无法释放用户给的数据
        delete[] node->arrayBuffer;
        delete[] node->elementBuffer;
        node->arraysize = 0;
        node->elementsize = 0;
        node->arrayBuffer = nullptr;
        node->elementBuffer = nullptr;
    }
    pgl->glBindVertexArray(node->VAO);
    pgl->glBindTexture(GL_TEXTURE_2D, node->Texture);
    if (node->pEty->moved)
    {
        pgl->glBindBuffer(GL_ARRAY_BUFFER, node->VBO);
        pgl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, node->EBO);
        node->Ety.sizeBox = node->pEty->sizeBox;
        pgl->GenNode(node);
        pgl->glBufferData(GL_ARRAY_BUFFER, node->arraysize, node->arrayBuffer, GL_STATIC_DRAW);
        pgl->glBufferData(GL_ELEMENT_ARRAY_BUFFER, node->elementsize, node->elementBuffer, GL_STATIC_DRAW);
        //此时这些数据已经进入显存了，可以释放掉节省内存
        delete[] node->arrayBuffer;
        delete[] node->elementBuffer;
        node->arraysize = 0;
        node->elementsize = 0;
        node->arrayBuffer = nullptr;
        node->elementBuffer = nullptr;
        node->pEty->moved = CRFALSE;
    }
    if (node->pEty->enableVision)
    {
        pgl->glUniform2f(pgl->aspLocation, pgl->aspx, pgl->aspy);
        pgl->glUniform4f(pgl->colorLocation, node->Ety.color.r, node->Ety.color.g, node->Ety.color.b, node->Ety.color.a);
        pgl->glDrawElements(GL_TRIANGLES, node->elementcount, GL_UNSIGNED_INT, 0);
    }
    pgl->glBindVertexArray(0);
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
    node->pThis = this;
    node->VAO = 0;
    node->VBO = 0;
    node->EBO = 0;
    node->Texture = 0;
    node->arrayBuffer = nullptr;
    node->elementBuffer = nullptr;
    node->arraysize = 0;
    node->elementsize = 0;
    GenNode(node);
    //
    CRSTRUCTURE entityPool;  //tree
    if (CRTreeSeek(levels, &entityPool, node->Ety.level) == CRERR_NOTFOUND)
    {
        entityPool = CRTree();
        if (!entityPool)
        {
            pool->ReleaseBuffer(node->VBO);
            pool->ReleaseBuffer(node->EBO);
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
        aspx = (float)_h / (float)_w;
        aspy = 1.0;
    }
    else
    {
        dx = CRGL_RATIO;
        dy = _h * ratio / 2;
        aspx = 1.0f;
        aspy = (float)_w / (float)_h;
    }
}

void ccl_gl::PaintAll()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //
    _draw_titlebar_(_w, _h);
    Resize(_w, _h);
    //
    glUseProgram(shaderProgram);
    CRStructureForEach(levels, _paint_levels_, this);
    glUseProgram(0);  //有效的，不过记得还原到默认shader
    //
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
    Ratio();
}

void cr_vertex_buffer_pool::_load_apis_()
{
    glGenVertexArrays = (PGLGENVERTEXARRAYS)crGetProcAddress("glGenVertexArrays");
    glDeleteVertexArrays = (PGLDELETEVERTEXARRAYS)crGetProcAddress("glDeleteVertexArrays");
    glGenBuffers = (PGLGENBUFFERS)crGetProcAddress("glGenBuffers");
    glDeleteBuffers = (PGLDELETEBUFFERS)crGetProcAddress("glDeleteBuffers");
    glGenTextures = (PGLGENTEXTURES)crGetProcAddress("glGenTextures");
    glDeleteTextures = (PGLDELETETEXTURES)crGetProcAddress("glDeleteTextures");
}

cr_vertex_buffer_pool::cr_vertex_buffer_pool()
{
    _load_apis_();
    vaoPool = CRDynamicPtr();
    bufferPool = CRDynamicPtr();
    texPool = CRDynamicPtr();
}

cr_vertex_buffer_pool::~cr_vertex_buffer_pool()
{
    CRUINT32 size = 0;
    CRUINT32* buffer = (CRUINT32*)CRDynCopy(vaoPool, &size);
    if (buffer) glDeleteVertexArrays(size, buffer);
    CRDynFreeCopy(buffer);
    //
    buffer = (CRUINT32*)CRDynCopy(bufferPool, &size);
    if (buffer) glDeleteBuffers(size, buffer);
    CRDynFreeCopy(buffer);
    //
    buffer = (CRUINT32*)CRDynCopy(texPool, &size);
    if (buffer) glDeleteTextures(size, buffer);
    CRDynFreeCopy(buffer);
    //
    CRFreeStructure(vaoPool, NULL);
    CRFreeStructure(bufferPool, NULL);
    CRFreeStructure(texPool, NULL);
}

unsigned int cr_vertex_buffer_pool::GetVAO()
{
    unsigned int VAO = 0;
    if (CRStructureSize(vaoPool))
        CRDynPopPtr(vaoPool, (CRLVOID*)&VAO);
    else
        glGenVertexArrays(1, &VAO);
    return VAO;
}

unsigned int cr_vertex_buffer_pool::GetBuffer()
{
    CRUINT32 BUFFER = 0;
    if (CRStructureSize(bufferPool))
        CRDynPopPtr(bufferPool, (CRLVOID*)&BUFFER);
    else
        glGenBuffers(1, &BUFFER);
    return BUFFER;
}

unsigned int cr_vertex_buffer_pool::GetTexture()
{
    CRUINT32 Texture = 0;
    if (CRStructureSize(texPool))
        CRDynPopPtr(texPool, (CRLVOID*)&Texture);
    else
        glGenTextures(1, &Texture);
    return Texture;
}

CRCODE cr_vertex_buffer_pool::ReleaseVAO(unsigned int vao)
{
    CRCODE code =
    CRDynPushPtr(vaoPool, (CRLVOID)(CRUINT64)vao);
    if (code)
        glDeleteVertexArrays(1, &vao);  //假如无法放进池子，就地释放
    return code;
}

CRCODE cr_vertex_buffer_pool::ReleaseBuffer(unsigned int buffer)
{
    CRCODE code =
    CRDynPushPtr(bufferPool, (CRLVOID)(CRUINT64)buffer);
    if (code)
        glDeleteBuffers(1, &buffer);
    return code;
}

CRCODE cr_vertex_buffer_pool::ReleaseTexture(unsigned int texture)
{
    CRCODE code =
    CRDynPushPtr(texPool, (CRLVOID)(CRUINT64)texture);
    if (code)
        glDeleteTextures(1, &texture);
    return code;
}