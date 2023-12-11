#include <Crystal.h>
#include <parts/CrTreeExtra.h>
#include <crerrors.h>

//必备多线程安全控制，要用到一些多线程的锁
#ifdef CR_WINDOWS
#include <Windows.h>
#elif defined CR_LINUX
#include <pthread.h>
#endif

#ifdef CR_LINUX  //直接使出那一招，简单方便又快捷
void InitializeCriticalSection(pthread_mutex_t* mt)
{
	pthread_mutex_init(mt, NULL);
}
void DeleteCriticalSection(pthread_mutex_t* mt)
{
	pthread_mutex_destroy(mt);
}
void EnterCriticalSection(pthread_mutex_t* mt)
{
	pthread_mutex_lock(mt);
}
void LeaveCriticalSection(pthread_mutex_t* mt)
{
	pthread_mutex_unlock(mt);
}
#endif

/*
* 这个结构和core中的数据头一模一样
* 但是不推荐使用core中的函数来查询容量
* 因为这样会很乱
*/

//说实在的我有一个新想法，可以让extra的数据体由core来控制管理销毁

//共有的数据头
typedef struct
{
#ifdef CR_WINDOWS
	CRITICAL_SECTION cs;  //确保多线程安全
#elif defined CR_LINUX
	pthread_mutex_t cs;  //确保多线程安全
#endif
	CRUINT8 type;  //用于表示据结构类型
	CRUINT32 total;  //用于表示现在有多少个元素
} CRSTRUCTUREPUB;

typedef struct quadtreenode
{
	CRPOINTU point;
	struct quadtreenode* LT;  //left top
	struct quadtreenode* RT;  //right top
	struct quadtreenode* LB;  //left bottom
	struct quadtreenode* RB;  //right bottom
	CRLVOID data;
};

typedef struct
{
	CRSTRUCTUREPUB pub;
}CRQUADINNER, *PCRQUADINNER;

CRTREEXTRA CRQuadtree()
{
	return NULL;
}