#include <Crystal.h>
#include <parts/Crbinary.h>
#include <parts/CrDynExtra.h>
#include <crerrors.h>
#include <malloc.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

CRLD CRUINT8* CRDynArr(CRSTRUCTURE dyn, CRUINT32* size);

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

CRBOOL _compare_chars_(const char* chars1, const char* chars2, CRUINT32 len)
{
	for (int i = 0; i < len; i++)
		if (chars1[i] != chars2[i]) return CRFALSE;
	return CRTRUE;
}

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

//功能函数，在滑动窗口寻找最长匹配串，并返回长度
//传入token用以返回匹配串的偏移量
CRUINT8 _compare_win_(
	const CRUINT8* winBegin, const CRUINT8* winEnd,
	const CRUINT8* bufBegin, const CRUINT8* bufEnd,
	CRUINT16* token
)
{
	if (winBegin > winEnd || bufBegin > bufEnd)
		return 0;
	CRUINT16 tokenTmp = winEnd - winBegin;
	CRUINT8 longest = 0, length = 0;
	const CRUINT8* pTmpSource = winBegin;
	const CRUINT8* pTmpBuffer = bufBegin;
	while (winBegin <= winEnd)
	{
		while (pTmpBuffer <= bufEnd && pTmpSource <= winEnd)
		{
			if (*pTmpBuffer != *pTmpSource)
				break;
			length++;
			pTmpSource++;
			pTmpBuffer++;
		}
		//准备下一轮
		winBegin++;
		pTmpSource = winBegin;
		pTmpBuffer = bufBegin;
		if (length > longest)
		{
			longest = length;
			*token = tokenTmp + 1;
		}
		tokenTmp--;
		length = 0;
	}
	return longest;
}

CRAPI CRCODE CRCompress(CRSTRUCTURE dynIn, CRSTRUCTURE dynOut)
{
	if (!dynIn || !dynOut)
		return CRERR_INVALID;
	CRUINT32 size = 0;
	CRUINT8* buffer = CRDynArr(dynIn, &size);
	CRUINT8* begin = buffer;  //数据头
	CRUINT8* end = buffer + size - 1;  //数据尾
	CRUINT8* windowPos = begin;  //滑动窗口的位置
	CRUINT64 offset = 0;  //比特位偏移位置

	CRUINT16 token = 0;
	CRUINT8 length = 0;
	//压缩的第一个字符必然是直接编码的
	CRDynSetBits(dynOut, offset, LZ77_TYPE_BIT_SIZE, 0);
	offset += LZ77_TYPE_BIT_SIZE;
	CRDynSetBits(dynOut, offset, LZ77_NEXT_BIT_SIZE, *windowPos);
	offset += LZ77_NEXT_BIT_SIZE;
	//开始循环
	while (windowPos < end)
	{
		length = _compare_win_(
			(windowPos - begin) > LZ77_WINDOW_SIZE ? windowPos - LZ77_WINDOW_SIZE : begin,
			windowPos,
			windowPos + 1,
			(end - windowPos) > LZ77_BUFFER_SIZE ? windowPos + LZ77_BUFFER_SIZE : end,
			&token
		);
		windowPos++;
		if (length > 1)  //三元组
		{
			windowPos += length;
			CRDynSetBits(dynOut, offset, LZ77_TYPE_BIT_SIZE, 1);
			offset += LZ77_TYPE_BIT_SIZE;
			CRDynSetBits(dynOut, offset, LZ77_OFFSET_BIT_SIZE, token);
			offset += LZ77_OFFSET_BIT_SIZE;
			CRDynSetBits(dynOut, offset, LZ77_LENGTH_BIT_SIZE, length);
			offset += LZ77_LENGTH_BIT_SIZE;
			if (windowPos <= end)
				CRDynSetBits(dynOut, offset, LZ77_NEXT_BIT_SIZE, *windowPos);
			else
				CRDynSetBits(dynOut, offset, LZ77_NEXT_BIT_SIZE, 0);
			offset += LZ77_NEXT_BIT_SIZE;
		}
		else
		{
			CRDynSetBits(dynOut, offset, LZ77_TYPE_BIT_SIZE, 0);
			offset += LZ77_TYPE_BIT_SIZE;
			CRDynSetBits(dynOut, offset, LZ77_NEXT_BIT_SIZE, *windowPos);
			offset += LZ77_NEXT_BIT_SIZE;
		}
	}
	return 0;
}

