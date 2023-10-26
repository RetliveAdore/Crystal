#include "Demo.h"
#include <parts/Crbinary.h>
#include <parts/CrDynExtra.h>
#include <stdio.h>
#include <math.h>
char* str1 = "123456";
char* str2 = "123455";
char* str3 = "asdhjgahgoliyfuebuhaogsxutfawEIYFASCIPGEYZHXBb;khigf";
char* str4 = "c";

//展示十六进制数的奥秘所在
char hexChar[] =
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

	for (index = 0; index < strlen(str); index++)
		CRDynSet(dyn, str[index], index);
	CRDynSet(dyn, '\0', index);
	hash = CRHash64(dyn);
	printf("内容：%s\n哈希值：", str);
	DisplayHex(hash);
	printf("信息熵：%.5f\n\n", CREntropy(dyn));
	CRFreeStructure(dyn, NULL);
}

void HashDemo()
{
	CalculateHash(str1);
	CalculateHash(str2);
	CalculateHash(str3);
	CalculateHash(str4);
}

int Demo2(int argc, char** argv)
{
	HashDemo();
	return 0;
}