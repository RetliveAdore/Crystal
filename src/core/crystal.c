#include <Crystal.h>
#include <stdlib.h>
#include <stdio.h>

CRSTRUCTURE bin = NULL;

static const char* errs[] =
{
	"Fine\0",
	"out of memory\0",
	"invalid wipe function\0"
};
static const char* errNow = NULL;

//编译时写死在库中，可以用于验证真正的二进制版本
const CRVERSION ver =
{
	.v16[0] = CRV_MAJOR,
	.v16[1] = CRV_MINOR,
	.v16[2] = CRV_BUILD,
	.v16[3] = CRV_REVIS
};
//版本控制

static void _clear_callback_(CRLVOID data)
{
	((TrashBinFunc)data)();
}
static void _on_close_(void)
{
	CRFreeStructure(bin, _clear_callback_);
	bin = NULL;
}

CRAPI CRCODE CRInit()
{
	errNow = errs[0];
	bin = CRLinear();
	if (!bin)
	{
		return -1;
		errNow = errs[0];
	}
	atexit(_on_close_);
	return 0;
}

CRAPI const CRVERSION* CRVer()
{
	printf("Crystal Version: %d.%d.%d.%d\n", CRV_MAJOR, CRV_MINOR, CRV_BUILD, CRV_REVIS);
	return &ver;
}

CRAPI CRCODE CRAddtoTrashBin(TrashBinFunc WipeYourAss)
{
	if (!bin)
		return CRERR_CORE_UNINIT;
	if (!WipeYourAss)
		return CRERR_CORE_INVALID;
	CRCODE code = 0;
	CRLinPut(bin, WipeYourAss, 0);
	return 0;
}

CRAPI const char* CRErrorCore(CRCODE errcode)
{
	if (!errNow)
		errNow = errs[0];
	const char* ret = errNow;
	if (errcode && errcode < sizeof(errs) / sizeof(const char*))
		ret = errs[errcode];
	else if (!errcode)
		ret = errNow;
	else
		ret = "invalid errcode!";
	errNow = errs[0];
	return ret;
}