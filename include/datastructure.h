#ifndef _INCLUDE_CRDATASTRUCTURE_H_
#define _INCLUDE_CRDATASTRUCTURE_H_

#include "crdefs.h"

//数据结构就是数字生命基础
typedef CRLVOID CRSTRUCTURE;
typedef CRSTRUCTURE* PCRSTRUCTURE;

/**
当要对结构中所有的元素进行操作时：
例如销毁和遍历，
对于需要手动释放或者有特殊善后方法的数据，
可以传入函数指针来解决
*/
typedef void(*DSCallback)(CRLVOID data);

/**
分别为：创建——动态数组、键值树、线性表、环形数组
需要注意：Dynamic和Loop每个单元存储的都是一字节（8位）
而Tree和Linear每个单元存储的都是一个void指针（通常64位）
*/

CRAPI CRSTRUCTURE CRDynamic();
CRAPI CRSTRUCTURE CRTree();
CRAPI CRSTRUCTURE CRLinear();
CRAPI CRSTRUCTURE CRLoop();

CRAPI CRCODE CRFreeStructure(CRSTRUCTURE s, DSCallback cal);
CRAPI CRUINT32 CRStructureSize(CRSTRUCTURE s);

//

/*
* 动态数组的最大容量其实是有限制的，为了防止某些脑瘫一上来就把物理内存耗尽。
* 如果达到上限，或者分配内存失败，你将获得错误提示
* 动态数组不仅能主动扩增空间，也能主动缩减空间，以增加空间使用效率
*/

CRAPI CRCODE CRDynPush(CRSTRUCTURE dyn, CRUINT8 data);
CRAPI CRCODE CRDynSet(CRSTRUCTURE dyn, CRUINT8 data, CRUINT32 sub);
CRAPI CRCODE CRDynPop(CRSTRUCTURE dyn, CRUINT8* data);
CRAPI CRCODE CRDynSeek(CRSTRUCTURE dyn, CRUINT8* data, CRUINT32 sub);
CRAPI CRUINT8* CRDynCopy(CRSTRUCTURE dyn, CRUINT32* size);

//

/*
* 键值树内部实现方式是红黑树
* seek并不会移除元素
* 当使用Get并成功之后，此元素将从树中移除
*/

CRAPI CRCODE CRTreePut(CRSTRUCTURE tree, CRLVOID data, CRINT64 key);
CRAPI CRCODE CRTreeSeek(CRSTRUCTURE tree, CRLVOID* data, CRINT64 key);
CRAPI CRCODE CRTreeGet(CRSTRUCTURE tree, CRLVOID* data, CRINT64 key);

//

/*
* 线性表由链表实现只需要一些基础操作即可
* 链表是环形双向的
*/

CRAPI CRCODE CRLinPut(CRSTRUCTURE lin, CRLVOID data, CRINT64 seek);
CRAPI CRCODE CRLinSeek(CRSTRUCTURE lin, CRLVOID* data, CRINT64 seek);
CRAPI CRCODE CRLinGet(CRSTRUCTURE lin, CRLVOID* data, CRINT64 seek);

/*
* 这些都是对上述线性表功能的再封装
*/

#define CRQueuePush(lin, data) CRLinPut((lin), (data), -1)
#define CRQueuePop(lin, data) CRLinGet((lin), (data), -1)

#endif