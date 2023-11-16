#ifndef _INCLUDE_CRBINARY_H_
#define _INCLUDE_CRBINARY_H_

#include "../datastructure.h"

/*
* 方晶引擎应当具备现代数据的处理能力
* 比如说：
* 压缩解压缩（LZ77）
* 哈希运算（自用的，64位）
* 快速傅里叶变换
* 文件空间映射（统一接口）
* ...
*/

//初始化之后才能正常使用全部功能
CRAPI CRCODE CRBinaryInit();
CRAPI void CRBinaryUninit();

#ifdef __cplusplus
extern "C" {
#endif

//传入动态数组来计算其包含的数据的哈希值，返回方晶引擎独有的64位摘要
CRAPI CRUINT64 CRHash64(CRSTRUCTURE dyn);
//用于计算信息熵
CRAPI double CREntropy(CRSTRUCTURE dyn);

CRAPI CRCODE CRCompress(CRSTRUCTURE dynIn, CRSTRUCTURE dynOut);
CRAPI CRCODE CRDecompress(CRSTRUCTURE dynIn, CRSTRUCTURE dynOut);

//用于加载PCM流数据，通常与CRAudio配合使用
CRAPI CRCODE CRLoadWave(const char* path, CRSTRUCTURE out, CRWWINFO* inf);

#ifdef __cplusplus
}
#endif

#endif  //inclued
