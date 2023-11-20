/*
* 声音系统添加的似乎比较快，但其实是因为UI系统思路已经成熟，
* 应该优先解决一些未涉及过的问题。
* Windows使用WASAPI，Linux使用ALSA
*/

#ifndef _INCLUDE_CRAUDIO_H_
#define _INCLUDE_CRAUDIO_H_

#include "../datastructure.h"

typedef void (*CRAudioStreamCbk)(CRUINT8* buffer, CRUINT32 frames, CRUINT32 size);
typedef CRLVOID CRAUDIOPLAY;

#ifdef __cplusplus
extern "C" {
#endif

//需要初始化才能正常使用
CRAPI CRCODE CRAudioInit();
CRAPI void CRAudioUnInit();

/*使用一整个buffer来直接播放完整音频*/
CRAPI CRAUDIOPLAY CRAudioBuffer(CRSTRUCTURE dynPcm, CRWWINFO* inf);
/*这个API更加具有应变性，可以用作直播流*/
CRAPI CRAUDIOPLAY CRAudioStream(CRWWINFO* inf, CRAudioStreamCbk);
/*在上面两个API调用成功之后需要用这个来关闭并释放内存*/
CRAPI CRCODE CRAudioClose(CRAUDIOPLAY play);
/*如果想要等待音频播放完毕就使用这个API来释放*/
CRAPI CRCODE CRAudioWait(CRAUDIOPLAY play);
/*查询播放进度【1】：播放完毕 ; 【0】：刚刚开始播放 ; 【-1】：出错了*/
CRAPI double CRAudioCheckProgress(CRAUDIOPLAY play);

CRAPI CRCODE CRAudioPause(CRAUDIOPLAY play);
CRAPI CRCODE CRAudioResume(CRAUDIOPLAY play);

#ifdef __cplusplus
}
#endif

#endif  //include
