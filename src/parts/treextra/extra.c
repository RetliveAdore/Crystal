#include <Crystal.h>
#include <parts/CrTreeExtra.h>
#include <crerrors.h>

//�ر����̰߳�ȫ���ƣ�Ҫ�õ�һЩ���̵߳���
#ifdef CR_WINDOWS
#include <Windows.h>
#elif defined CR_LINUX
#include <pthread.h>
#endif

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

/*
* ����ṹ��core�е�����ͷһģһ��
* ���ǲ��Ƽ�ʹ��core�еĺ�������ѯ����
* ��Ϊ���������
*/

//˵ʵ�ڵ�����һ�����뷨��������extra����������core�����ƹ�������

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