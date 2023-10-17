#include <Crystal.h>
#include <iostream>
#include <cstdlib>

void CRVersionOnExit()
{
	CRVer();
	return;
}

int main(int argc, char** argv)
{
	CRCODE code = 0;
	if (code = CRInit() < 0)
	{
		printf("init error: %s\n", CRErrorCore(0));
		return 1;
	}
	if (code = CRAddtoTrashBin(CRVersionOnExit) != 0)
		printf("Add error: %s\n", CRErrorCore(code));
	return 0;
}