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

#ifdef __cplusplus
extern "C" {
#endif

/**
分别为：创建——动态数组、键值树、线性表、环形数组
需要注意：Dynamic和Loop每个单元存储的都是一字节（8位）
而Tree和Linear每个单元存储的都是一个void指针（通常64位）
*/

CRAPI CRSTRUCTURE CRDynamic();
CRAPI CRSTRUCTURE CRDynamicPtr();  //此函数创建的动态数组以指针（通常64位/8字节）为基本单元
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
//上下两种不可混用，否则会报INVALID错误
CRAPI CRCODE CRDynPushPtr(CRSTRUCTURE dyn, CRLVOID data);
CRAPI CRCODE CRDynSetPtr(CRSTRUCTURE dyn, CRLVOID data, CRUINT32 sub);
CRAPI CRCODE CRDynPopPtr(CRSTRUCTURE dyn, CRLVOID* data);
CRAPI CRCODE CRDynSeekPtr(CRSTRUCTURE dyn, CRLVOID* data, CRUINT32 sub);
//下面两个函数可以混用，本质上不构成区别
CRAPI void* CRDynCopy(CRSTRUCTURE dyn, CRUINT32* size);
CRAPI void CRDynFreeCopy(void* data);

//使用已有的数据立即初始化动态数组
//假如buffer是NULL，就清空数组并重新初始化
//要注意，size是字节数而非下标大小，通常CRLVOID一个下标就有四字节
//同样可以混用，但是在指针存储模式下size必须是单个指针字节数的整数倍
//否则将返回INVALID
CRAPI CRCODE CRDynSetup(CRSTRUCTURE dyn, void* buffer, CRUINT32 size);

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

#ifdef __cplusplus
}
#endif

/*
* 这些都是对上述线性表功能的再封装
*/

#define CRQueuePush(lin, data) CRLinPut((lin), (data), -1)
#define CRQueuePop(lin, data) CRLinGet((lin), (data), -1)

#endif