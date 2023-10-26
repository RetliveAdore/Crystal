#include <Crystal.h>
#include <parts/Crbinary.h>
#include <parts/CrDynExtra.h>
#include <malloc.h>
#include <math.h>

// （三元组）总共26比特
// +-----------+-------------------------+--------------+-----------------+
// | type( 1 ) | offset in window(12bit) | length(5bit) | next bits(8bit) |
// +-----------+-------------------------+--------------+-----------------+
// （普通字节）总共9比特
// +-----------+-----------------+
// | type( 0 ) | next bits(8bit) |
// +-----------+-----------------+
// 需要很多位操作

//1:三元组
//0:普通字符
#define LZ77_TYPE_BIT_SIZE   1   //类型，表明是普通字符还是三元组的标志位
#define LZ77_OFFSET_BIT_SIZE 12  //偏移量，在编码窗口中的偏移量
#define LZ77_LENGTH_BIT_SIZE 5   //长度，匹配到的最大长度
#define LZ77_NEXT_BIT_SIZE   8   //下一个普通字符的编码

#define LZ77_WINDOW_SIZE     4095  //滑动窗口的大小限制（1111_1111_1111B）
#define LZ77_BUFFER_SIZE     31    //超前缓冲区大小限制（1_1111B）

CRAPI CRCODE CRBinaryInit()
{
	return 0;
}

CRAPI void CRBinaryUninit()
{}

//

/*
* 一些基础的数据转化
*/

CRUINT64 _rotate_bin_(CRUINT64 bin, CRUINT8 offs)
{
	return bin << offs | bin >> (64 - offs);
}
CRAPI CRUINT64 CRHash64(CRSTRUCTURE dyn)
{
	CRUINT64 hash = 0x17865fabdc3e9402;
	CRUINT8 byte = 0;
	CRUINT32 size = CRStructureSize(dyn);
	for (CRUINT32 index = 0; index < size; index++)
	{
		CRDynSeek(dyn, &byte, index);
		hash ^= byte;
		hash = _rotate_bin_(hash, byte % 64);
		hash ^= _rotate_bin_(hash, 7);
	}
	return hash;
}

CRAPI double CREntropy(CRSTRUCTURE dyn)
{
	CRUINT32* hash = calloc(256, sizeof(CRUINT32));
	if (!hash)
		return 0.0f;
	double entropy = 0.0f;
	CRUINT32 size = CRStructureSize(dyn);
	CRUINT8 byte = 0;
	for (int i = 0; i < size; i++)
	{
		CRDynSeek(dyn, &byte, i);
		hash[byte]++;
	}
	for (int j = 0; j < 256; j++)
	{
		if (hash[j])
		{
			double portability = (double)hash[j] / (double)size;
			entropy -= portability * log(portability) * 1.4426950408889634;
		}
	}
	free(hash);
	return entropy;
}

//

/*
* 下方位数据压缩解压实现（LZ77）
*/

CRUINT8 _compare_win_()
{

}

CRAPI CRCODE CRCompress(CRSTRUCTURE dynIn, CRSTRUCTURE* dynOut)
{
	if (!dynOut)
		return CRERR_INVALID;

	return 0;
}

CRAPI CRCODE CRDecompress(CRSTRUCTURE dynIn, CRSTRUCTURE* dynOut)
{
	if (!dynOut)
		return CRERR_INVALID;

	return 0;
}