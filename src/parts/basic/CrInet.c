#include "basicheader.h"
#include <stdio.h>
extern CRUINT64 CurrentIDsocket;
extern CRSTRUCTURE socketTree;
extern CRSTRUCTURE availableIDsocket;
extern CRBOOL crInitedBasic;

#define CR_INET_CLIENT 0x01
#define CR_INET_SERVER 0x02

typedef struct
{
#ifdef CR_WINDOWS
	SOCKET soc;
#elif defined CR_LINUX
	int socket;
#endif
	CRINET id;
	CRUINT8 type;
	CRBOOL alive;
}CRINETINNER, *PCRINETINNER;

struct serverProcessInf
{
	PCRINETINNER pInner;
	CRInetFunction process;
};

//第三棒
void _server_process_(CRLVOID data, CRTHREAD idThis)
{
	struct serverProcessInf* pInf = data;
	CRINET id = 0;
	CRLinGet(availableIDsocket, &id, 0);
	if (!id)
		id = CurrentIDsocket++;
	CRTreePut(socketTree, pInf->pInner, id);
	//
	pInf->process(id);
	//
	CRTreeGet(socketTree, NULL, id);
	CRLinPut(availableIDsocket, id, 0);
	//
	closesocket(pInf->pInner->soc);
	free(pInf->pInner);
	free(pInf);
}

//第二棒
void _server_thread_(CRLVOID* data, CRTHREAD idThis)
{
	struct serverProcessInf* pInf = data;
	fd_set rdfs;
	fd_set rdfs_bk;
	FD_ZERO(&rdfs);
	FD_ZERO(&rdfs_bk);
	FD_SET(pInf->pInner->soc, &rdfs_bk);

	struct timeval tv;
	int iRet = 0;
	while (pInf->pInner->alive)
	{
		rdfs = rdfs_bk;
		tv.tv_sec = 0;
		tv.tv_usec = 1000;
		iRet = select(pInf->pInner->soc + 1, &rdfs, NULL, NULL, &tv);
		if (iRet == -1)
			pInf->pInner->alive = CRFALSE;
		else if (iRet > 0)
		{
			struct serverProcessInf* pNext = malloc(sizeof(struct serverProcessInf));
			if (!pNext)
				continue;
			pNext->pInner = malloc(sizeof(CRINETINNER));
			if (!pNext->pInner)
			{
				free(pNext);
				continue;
			}
			pNext->process = pInf->process;
			pNext->pInner->type = 0;
			pNext->pInner->soc = accept(pInf->pInner->soc, NULL, NULL);
			CRThread(_server_process_, pNext);
		}
	}
	closesocket(pInf->pInner->soc);
	CRTreeGet(socketTree, NULL, pInf->pInner->id);
	CRLinPut(availableIDsocket, pInf->pInner->id, 0);
	//上一棒交接的内存，需要释放（close并不会释放type为server的内存
	free(pInf->pInner);
	free(pInf);
}

//第一棒
CRAPI CRINET CRServerInet(CRInetFunction func, CRUINT16 port)
{
	if (!crInitedBasic)
	{
		CRThrowError(CRERR_UNINITED, NULL);
		goto Failed;
	}
	if (!func)
	{
		CRThrowError(CRERR_INVALID, NULL);
		goto Failed;
	}
	PCRINETINNER pInner = malloc(sizeof(CRINETINNER));
	if (!pInner)
		goto Failed;
	pInner->type = CR_INET_SERVER;
	pInner->alive = CRTRUE;
	pInner->soc = socket(AF_INET, SOCK_STREAM, 0);
	if (pInner->soc == INVALID_SOCKET)
	{
		CRThrowError(CRERR_BASIC_INETCREATE, CRDES_BASIC_INETCREATE);
		goto Failed;
	}
	//使用非阻塞式的
	int imod = 1;
	if (ioctlsocket(pInner->soc, FIONBIO, (u_long*)&imod) == SOCKET_ERROR)
	{
		CRThrowError(CRERR_BASIC_INETIOCTL, CRDES_BASIC_INETIOCTL);
		goto Failed;
	}
	SOCKADDR_IN addr_in;
	addr_in.sin_addr.S_un.S_addr = htonl(ADDR_ANY);
	addr_in.sin_family = AF_INET;
	addr_in.sin_port = htons(port);
	//绑定，然后监听端口
	if (bind(pInner->soc, (SOCKADDR*)&addr_in, sizeof(SOCKADDR_IN)) < 0)
	{
		CRThrowError(CRERR_BASIC_INETBIND, CRDES_BASIC_INETBIND);
		goto Failed;
	}
	if (listen(pInner->soc, SERVER_MAX_ACCEPT) < 0)
	{
		CRThrowError(CRERR_BASIC_LISTENING, CRDES_BASIC_LISTENING);
		goto Failed;
	}
	//然后剩下的工作就是开启监听线程并返回inetID
	struct serverProcessInf* pInf = malloc(sizeof(struct serverProcessInf));
	if (!pInf)
	{
		CRThrowError(CRERR_OUTOFMEM, NULL);
		goto Failed;
	}
	pInf->pInner = pInner;
	pInf->process = func;
	CRThread(_server_thread_, pInf);
	CRINET id = 0;
	CRLinGet(availableIDsocket, &id, 0);
	if (!id)
		id = CurrentIDsocket++;
	pInner->id = id;
	CRTreePut(socketTree, pInner, id);
	return id;
Failed:
	if (pInner)
	{
		if (pInner->soc != INVALID_SOCKET)
			closesocket(pInner->soc);
		free(pInner);
	}
	return 0;
}

