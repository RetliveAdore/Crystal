#include "Demo.h"

void CRVersionOnExit()
{
	CRVer();
	return;
}

int main(int argc, char** argv)
{
	CRCODE code = 0;
	if (code = CRInit() < 0)
		return 1;
	if (code = CRAddtoTrashBin(CRVersionOnExit) != 0)
		printf("Add error: %s\n", CRGetError(code));
	
	CRUINT8 sub = 0;
	while (1)
	{
		ClearCommand();
		GetCommand();
		if (!ProcessCommand(argc, argv))
			break;
	}
	return 0;
}