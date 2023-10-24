#ifndef _INCLUDE_CRTHERAD_H_
#define _INCLUDE_CRTHERAD_H_

#include "../datastructure.h"

typedef CRLVOID CRTIMER;
typedef CRLVOID CRTHREAD;
typedef CRLVOID CRLOCK;

typedef void (*CRThreadFunction)(CRLVOID userdata, CRTHREAD idThis);

//初始化之后才能正常使用全部功能
CRAPI CRCODE CRBasicInit();
CRAPI void CRBasicUninit();

//

/*
* 和时间相关的函数，包含休眠、计时器等
*/

#ifdef __cplusplus
extern "C" {
#endif

CRAPI void CRSleep(CRUINT64 ms);

CRAPI CRTIMER CRTimer();
CRAPI double CRTimerPeek(CRTIMER timer);
CRAPI double CRTimerMark(CRTIMER timer);
CRAPI CRCODE CRTimerClose(CRTIMER timer);

//

/*
* 和多线程相关的函数，包含创建、等待线程，线程锁之类
*/

CRAPI CRTHREAD CRThread(CRThreadFunction func, CRLVOID userData);
CRAPI CRCODE CRWaitThread(CRTHREAD thread);

CRAPI CRLOCK CRLockCreate();
CRAPI void CRLockRelease(CRLOCK lock);

//是否遵守规则全凭自愿，遇到乱搞以下函数概不负责

CRAPI void CRLock(CRLOCK lock);  //会阻塞直到加锁成功
CRAPI void CRUnlock(CRLOCK lock);  //无论锁的状态，立刻解锁

#ifdef __cplusplus
}
#endif

#endif //include
