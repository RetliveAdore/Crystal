#include "Demo.h"
#include <parts/Crbasic.h>
#include <stdio.h>

#define DEMO_PORT 24600
#define CHECK_ERROR(code) if (code) printf("error: %s\n", CRGetError(code))

CRBOOL connected = CRFALSE;
char* cmd[128];
CRUINT8 sub = 0;
CRINET inet = 0;

static void GetCmdStr()
{
	printf("[type port <address>]:\n");
	while (sub < 127)
	{
		cmd[sub++] = getchar();
		if (cmd[sub] == '\n')
		{
			cmd[sub] = '\0';
			sub = 0;
			break;
		}
	}
	//split
	CRUINT8 head = 0;
	CRUINT8 len = 0;
	for (int i = 0; i < strlen(cmd); i++)
	{
	}
}

/*将字符串转化为数字 */
static CRUINT16 eval(const char* strNum)
{
	CRUINT8 len = strlen(strNum);
	if (len > 5)
		return 0;
	CRUINT16 base = 1;
	CRUINT16 ret = 0;
	for (; len > 0; len--)
		base *= 10;
	int i = 0;
	while (base >= 1)
	{
		ret += (strNum[i++] - '0') * base;
		if (strNum[i] < '0' || strNum[i] > '9')
			return 0;
		base /= 10;
	}
	return ret;
}

void process(CRINET inet)
{
	printf("connected\n");
	connected = CRTRUE;
}

int server(CRUINT16 port)
{
	CRCODE code = 0;
	inet = CRServerInet(process, DEMO_PORT);
	if (!server)
		printf("error: %s\n", CRGetError(0));

	return 0;
}

int client(const char* addr, CRUINT16 port)
{
	CRCODE code = 0;
	inet = CRClientInet("127.0.0.1", DEMO_PORT, 5);
	if (!client)
		printf("error: %s\n", CRGetError(0));

	return 0;
}

int Demo1(int argc, char** argv)
{
	CRCODE code =
	CRBasicInit();
	CHECK_ERROR(code);
	char symbol = getchar();
	if (symbol == 's')
	{
		printf("server\n");
		server(DEMO_PORT);
	}
	else if (symbol == 'c')
	{
		printf("client\n");
		client("127.0.0.1", DEMO_PORT);
		connected = CRTRUE;
	}
	else goto Failed;
	while (!connected)
		CRSleep(1);
	code = CRCloseInet(inet);
	CHECK_ERROR(code);
Failed:
	CRBasicUninit();
	return 0;
}