/*
* 声音系统添加的似乎比较快，但其实是因为UI系统思路已经成熟，
* 应该优先解决一些未涉及过的问题。
* Windows使用WASAPI，Linux使用ALSA
*/

#ifndef _INCLUDE_CRAUDIO_H_
#define _INCLUDE_CRAUDIO_H_

#include "../datastructure.h"

#ifdef __cplusplus
extern "C" {
#endif

//需要初始化才能正常使用
CRAPI CRCODE CRAudioInit();
CRAPI void CRAudioUnInit();

CRAPI CRCODE CRAudioPlay(CRSTRUCTURE dynPcm, CRWWINFO* inf);

#ifdef __cplusplus
}
#endif

#endif  //include
