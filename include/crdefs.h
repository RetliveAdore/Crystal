#ifndef _INCLUDE_CRDEFS_H_
#define _INCLUDE_CRDEFS_H_

#ifdef WIN32
#  define CR_WINDOWS
#  define CRLD __declspec(dllimport)
#  ifdef CR_SHARED_LIB
#    ifdef CR_BUILD_DLL
#      define CRAPI __declspec(dllexport)
#    else
#      define CRAPI __declspec(dllimport)
#    endif
#  else
#    define CRAPI extern
#  endif

#elif __linux__
#  define CR_LINUX
#  define CRAPI extern
#  define CRLD  extern

#else
#  error unsupported platform.
#endif

#ifdef CR_WINDOWS
#  ifdef CRENTRY_CRT
#    pragma comment(linker,"/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
#  endif
#  pragma comment(linker, "/align:256")
#endif

#ifndef __cplusplus
#  define CRBOOL  _Bool
#  define CRFALSE 0
#  define CRTRUE  1
#  ifndef NULL
#    define NULL (void*)0
#  endif
#else
#  define CRBOOL bool
#  define CRFALSE false
#  define CRTRUE true
#  ifndef NULL
#    define NULL nullptr
#  endif
#endif

#ifdef CR_WINDOWS
typedef unsigned char CRUINT8;
typedef unsigned short CRUINT16;
typedef unsigned int CRUINT32;
typedef unsigned long long CRUINT64;
typedef char CRINT8;
typedef short CRINT16;
typedef int CRINT32;
typedef long CRINT64;

#elif defined CR_LINUX
#ifdef __INT8_TYPE__
typedef __INT8_TYPE__ CRINT8;
#else
typedef char CRINT8;
#endif
#ifdef __INT16_TYPE__
typedef __INT16_TYPE__ CRINT16;
#else
typedef short CRINT16;
#endif
#ifdef __INT32_TYPE__
typedef __INT32_TYPE__ CRINT32;
#else
typedef int CRINT32;
#endif
#ifdef __INT64_TYPE__
typedef __INT64_TYPE__ CRINT64;
#else
typedef long CRINT64
#endif
#ifdef __UINT8_TYPE__
typedef __UINT8_TYPE__ CRUINT8;
#else
typedef unsigned char CRUINT8;
#endif
#ifdef __UINT16_TYPE__
typedef __UINT16_TYPE__ CRUINT16;
#else
typedef unsigned short CRUINT16;
#endif
#ifdef __UINT32_TYPE__
typedef __UINT32_TYPE__ CRUINT32;
#else
typedef unsigned int CRUINT32;
#endif
#ifdef __UINT64_TYPE__
typedef __UINT64_TYPE__ CRUINT64;
#else
typedef unsigned long CRUINT64;
#endif
#endif

typedef void* CRLVOID;
typedef CRINT32 CRCODE;

//

/*
* WAVE格式的文件相关结构体
*/

typedef struct
{
	CRUINT32 ChunkID;
	CRUINT32 ChunkSize;
}CRWWBLOCK;

typedef struct
{
	CRUINT16 AudioFormat;   //PCM固定为1
	CRUINT16 NumChannels;   //声道数量
	CRUINT32 SampleRate;    //采样率，如：44100
	CRUINT32 ByteRate;
	CRUINT16 BlockAlign;
	CRUINT16 BitsPerSample; //采样位宽，如16，24，32
}CRWWINFO;

typedef struct
{
	//
	CRWWBLOCK whole;  //ChunkID is 'RIFF' or it`s wrong file
	CRUINT32 format;  //'WAVE' or wrong data
	//
	CRWWBLOCK block2;  //ChunkID if 'fmt'
	CRWWINFO inf;
	//
	//...data...  //Search a chunk whose id is 'data', then you find the
	//            //actual PCM data
}CRWWHEADER;

//

#endif