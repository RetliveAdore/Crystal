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

/*创建一个二叉区域查找树*/
CRTREEXTRA CRRangeTree();

/*创建一个四叉查找树*/
CRTREEXTRA CRQuadtree();
/*创建一个四叉区域查找树*/
CRTREEXTRA CRRangeQuadtree();

#ifdef __cplusplus
}
#endif

#endif //include
