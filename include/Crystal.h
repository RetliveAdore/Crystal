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
#define CRV_BUILD 0  //每次commit都要记得至少把build版本加一
#define CRV_REVIS 4  //这个几乎可以随便写，但是只能变大不准减小

#define CRERR_CORE_FINE    0
#define CRERR_CORE_UNINIT  1
#define CRERR_CORE_INVALID 2

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

//

/**
擦屁股功能
用于主进程退出时的收尾处理，一般用来释放服务和内存
作者很懒，不想知道它什么时候退出，所以希望Crystal能够自己擦屁股
*/

//需要注意的一点是，在某些平台标准C库会提早释放，导致像printf之类的无法使用
//所以说擦屁股函数最好不要依赖于标准库，但是Crystal库里面的函数是可以使用的
CRAPI CRCODE CRAddtoTrashBin(TrashBinFunc WipeYourAss);

CRAPI const char* CRErrorCore(CRCODE errcode);

#ifdef __cplusplus
}
#endif

#endif //incldue
