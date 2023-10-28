#include "Demo.h"
#include <parts/Crbinary.h>
#include <parts/CrDynExtra.h>
#include <stdio.h>
#include <math.h>

char* str1 = "asdhjgahgoliyfuebuhaogsxutfawEIYFASCIPGEYZHXBb;khigf";
char* str2 = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa123123131231231ljhscfoahwbdocuyzslcgfblaiouerwyllllll";

//展示十六进制数的奥秘所在
char hexChar[16] =
{
	'0', '1', '2', '3',
	'4', '5', '6', '7',
	'8', '9', 'a', 'b',
	'c', 'd', 'e', 'f'
};
void DisplayHex(CRUINT64 hex)
{
	CRUINT8* tmp = (CRUINT8*)&hex;
	for (int i = 0; i < 8; i++)
	{
		putchar(hexChar[tmp[i] >> 4 & 0xf]);
		putchar(hexChar[tmp[i] & 0xf]);
	}
	printf("\n");
}

void CalculateHash(char* str)
{
	CRUINT64 hash;
	CRUINT32 index;
	CRSTRUCTURE dyn = CRDynamic();
	if (!dyn)
	{
		printf("%s\n", CRGetError(0));
		return;
	}

	CRDynSetup(dyn, str, strlen(str) + 1);
	hash = CRHash64(dyn);
	printf("内容：%s\n信息熵：%.5f\n哈希值：", str, CREntropy(dyn));
	DisplayHex(hash);
	printf("\n");
	CRFreeStructure(dyn, NULL);
}

void HashDemo()
{
	CalculateHash(str1);
	CalculateHash(str2);
}

void CompressTest()
{
	CRSTRUCTURE srcDyn = CRDynamic();
	CRSTRUCTURE compressed = CRDynamic();
	CRSTRUCTURE decompressed = CRDynamic();

	//这里strlen加一是因为还需要把结尾的'\0'包含进去
	CRDynSetup(srcDyn, str2, strlen(str2) + 1);
	
	CRCompress(srcDyn, compressed);
	CRDecompress(compressed, decompressed);

	CRUINT8* src = CRDynCopy(srcDyn, NULL);
	printf("压缩前：%s\n信息熵：%.5f\n", src, CREntropy(srcDyn));

	CRUINT8* comp = CRDynCopy(compressed, NULL);
	printf("压缩后：%s\n信息熵：%.5f\n", comp, CREntropy(compressed));

	CRUINT8* copy = CRDynCopy(decompressed, NULL);
	printf("解压后：%s\n信息熵：%.5f\n", copy, CREntropy(decompressed));

	CRDynFreeCopy(src);
	CRDynFreeCopy(comp);
	CRDynFreeCopy(copy);

	CRFreeStructure(srcDyn, NULL);
	CRFreeStructure(compressed, NULL);
	CRFreeStructure(decompressed, NULL);
}

int Demo2(int argc, char** argv)
{
	HashDemo();
	CompressTest();
	return 0;
}