/*
* 无论是不是游戏引擎，各种高级树型数据结构都是大量运用的。
* 不想以后难受的话就从现在开始考虑“基建”吧
* 2023-11-23-Adore
*/

#ifndef _INCLUDE_CRTREEEXTRA_H_
#define _INCLUDE_CRTREEEXTRA_H_

//实际上这里的树都是相对独立的体系，不需要用到原本的头文件
#include "../crdefs.h"

typedef CRLVOID CRTREEXTRA;

#ifdef __cplusplus
extern "C" {
#endif

/*
* Crystal中四叉树不用作一般数据存储，仅用于空间索引，不进行平衡处理
* 这个数据结构是特化用于碰撞检测和UI快速查找的，不宜用于其他用途
*/

/*创建一个四叉查找树*/
CRAPI CRTREEXTRA CRQuadtree();

CRAPI CRCODE CRQuadtreePut();
CRAPI CRCODE CRQuadtreeGet();
CRAPI CRCODE CRQuadtreeNear();

CRAPI CRCODE CRFreeTreextra(CRTREEXTRA tree);

#ifdef __cplusplus
}
#endif

#endif //include
