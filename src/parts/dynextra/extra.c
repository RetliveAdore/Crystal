/*
* 因为动态数组十分具有潜力，此处专门在原有的基础上拓展一些扩展功能，
* 比如说：
* 亚字节操作，
* 文件映射内存，
* ...
*/

#include <Crystal.h>
#include <parts/CrDynExtra.h>
#include <crerrors.h>
#include <datastructureheader.h>

#ifdef CR_WINDOWS
#include <Windows.h>
#elif defined CR_LINUX
#include <pthread.h>
#endif

CRLD CRCODE CRDynSizeUp(CRSTRUCTURE dyn, CRUINT32 size, CRUINT32 capacity);
CRLD CRCODE CRDynSizeDown(CRSTRUCTURE dyn, CRUINT32 capacity);

//

CRAPI CRUINT64 CRDynGetBits(CRSTRUCTURE dyn, CRUINT64 offset, CRUINT8 len)
{
	if (len > 63)  //从0开始
		len = 63;
	PCRDYN pInner = dyn;
	if (!pInner || pInner->pub.type != DYN)
	{
		CRThrowError(CRERR_INVALID, NULL);
		return 0;
	}
	EnterCriticalSection(&(pInner->pub.cs));
	CRUINT64 endpos = (offset + len) >> 3;
	endpos += (offset + len) % 8 > 0 ? 1 : 0;
	if (endpos >= pInner->pub.total)
	{
		CRThrowError(CRERR_NOTFOUND, NULL);
		LeaveCriticalSection(&(pInner->pub.cs));
		return 0;
	}
	CRUINT64 back = 0;
	CRUINT32 size = pInner->pub.total;
	CRUINT8* buffer = pInner->arr; //寻址到需要进行读取的那个字节
	CRUINT8 offs = offset % 8;  //偏移量
	buffer += offset >> 3;
	for (; len > 0; len--)
	{
		back |= (((*buffer & CR_BIT_MASK_1 >> offs) >> (7 - offs)) << (len - 1));
		offs++;
		if (offs > 7)
		{
			offs = 0;
			buffer++;
		}
	}
	LeaveCriticalSection(&(pInner->pub.cs));
	return back;
}

CRAPI CRCODE CRDynSetBits(CRSTRUCTURE dyn, CRUINT64 offset, CRUINT8 len, CRUINT64 bits)
{
	if (len > 63)  //从0开始
		len = 63;
	PCRDYN pInner = dyn;
	if (!pInner || pInner->pub.type != DYN)
		return CRERR_INVALID;
	CRUINT64 endpos = (offset + len) >> 3;
	endpos += (offset + len) % 8 > 0 ? 1 : 0;
	if (endpos >= DYN_MAX)
	{
		CRThrowError(CRERR_STRUCTURE_FULL, CRDES_STRUCTURE_FULL);
		return CRERR_STRUCTURE_FULL;
	}
	EnterCriticalSection(&(pInner->pub.cs));
	if (endpos >= pInner->capacity && endpos < DYN_MAX)
	{
		while (pInner->capacity <= endpos) pInner->capacity <<= 1;
		CRCODE code = CRDynSizeUp(dyn, pInner->pub.total, pInner->capacity);
		if (code)
		{
			LeaveCriticalSection(&(pInner->pub.cs));
			return code;
		}
		pInner->pub.total = endpos + 1;
	}
	else if (endpos >= pInner->pub.total)
		pInner->pub.total = endpos + 1;
	CRUINT8* buffer = pInner->arr;
	CRUINT8 offs = offset % 8;
	buffer += offset >> 3;
	for (; len > 0; len--)
	{
		*buffer &= CR_BIT_MASK_0 >> offs | CR_BIT_COVER << (8 - offs);
		*buffer |= (bits >> (len - 1) & 1) << (7 - offs);
		offs++;
		if (offs > 7)
		{
			offs = 0;
			buffer++;
		}
	}
	LeaveCriticalSection(&(pInner->pub.cs));
	return 0;
}
