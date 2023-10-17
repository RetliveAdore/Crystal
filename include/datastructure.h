#ifndef _INCLUDE_CRDATASTRUCTURE_H_
#define _INCLUDE_CRDATASTRUCTURE_H_

#include "crdefs.h"

/*
错误代码：
*/

#define CRERR_STRUC_FINE     0
#define CRERR_STRUC_OUTOFMEM 1
#define CRERR_STRUC_INVALID  2
#define CRERR_STRUC_RESIZE   3
#define CRERR_STRUC_EMPTY    4
#define CRERR_STRUC_NOTFOND  5

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
传入错误代码查询错误，传入0则默认查询最近出现的错误
对于那种不返回CRCODE但有可能出错的函数，传入0来查询错误
在查询动作之后，默认错误会置零，等待下次发生错误以供查询
*/
CRAPI const char* CRErrorStructure(CRCODE errcode);

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

CRAPI CRCODE CRTreePut(CRSTRUCTURE tree, CRLVOID data, CRUINT64 id);
CRAPI CRCODE CRTreeSeek(CRSTRUCTURE tree, CRLVOID* data, CRUINT64 id);
CRAPI CRCODE CRTreeGet(CRSTRUCTURE tree, CRLVOID* data, CRUINT64 id);

//

/*
* 线性表由链表实现只需要一些基础操作即可
* 链表是环形双向的
*/

CRAPI CRCODE CRLinPut(CRSTRUCTURE lin, CRLVOID data, CRINT64 seek);
CRAPI CRCODE CRLinSeek(CRSTRUCTURE lin, CRLVOID* data, CRINT64 seek);
CRAPI CRCODE CRLinGet(CRSTRUCTURE lin, CRLVOID* data, CRINT64 seek);

#endif