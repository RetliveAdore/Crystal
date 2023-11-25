#ifndef _INCLUDE_CRTHERAD_H_
#define _INCLUDE_CRTHERAD_H_

#include "../datastructure.h"

typedef CRLVOID CRTIMER;
typedef CRLVOID CRTHREAD;
typedef CRLVOID CRLOCK;
typedef CRLVOID CRINET;  //之所以把网络部分放到这里，是因为现在网络太重要了，足以集成到核心

typedef void (*CRThreadFunction)(CRLVOID userdata, CRTHREAD idThis);
/*此回调函数用于设置接收到数据之后的处理动作*/
typedef void (*CRInetFunction)(CRINET inet);

#ifdef __cplusplus
extern "C" {
#endif

//初始化之后才能正常使用全部功能
CRAPI CRCODE CRBasicInit();
CRAPI void CRBasicUninit();

//

/*
* 和时间相关的函数，包含休眠、计时器等
*/

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

//

/*
* 和文件操作相关的函数，将其映射到内存，封包解包等
* 文件映射到内存之后使用动态数组的接口来操作，
* 操作部分的实现将会在Cryatal数据结构核心之中
*/

//

/*
* 现代人没人不上网吧？
* 网络集成必须有
*/

CRAPI CRINET CRServerInet(CRInetFunction func, CRUINT16 port);
CRAPI CRINET CRClientInet(const char* address, CRUINT16 port, CRUINT16 timeoutSecond);
CRAPI CRCODE CRCloseInet(CRINET inet);

#ifdef __cplusplus
}
#endif

#endif //include
