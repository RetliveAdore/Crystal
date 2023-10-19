#ifndef _INCLUDE_CRTHERAD_H_
#define _INCLUDE_CRTHERAD_H_

#include "../datastructure.h"

typedef CRLVOID CRTIMER;
typedef CRLVOID CRTHREAD;

//初始化之后才能正常使用全部功能
CRAPI CRCODE CRBasicInit();

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
* 和多线程相关的函数，包含创建、分离、等待线程，线程锁之类
*/

#ifdef __cplusplus
}
#endif

#endif //include
