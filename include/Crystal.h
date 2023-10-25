#ifndef _INCLUDE_RYSTAL_H_
#define _INCLUDE_RYSTAL_H_

#include "crdefs.h"
#include "datastructure.h"

/**
实际上这里对于版本编号没有那么严格的要求
只要知道孰新孰旧即可
当然，对于有重大更新的版本编号最好还是改一改MAJOR或者MINOR
内部存储版本号有十六位，数字区间在0~65535之间
*/
typedef union
{
	CRUINT64 v64;
	/*
		v16:
		v16[0]~major
		v16[1]~minor
		v16[2]~build
		v16[3]~revision
	*/
	CRUINT16 v16[4];
}CRVERSION;
#define CRV_MAJOR 0
#define CRV_MINOR 0
#define CRV_BUILD 4   //每次commit都要记得至少把build版本加一
#define CRV_REVIS 0   //这个几乎可以随便写，但是只能变大不准减小

#define CRERR_FINE      0
#define CRERR_UNINITED  1
#define CRERR_INVALID   2
#define CRERR_NOTFOUND  3
#define CRERR_OUTOFMEM  4
#define CRERR_CONFLICT  5
//按顺序从小到大，此宏定义用于指示边界
#define CRERR_MAXCODE   6

//在引擎中不存在的功能可以通过添加第三方模块来添加
typedef CRINT64 CRMODULE;
/*
使用
*/
typedef void (*CRModFunction)(CRLVOID structure);
//服务请求数据格式
typedef struct request_structure
{
	CRUINT8 service_id;//请求的服务
	CRUINT64 id;       //组件的uuid
	const char* name;  //组件名称或者用于返回错误
	CRSTRUCTURE fl;    //函数列表
	CRUINT32 fun_id;   //请求的函数id
	CRModFunction func;//返回的函数
} CRMOD_REQ;
typedef CRMOD_REQ* (*CRMODService)(CRMOD_REQ* request);

/**
擦屁股功能
传递一个函数，告诉Crystal退出之后该怎么做
*/
typedef void (*TrashBinFunc)(void);

#ifdef __cplusplus
extern "C" {
#endif

CRAPI CRCODE CRInit();
CRAPI const CRVERSION* CRVer();

/*
传入的错误代码不要小于1000
假如没有将crystal初始化，将返回CRFALSE
假如传入的错误代码是内置代码，将会使其更新为默认错误
重复投送相同的错误代码，后来者将会覆盖之前的description
*/
CRAPI CRBOOL CRThrowError(CRCODE errcode, const char* desc);
/*
传入0默认查询当前发生的错误
假如查询不到错误或者没有错误，将固定返回Fine，查询之后将移除该错误
*/
CRAPI const char* CRGetError(CRCODE errcode);

//

/**
擦屁股功能
用于主进程退出时的收尾处理，一般用来释放服务和内存
作者很懒，不想知道它什么时候退出，所以希望Crystal能够自己擦屁股
*/

//需要注意的一点是，在某些平台标准C库会提早释放，导致像printf之类的无法使用
//所以说擦屁股函数最好不要依赖于标准库，但是Crystal库里面的函数是可以使用的
CRAPI CRCODE CRAddtoTrashBin(TrashBinFunc WipeYourAss);

#ifdef __cplusplus
}
#endif

#endif //incldue