CRAPI CRINET CRClientInet(const char* address, CRUINT16 port, CRUINT16 timeoutSecond)
{
	if (!crInitedBasic)
	{
		CRThrowError(CRERR_UNINITED, NULL);
		goto Failed;
	}
	if (!address)
	{
		CRThrowError(CRERR_INVALID, NULL);
		goto Failed;
	}
	CRINET id = 0;
	PCRINETINNER pInner = malloc(sizeof(CRINETINNER));
	if (!pInner)
		goto Failed;
	pInner->type = CR_INET_CLIENT;
	pInner->alive = CRTRUE;
	pInner->soc = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (pInner->soc == INVALID_SOCKET)
	{
		CRThrowError(CRERR_BASIC_INETCREATE, CRDES_BASIC_INETCREATE);
		goto Failed;
	}
	int imod = 1;
	if (ioctlsocket(pInner->soc, FIONBIO, (u_long*)&imod) == SOCKET_ERROR)
	{
		CRThrowError(CRERR_BASIC_INETIOCTL, CRDES_BASIC_INETIOCTL);
		goto Failed;
	}
	SOCKADDR_IN addr_in;
	addr_in.sin_addr.S_un.S_addr = inet_addr(address);
	addr_in.sin_family = AF_INET;
	addr_in.sin_port = htons(port);
	int iRet = connect(pInner->soc, (struct sockaddr*)&addr_in, sizeof(struct sockaddr_in));
	if (iRet == 0)
		goto Succeed;
	else if (iRet < 0)
	{
		iRet = WSAGetLastError();
		if (iRet == WSAEWOULDBLOCK || iRet == WSAEINVAL)
		{
			fd_set wtfds;
			struct timeval tv;
			FD_ZERO(&wtfds);
			FD_SET(pInner->soc, &wtfds);
			tv.tv_sec = timeoutSecond;
			tv.tv_usec = 0;
			iRet = select(0, NULL, &wtfds, NULL, &tv);
			if (iRet < 0)  //出错（嗯~~已经……已经坏掉了……
				goto Error;
			else if (iRet == 0)  //超时了（不要~~不要停下来~~！
			{
				CRThrowError(CRERR_BASIC_TIMEOUT, CRDES_BASIC_TIMEOUT);
				goto Failed;
			}
			else if (FD_ISSET(pInner->soc, &wtfds))
				goto Succeed;
		}
		else if (iRet == WSAEISCONN)
			goto Succeed;
	}
Error:
	CRThrowError(CRERR_BASIC_CONNECT, CRDES_BASIC_CONNECT);
	goto Failed;
Succeed:
	CRLinGet(availableIDsocket, &id, 0);
	if (!id)
		id = CurrentIDsocket++;
	CRTreePut(socketTree, pInner, id);
	pInner->id = id;
	return id;
Failed:
	if (pInner)
	{
		if (pInner->soc != INVALID_SOCKET)
			closesocket(pInner->soc);
		free(pInner);
	}
	return 0;
}

CRAPI CRCODE CRCloseInet(CRINET inet)
{
	PCRINETINNER pInner = NULL;
	CRTreeGet(socketTree, &pInner, inet);
	if (!pInner)
		return CRERR_INVALID;
	if (pInner->type == CR_INET_SERVER)
		pInner->alive = CRFALSE;
	else if (pInner->type == CR_INET_CLIENT)
	{
		closesocket(pInner->soc);
		free(pInner);
		CRLinPut(availableIDsocket, inet, 0);
	}
	else return CRERR_INVALID;
	return 0;
}

void _inet_clear_callback_(CRLVOID* data)
{
	PCRINETINNER pInner = data;
	closesocket(pInner->soc);
	if (pInner->type == CR_INET_CLIENT)
		free(pInner);
}