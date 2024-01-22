#ifndef _INCLUDE_CCLGL_HPP_
#define _INCLUDE_CCLGL_HPP_
#include <openglAPIs.h>
#include <Crystal.h>
#include <parts/CrUI.h>
#include <parts/CrTreeExtra.h>
#include <freetype/freetype.h>

extern FT_Library ftLib;
#define QUAD_RANGE 4000

#ifdef CR_WINDOWS
#include <Windows.h>
#include <corecrt_math_defines.h>
#pragma comment(lib, "opengl32.lib")

#elif defined CR_LINUX
#include <GL/glx.h>
#endif

CRBOOL CROpenGLInit();
void CROpenGLUninit();

// VAO: Vertex Array Object
// VBO: Vertex Buffer Object
// EBO(IBO): Element(Index) Buffer Object
//听说有些地方释放VAO会失败。。。
//那干脆用一个池子来管理，遵循闲置VAO和VBO转让机制
class cr_vertex_buffer_pool
{
public:
    unsigned int GetVAO();
    unsigned int GetBuffer();
    unsigned int GetTexture();
    CRCODE ReleaseVAO(unsigned int vao);
    CRCODE ReleaseBuffer(unsigned int buffer);
    CRCODE ReleaseTexture(unsigned int texture);
public:
    cr_vertex_buffer_pool();
    ~cr_vertex_buffer_pool();
    cr_vertex_buffer_pool& operator=(const cr_vertex_buffer_pool&) = delete;
    cr_vertex_buffer_pool(const cr_vertex_buffer_pool&) = delete;
private:
    void _load_apis_();
private:
    CRSTRUCTURE vaoPool;
    CRSTRUCTURE bufferPool;
    CRSTRUCTURE texPool;

    //严重警告，下面的所有函数都不支持跨线程调用，否则会立即出现野指针错误
    //一坨大便
    CR_GLAPI PGLGENVERTEXARRAYS glGenVertexArrays;
    CR_GLAPI PGLDELETEVERTEXARRAYS glDeleteVertexArrays;
    CR_GLAPI PGLGENBUFFERS glGenBuffers;
    CR_GLAPI PGLDELETEBUFFERS glDeleteBuffers;
    CR_GLAPI PGLGENTEXTURES glGenTextures;
    CR_GLAPI PGLDELETETEXTURES glDeleteTextures;
};

//这些方法是从CCL里面来的，就不改名了
class ccl_gl
{
public:
    CRCODE AddEntity(CRUIENTITY* pEntity);
    void Ratio();
    void PaintAll();
    void Resize(CRUINT32 x, CRUINT32 y);
    void SeekEntity(CRPOINTU point, CRSTRUCTURE dyn);
public:
#ifdef CR_WINDOWS
    ccl_gl(HDC hDc);
    ~ccl_gl();
    // 禁止拷贝
    ccl_gl& operator=(const ccl_gl&) = delete;
    ccl_gl(const ccl_gl&) = delete;
#elif defined CR_LINUX
    ccl_gl(Display* pDisplay, XVisualInfo* vi, Window win);
    ~ccl_gl();
    // 禁止拷贝
    ccl_gl& operator=(const ccl_gl&) = delete;
    ccl_gl(const ccl_gl&) = delete;
#endif
private:
    void InitGL();
    void DrawLine(float x1, float y1, float x2, float y2, float level, float stroke, CRCOLORF* pColor);
    //
    void GenRect(CRLVOID inner, float x1, float y1, float x2, float y2, float stroke, CRCOLORF* pColor);
    void GenFilledRect(CRLVOID inner, float x1, float y1, float x2, float y2, CRCOLORF* pColor);
    void GenElipse(CRLVOID inner, float x, float y, float rx, float ry, float stroke, CRCOLORF* pColor);
    void GenFilledElipse(CRLVOID inner, float x, float y, float rx, float ry, CRCOLORF* pColor);
    void GenLine(CRLVOID inner, float x1, float y1, float x2, float y2, float stroke, CRCOLORF* pColor);
    
    void GenNode(CRLVOID inner);

    //这个接口的设计思路是，每次绘制一个字符，一个一个组成一行文字
    //此API负责画，画什么由外部调用决定，传入UNICODE编码和坐标进行绘制
    void DrawFont(wchar_t ch, CRUINT32 x, CRUINT32 y);
    //中文的宽度为一单位，高度为一单位，英文的小写宽度为0.5单位，高度为一单位
    void SetFontSize(CRUINT32 w, CRUINT32 h);

    void _load_apis_();
    void _fill_port_(float r, float g, float b);
    void _draw_titlebar_(CRINT32 _w, CRINT32 _h);
private:
#ifdef CR_WINDOWS
    HDC _hDc;
    //OpenGL的环境
    HGLRC _hRc;
#elif defined CR_LINUX
    Display* dpy;
    Window w;
    //OpenGL的环境
    GLXContext context;
#endif
    cr_vertex_buffer_pool* pool;
    CRUINT32 VertexShader;
    CRUINT32 FragmentShader;
    CRUINT32 shaderProgram;
    CRINT32 colorLocation;
    CRINT32 aspLocation;
    CRINT32 texture0;
    CRUINT32 publicTexture;
    CRCOLORU whiteColor = {255, 255, 255, 255};  //1x1纹理，主打一个节省空间
    float aspx = 1.0;
    float aspy = 1.0;