CRAPI CRCODE CRDecompress(CRSTRUCTURE dynIn, CRSTRUCTURE dynOut)
{
	if (!dynIn || !dynOut)
		return CRERR_INVALID;
	CRUINT32 size = CRStructureSize(dynIn);
	CRUINT64 offset = 0;
	CRUINT8 bufferToken = 0;
	CRUINT16 length = 0, token = 0;
	CRUINT64 windowPos = 0;
	CRUINT8 byte = 0;
	while (offset >> 3 < size - 1)
	{
		if (CRDynGetBits(dynIn, offset, LZ77_TYPE_BIT_SIZE))  //坏了，遇到三元组了
		{
			offset += LZ77_TYPE_BIT_SIZE;
			token = CRDynGetBits(dynIn, offset, LZ77_OFFSET_BIT_SIZE);
			offset += LZ77_OFFSET_BIT_SIZE;
			length = CRDynGetBits(dynIn, offset, LZ77_LENGTH_BIT_SIZE);
			offset += LZ77_LENGTH_BIT_SIZE;
			windowPos = CRStructureSize(dynOut) - token;
			while (length)
			{
				CRDynSeek(dynOut, &byte, windowPos);
				CRDynPush(dynOut, byte);
				length--;
				windowPos++;
			}
			CRDynPush(dynOut, CRDynGetBits(dynIn, offset, LZ77_NEXT_BIT_SIZE));
		}
		else
		{
			offset += LZ77_TYPE_BIT_SIZE;
			CRDynPush(dynOut, CRDynGetBits(dynIn, offset, LZ77_NEXT_BIT_SIZE));
		}
		offset += LZ77_NEXT_BIT_SIZE;
	}
	return 0;
}

CRAPI CRCODE CRLoadWave(const char* path, CRSTRUCTURE out, CRWWINFO* inf)
{
	if (!inf)
		return CRERR_INVALID;
	FILE* fp = fopen(path, "rb");
	if (!fp)
		goto PathError;
	CRWWHEADER header;
	CRWWBLOCK block;
	fread(&header, sizeof(CRWWHEADER), 1, fp);
	//假如不符合说明文件出错了
	if (!_compare_chars_((CRUINT8*)&header.whole.ChunkID, "RIFF", 4)) goto FileError;
	if (!_compare_chars_((CRUINT8*)&header.format, "WAVE", 4)) goto FileError;
	//
	fread(&block, sizeof(block), 1, fp);
	while (!_compare_chars_((CRUINT8*)&block.ChunkID, "data", 4))
	{
		fseek(fp, block.ChunkSize, SEEK_CUR);
		if (!fread(&block, sizeof(block), 1, fp))
			break;
	}
	if (!_compare_chars_((CRUINT8*)&block.ChunkID, "data", 4))
		goto FileError;
	//现在是正常加载情况
	//
	CRUINT8* buffer = malloc(block.ChunkSize);
	if (!buffer)
	{
		fclose(fp);
		return CRERR_OUTOFMEM;
	}
	if (fread(buffer,1 ,block.ChunkSize, fp) != block.ChunkSize) //然后就遇到不正常的情况了
	{
		free(buffer);
		goto FileError;
	}
	CRDynSetup(out, buffer, block.ChunkSize);
	free(buffer);
	//
	fclose(fp);
	memcpy(inf, &header.inf, sizeof(CRWWINFO));
	return 0;
PathError:
	CRThrowError(CRERR_BINARY_INVALIDPATH, CRDES_BINARY_INVALIDPATH);
	return CRERR_BINARY_INVALIDPATH;
FileError:
	CRThrowError(CRERR_BINARY_BROKENFILE, CRDES_BINARY_BROKENFILE);
	fclose(fp);
	return CRERR_BINARY_BROKENFILE;
}