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

#endif  //include