    CR_GLAPI PGLGETSTRING glGetString;
    CR_GLAPI PGLCLEARCOLOR glClearColor;
    CR_GLAPI PGLCLEAR glClear;
    CR_GLAPI PGLLOADIDENTITY glLoadIdentity;
    CR_GLAPI PGLVIEWPORT glViewport;
    CR_GLAPI PGLORTHO glOrtho;
    CR_GLAPI PGLDISABLE glDisable;
    CR_GLAPI PGLENABLE glEnable;
    CR_GLAPI PGLBLENDFUNC glBlendFunc;
    CR_GLAPI PGLBEGIN glBegin;
    CR_GLAPI PGLEND glEnd;
    CR_GLAPI PGLCOLOR3F glColor3f;
    CR_GLAPI PGLCOLOR4F glColor4f;
    CR_GLAPI PGLVERTEX3F glVertex3f;
    CR_GLAPI PGLBINDVERTEXARRAY glBindVertexArray;
    CR_GLAPI PGLBINDBUFFER glBindBuffer;
    CR_GLAPI PGLBINDTEXTURE glBindTexture;
    CR_GLAPI PGLVERTEXATTRIBPOINTER glVertexAttribPointer;
    CR_GLAPI PGLBUFFERDATA glBufferData;
    CR_GLAPI PGLTEXIMAGE2D glTexImage2D;
    CR_GLAPI PGLCREATESHADER glCreateShader;
    CR_GLAPI PGLDELETESHADER glDeleteShader;
    CR_GLAPI PGLSHADERSOURCE glShaderSource;
    CR_GLAPI PGLCOMPILESHADER glCompileShader;
    CR_GLAPI PGLCREATEPROGRAM glCreateProgram;
    CR_GLAPI PGLDELETEPROGRAM glDeleteProgram;
    CR_GLAPI PGLATTACHSHADER glAttachShader;
    CR_GLAPI PGLDETACHSHADER glDetachShader;
    CR_GLAPI PGLLINKPROGRAM glLinkProgram;
    CR_GLAPI PGLUSEPROGRAM glUseProgram;
    CR_GLAPI PGLENABLEVERTEXATTRIBARRAY glEnableVertexAttribArray;
    CR_GLAPI PGLDISABLEVERTEXATTRIBARRAY glDisableVertexAttribArray;
    CR_GLAPI PGLDRAWARRAYS glDrawArrays;
    CR_GLAPI PGLDRAWELEMENTS glDrawElements;
    CR_GLAPI PGLPOLYGONMODE glPolygonMode;
    CR_GLAPI PGLUNIFORM1I glUniform1i;
    CR_GLAPI PGLUNIFORM2F glUniform2f;
    CR_GLAPI PGLUNIFORM4F glUniform4f;
    CR_GLAPI PGLGETUNIFORMLOCATION glGetUniformLoaction;
    CR_GLAPI PGLTEXPARAMETERI glTexParameteri;
    CR_GLAPI PGLTEXPARAMETERFV glTexParameterfv;
    CR_GLAPI PGLGENERATEMIPMAP glGenerateMipmap;
    CR_GLAPI PGLACTIVETEXTURE glActiveTexture;

    CRINT32 _w = 0, _h = 0;
    CRUINT32 CurrentID = 1;

    //用于坐标系变换——从OpenGL坐标变换为窗口坐标
    float ratio = 0, dx = 0, dy = 0;

    //
    CRSTRUCTURE available;  //linear
    CRSTRUCTURE toremove;  //线性表
    CRSTRUCTURE emptylevel;  //线性表

    //这里会用比较繁杂的数据结构嵌套来存储图像实体的先后关系及层级关系
    //层级数字越小越先绘制，同层级可能会随机绘制
    CRSTRUCTURE levels;  //键值树，存储需要绘制的UI实体元素
    //levels存储的是层级，在每个层级中存储对应的items，层级按照从小到大依次渲染
    //每个层级内部的元素也有编号（编号可重合，但重合编号但是元素将随机顺序渲染）按照从小到大依次渲染
    CRSTRUCTURE existTexture;  //用于存储已经存在的纹理，避免重复创建
    CRSTRUCTURE existTextureToken;  //记录每一个纹理的引用次数，如果归零就删除纹理

    CRTREEXTRA quadTree;
    friend void _paint_entities_(CRLVOID, CRLVOID, CRUINT64);
    friend void _paint_levels_(CRLVOID, CRLVOID, CRUINT64);
    friend void _free_entity_pool_(CRLVOID);
    friend void _free_textures_(CRLVOID, CRLVOID, CRUINT64);
};

#endif