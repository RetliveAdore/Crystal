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

CRTREEXTRA CRQuadtree()
{
	return NULL;
}