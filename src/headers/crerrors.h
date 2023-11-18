#ifndef _INCLUDE_CRERRORS_H_
#define _INCLUDE_CRERRORS_H_

/*
* 此处用于记录Crystal官方内部的所有扩展错误代码，包含Code和Description部分
* 所有错误代码均大于1000（有符号32位整数）（CRCODE）
* 编码由小到大依次排列，先到先得
*/
#define CRERR_STRUCTURE_RESIZE 1001
#define CRDES_STRUCTURE_RESIZE "failed to resize -> realloc() <-, in crystal structure"

#define CRERR_STRUCTURE_FULL 1002
#define CRDES_STRUCTURE_FULL "failed to extend dynamic array's space, size limited to 512MB"

#define CRERR_BINARY_DYN_CREATE 1003
#define CRDES_BINARY_DYN_CREATE "failed to create a dynamic array"

#define CRERR_WINDOW_REGISTERCLASS 1004
#define CRDES_WINDOW_REGISTERCLASS "error ->RegistwndClassEx<- platform Windows"

#define CRERR_WINDOW_OPENDISPLAY 1005
#define CRDES_WINDOW_OPENDISPLAY "error ->XOpenDisplay<- platform Linux"

#define CRERR_WINDOW_CRCREATEPOOL 1006
#define CRDES_WINDOW_CRCREATEPOOL "failed to create window Pool"

#define CRERR_WINDOW_CRCREATEIDPOOL 1007
#define CRDES_WINDOW_CRCREATEIDPOOL "failed to create ID pool"

#define CRERR_BINARY_INVALIDPATH 1008
#define CRDES_BINARY_INVALIDPATH "failed to open the file in this path!"

#define CRERR_BINARY_BROKENFILE 1009
#define CRDES_BINARY_BROKENFILE "data in file broken, could not read correctly"

#define CRERR_AUDIO_FAILEDCOM 1010
#define CRDES_AUDIO_FAILEDCOM "error COM platform Windows"

#define CRERR_AUDIO_FAILEDALSA 1011
#define CRDES_AUDIO_FAILEDALSA "error ALSA platform Linux"

#endif  //include
