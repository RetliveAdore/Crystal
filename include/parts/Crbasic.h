#ifndef _INCLUDE_CRTHERAD_H_
#define _INCLUDE_CRTHERAD_H_

#include "../datastructure.h"

typedef CRLVOID CRTIMER;
typedef CRLVOID CRTHREAD;

//初始化之后才能正常使用全部功能
CRAPI CRCODE CRBasicInit();
CRAPI const char* CRErrorBasic(CRCODE errcode);

#define CRERR_BAS_FIME     0
#define CRERR_BAS_UNINIT   1
#define CRERR_BAS_OUTOFMEM 2
#define CRERR_BAS_INVALID  3

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

#ifdef __cplusplus
}
#endif

#endif //include
