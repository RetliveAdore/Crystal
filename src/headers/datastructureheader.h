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
#define LOO 0x03  //��ǰδʹ��
#define QUA 0x04

//

//����������512MB
#define DYN_MAX (sizeof(CRUINT8) << 29)
#define DYN_MAX_PTR ((sizeof(CRUINT8) << 29) / sizeof(CRLVOID))

#define GET_MOD(num, mod) { if ((num) < 0) (num) = -(-(num) % (mod)); else if ((num > 0)) (num) = ((num) % (mod)); }

#include <stdio.h>
FILE* fp = NULL;

//���е�����ͷ
typedef struct
{
#ifdef CR_WINDOWS
	CRITICAL_SECTION cs;  //ȷ�����̰߳�ȫ
#elif defined CR_LINUX
	pthread_mutex_t cs;  //ȷ�����̰߳�ȫ
#endif
	CRUINT8 type;  //���ڱ�ʾ�ݽṹ����
	CRUINT32 total;  //���ڱ�ʾ�����ж��ٸ�Ԫ��
} CRSTRUCTUREPUB;

#ifdef CR_LINUX  //ֱ��ʹ����һ�У��򵥷����ֿ��
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
	CRUINT64 key;  //����֧�������
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