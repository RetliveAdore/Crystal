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

CRTREEXTRA CRQuadtree()
{
	return NULL;
}