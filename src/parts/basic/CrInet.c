#include <Crystal.h>
#include <parts/Crbasic.h>
#include <malloc.h>
#include <crerrors.h>

#define SERVER_MAX_ACCEPT 20

#ifdef CR_WINDOWS
#include <WinSock2.h>
#elif defined CR_LINUX
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include <fcntl.h>
#define closesocket(soc) close(soc)
#endif

extern CRUINT64 CurrentIDsocket;
extern CRSTRUCTURE socketTree;
extern CRSTRUCTURE availableIDsocket;

#define CR_INET_CLIENT 0x00
#define CR_INET_SERVER 0x01

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

CRAPI CRINET CRServerInet(CRUINT16 port)
{
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
		goto Failed;
	}
	SOCKADDR_IN addr_in;
	addr_in.sin_addr.S_un.S_addr = htoml(ADDR_ANY);
	addr_in.sin_family = AF_INET;
	addr_in.sin_port = htons(port);
	//绑定，然后监听端口
	if (bind(pInner->soc, (SOCKADDR*)&addr_in, sizeof(SOCKADDR_IN)) < 0)
	{
		CRThrowError(CRERR_BASIC_INETBIND, CRDES_BASIC_INETBIND);
		goto Failed;
	}
Failed:
	if (pInner)
	{
		if (pInner->soc != INVALID_SOCKET)
			closesocket(pInner->soc);
		free(pInner);
	}
	return 0;
}

CRAPI CRINET CRClientInet()
{
	return 0;
}

CRAPI CRCODE CRCloseInet(CRINET inet)
{
	PCRINETINNER pInner = NULL;
	CRTreeSeek(socketTree, pInner, inet);
	if (!pInner)
		return CRERR_INVALID;
	if (pInner->type == CR_INET_SERVER)
		pInner->alive = CRFALSE;
	else if (pInner->type == CR_INET_CLIENT)
	{
		closesocket(pInner->soc);
		CRTreeGet(socketTree, NULL, inet);
		CRLinPut(availableIDsocket, inet, 0);
	}
	else return CRERR_INVALID;
	return 0;
}