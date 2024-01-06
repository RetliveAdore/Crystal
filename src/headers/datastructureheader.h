#ifndef _INCLUDE_CR_DATASTRUCTUREHEADER_H_
#define _INCLUDE_CR_DATASTRUCTUREHEADER_H_

#include <Crystal.h>
#include <malloc.h>
#include <string.h>
#include <crerrors.h>

#ifdef CR_WINDOWS
#include <Windows.h>
#elif defined CR_LINUX
#include <pthread.h>
#endif

#define DYN 0x00
#define TRE 0x01
#define LIN 0x02
#define LOO 0x03  //当前未使用
#define QUA 0x04

//

//容量限制是512MB
#define DYN_MAX (sizeof(CRUINT8) << 29)
#define DYN_MAX_PTR ((sizeof(CRUINT8) << 29) / sizeof(CRLVOID))

#define GET_MOD(num, mod) { if ((num) < 0) (num) = -(-(num) % (mod)); else if ((num > 0)) (num) = ((num) % (mod)); }

#include <stdio.h>
FILE* fp = NULL;

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

//

typedef struct
{
	CRSTRUCTUREPUB pub;
	CRUINT8 feature;  //0-CRUINT8* arr, 1-CRLVOID* ptr
	union
	{
		CRLVOID* ptr;
		CRUINT8* arr;
	};
	CRUINT32 capacity;
}CRDYN, * PCRDYN;

//

typedef struct treenode
{
	CRINT64 key;
	CRLVOID data;
	CRBOOL red;
	struct treenode* parent;
	struct treenode* left;
	struct treenode* right;
}TREENODE, * PTREENODE;

typedef struct
{
	CRSTRUCTUREPUB pub;
	PTREENODE root;
}CRTREE, * PCRTREE;

//

typedef struct linearnode
{
	CRLVOID data;
	CRUINT64 key;  //用来支持排序的
	struct linearnode* prior;
	struct linearnode* after;
}LINEARNODE, * PLINEARNODE;

typedef struct
{
	CRSTRUCTUREPUB pub;
	PLINEARNODE hook;
}CRLINEAR, * PCRLINEAR;

//

typedef struct
{
	CRSTRUCTURE pub;
	CRUINT8* arr;
}CRLOOP, * PCRLOOP;

//

#endif