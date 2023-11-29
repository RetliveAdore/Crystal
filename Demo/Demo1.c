#include "Demo.h"
#include <parts/Crbasic.h>
#include <stdio.h>
#include <malloc.h>

#define DEMO_PORT 24600
#define CHECK_ERROR(code) if (code) printf("error: %s\n", CRGetError(code))

char cmd[128];
CRUINT8 sub = 0;
CRINET g_inet = 0;
CRBOOL stop = CRFALSE;

/*
server port
client port address
*/
char* splits[3];

 /*将字符串转化为数字 */
static CRUINT16 eval(const char* strNum)
{
 	CRUINT8 len = strlen(strNum) - 1;
 	if (len > 5)
 		return 0;
 	CRUINT16 base = 1;
 	CRUINT16 ret = 0;
 	for (; len > 0; len--)
 		base *= 10;
 	int i = 0;
 	while (base >= 1)
 	{
 		ret += (strNum[i] - '0') * base;
 		if (strNum[i] < '0' || strNum[i] > '9')
			return 0;
 		base /= 10;
		i++;
 	}
 	return ret;
}

void process(CRINET inet)
{
	printf("connected\n");
	
	CRCODE len = 0;
	while (1)
	{
		len = CRInetRcv(inet, 127, cmd);
		if (len == 0)
			goto End;
		else if (len > 0)
		{
			cmd[len] = '\0';
			if (Compare(cmd, "shutdown"))
				goto Stop;
			printf("recived: %s\n", cmd);
			CRInetSnd(inet, 127, cmd);
		}
		else
		{
			CRSleep(1);
			continue;
		}
	}
Stop:
	stop = CRTRUE;
End:
	printf("disconnected\n");
}

int server(CRUINT16 port)
{
	stop = CRFALSE;
	printf("port: %d\n", port);
	g_inet = CRServerInet(process, DEMO_PORT);
	if (!g_inet)
		printf("error: %s\n", CRGetError(0));

	while (!stop) CRSleep(1);
	return 0;
}

void _client_rcv_thread_(CRLVOID data, CRTHREAD idThis)
{
	char mcmd[128];
	CRUINT8 msub = 0;
	CRCODE code = 0;
	CRBOOL* stop = data;
	while (!*stop)
	{
		code = CRInetRcv(g_inet, 127, mcmd);
		if (code == 0)
			return;
		else if (code < 0)
			continue;
		else
		{
			mcmd[code] = '\0';
			printf("recived: %s\n", cmd);
		}
	}
}

int client(const char* addr, CRUINT16 port)
{
	
	g_inet = CRClientInet(addr, DEMO_PORT, 5);
	if (!g_inet)
		printf("error: %s\n", CRGetError(0));

	CRBOOL stop = CRFALSE;
	CRTHREAD thr = CRThread(_client_rcv_thread_, &stop);
	while (1)
	{
		while (sub < 127)
		{
			cmd[sub] = getchar();
			if (cmd[sub] == '\n')
			{
				cmd[sub] = '\0';
				sub = 0;
				break;
			}
			sub++;
		}
		if (Compare(cmd, "quit"))
			break;
		else
			CRInetSnd(g_inet, strlen(cmd), cmd);
	}
	stop = CRTRUE;
	CRWaitThread(thr);
	return 0;
}

static void GetCmdStr()
{
	printf("[server/client port <address>]:");
	while (sub < 127)
	{
		cmd[sub] = getchar();
		if (cmd[sub] == '\n')
		{
			cmd[sub] = '\0';
			sub = 0;
			break;
		}
		sub++;
	}
	//split
	CRUINT8 curr = 0;
	CRUINT8 len = 0;
	CRUINT8 now = 0;
	char* pos = cmd;
	//
	splits[0] = NULL;
	splits[1] = NULL;
	splits[2] = NULL;
	//
	while (cmd[curr] != ' ' && now < strlen(cmd)) curr++, len++, now++;
	if (len <= 0)
		goto Clear;
	curr++;
	splits[0] = malloc(len + 1);
	if (!splits[0])
		goto Clear;
	memcpy(splits[0], pos, len);
	splits[0][len] = '\0';
	pos += len + 1;
	//
	len = 0;
	while (cmd[curr] != ' ' && now < strlen(cmd)) curr++, len++, now++;
	if (len <= 0)
		goto Clear;
	curr++;
	splits[1] = malloc(len + 1);
	if (!splits[1])
		goto Clear;
	memcpy(splits[1], pos, len);
	splits[1][len] = '\0';
	pos += len + 1;
	//
	len = 0;
	while (cmd[curr] != ' ' && now < strlen(cmd)) curr++, len++, now++;
	if (len <= 0)
		goto Process;
	curr++;
	splits[2] = malloc(len + 1);
	if (!splits[2])
		goto Clear;
	memcpy(splits[2], pos, len);
	splits[2][len] = '\0';
Process:
	//分割完成，开始解析并执行
	if (Compare(splits[0], "server"))
		server(eval(splits[1]));
	else if (Compare(splits[0], "client"))
	{
		if (!splits[2])
		{
			printf("error: no IP given!\n");
			goto Clear;
		}
		client(splits[2], eval(splits[1]));
	}
	else printf("type error->%s\n", splits[0]);
Clear:
	//清理内存
	if (splits[0])
		free(splits[0]);
	if (splits[1])
		free(splits[1]);
	if (splits[2])
		free(splits[2]);
}

int Demo1(int argc, char** argv)
{
	CRCODE code =
	CRBasicInit();
	CHECK_ERROR(code);

	GetCmdStr();

	code = CRCloseInet(g_inet);
	CHECK_ERROR(code);
Failed:
	CRBasicUninit();
	return 0;
}