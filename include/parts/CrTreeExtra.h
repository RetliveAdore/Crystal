/*
* 无论是不是游戏引擎，各种高级树型数据结构都是大量运用的。
* 不想以后难受的话就从现在开始考虑“基建”吧
* 2023-11-23-Adore
*/

#ifndef _INCLUDE_CRTREEEXTRA_H_
#define _INCLUDE_CRTREEEXTRA_H_

#include "../crdefs.h"
#include "../datastructure.h"

typedef CRLVOID CRTREEXTRA;

#ifdef __cplusplus
extern "C" {
#endif

/*
* Crystal中四叉树不用作一般数据存储，仅用于空间索引，不进行平衡处理
* 这个数据结构是特化用于碰撞检测和UI快速查找的，不宜用于其他用途
*/

/*创建一个四叉查找树
* 与二叉树不同的是，这种树用来分割空间，必须有一个初始的空间范围
* 以此为根范围，以有用户设置为准，通常为窗口大小
* 但是窗口大小经常变化的话，建议使用一个固定的值，然后计算映射。因为不提供更改范围值的接口（也没设计对应的实现）
* -s-
* w / h: w 和 h 是必要的
* max: 当同一个节点中实体数量超过max之后，节点分裂，创建后此值不可修改
* -s-
* 要注意这个树是不平衡的，和红黑树不一样，而且在同一个区域插入太多元素会使其逐渐线性退化（重叠性能退化）
*/
CRAPI CRTREEXTRA CRQuadtree(CRUINT64 w, CRUINT64 h, CRUINT8 max);

CRAPI CRCODE CRQuadtreePushin(CRTREEXTRA tree, CRRECTU range, CRLVOID key);
CRAPI CRCODE CRQuadtreeRemove(CRTREEXTRA tree, CRLVOID key);
//选用动态数组作为输出承载是因为可能遇到元素重叠的情况，这时需要将其全部输出
CRAPI CRCODE CRQuadtreeSearch(CRTREEXTRA tree, CRPOINTU p, CRSTRUCTURE dynPtrOut);

CRAPI CRCODE CRFreeTreextra(CRTREEXTRA pTree, DSCallback cbk);

#ifdef __cplusplus
}
#endif

#endif //include
