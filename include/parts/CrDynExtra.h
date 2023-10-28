#ifndef _INCLUDE_CRDYNEXTRA_H_
#define _INCLUDE_CRDYNEXTRA_H_

#include "../datastructure.h"

//1000_0000
#define CR_BIT_MASK_1 0x80
//0111_1111
#define CR_BIT_MASK_0 0x7f
#define CR_BIT_COVER  0xff

/*
* 亚字节操作，可以在一段缓存中以比特位单位，任意偏移量，在64位长度范围内
* 进行获取或者设置比特数据
*/

/* 如何进行操作的说明：
GetBits:
{
offset->|<--len-->|
0101...0101_0101_0101...  //dyn

              |<--len-->|
0000_0000..0001_0101_0101 //back
|<--------64bit-------->|
}

SetBits:
{
offset->|<--len-->|
0000...0101_0101_0100...  //dyn

              |<--len-->|
0000_0000..0001_0101_0101 //bits
|<--------64bit-------->|
}
*/

//最长操作64位比特

CRAPI CRUINT64 CRDynGetBits(CRSTRUCTURE dyn, CRUINT64 offset, CRUINT8 len);
CRAPI CRCODE CRDynSetBits(CRSTRUCTURE dyn, CRUINT64 offset, CRUINT8 len, CRUINT64 bits);

#endif  //include
