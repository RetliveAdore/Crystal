#include "Demo.h"
#include <parts/CrAudio.h>
#include <parts/Crbinary.h>
#include <parts/Crbasic.h>

static char command[128];
static CRUINT8 sub = 0;
static CRSTRUCTURE streamPcm = 0;
static CRUINT32 streamOff = 0;
static CRBOOL end = CRFALSE;

static void ClearCommandD4()
{
	for (int i = 0; i < 128; i++) command[i] = '\0';
}

static void GetCommandD4()
{
	printf("[p/pause;r/resume;q/stop;g/progress]:");
	while (sub < 127)
	{
		command[sub] = getchar();
		if (command[sub] == '\n')
		{
			command[sub] = '\0';
			sub = 0;
			return;
		}
		sub++;
	}
	sub = 0;
}

//标准的，官方的，可靠的，习惯性的...
void Stream(CRUINT8* buffer, CRUINT32 frames, CRUINT32 size)
{
	for (int i = 0; i < size; i++)
		CRDynSeek(streamPcm, (CRUINT8*)&buffer[i], streamOff + i);
	streamOff += size;
	if (streamOff > CRStructureSize(streamPcm))
		end = CRTRUE;
}

static CRBOOL ProcessCommandD4(CRAUDIOPLAY play)
{
	if (!strlen(command))
		return CRTRUE;
	if (Compare(command, "p") || Compare(command, "pause"))
		CRAudioPause(play);
	else if (Compare(command, "r") || Compare(command, "resume"))
		CRAudioResume(play);
	else if (Compare(command, "q") || Compare(command, "stop"))
		return CRFALSE;
	else if (Compare(command, "g") || Compare(command, "progress"))
		printf("progress: %.2f%%\n", CRAudioCheckProgress(play) * 100);
	return CRTRUE;
}

int Demo4(int argc, char** argv)
{
	//init
	CRCODE code = CRAudioInit();
	if (code)
		printf("error: %s\n", CRGetError(code));
	//
	CRWWINFO inf;
	CRSTRUCTURE pcm = CRDynamic();
	code = CRLoadWave("./resource/au.wav", pcm, &inf);
	if (code)
		printf("Error: %s\n", CRGetError(code));

	printf("采样率：%d\n", inf.SampleRate);
	printf("声道数：%d\n", inf.NumChannels);
	printf("位宽：%d\n", inf.BitsPerSample);
	
	CRAUDIOPLAY play = CRAudioBuffer(pcm, &inf);
	if (!play)
		printf("error: %s\n", CRGetError(0));
	while (CRAudioCheckProgress(play) < 1.0f)
	{
		ClearCommandD4();
		GetCommandD4();
		if (!ProcessCommandD4(play))
			break;
	}
	//一定记得手动释放内存，虽然uninit时会自动释放，但是建议养成好习惯
	CRAudioClose(play);

	code = CRLoadWave("./resource/rick.wav", pcm, &inf);
	if (code)
		printf("Error: %s\n", CRGetError(code));
	printf("采样率：%d\n", inf.SampleRate);
	printf("声道数：%d\n", inf.NumChannels);
	printf("位宽：%d\n", inf.BitsPerSample);
	
	streamPcm = pcm;
	streamOff = 0;
	end = CRFALSE;
	play = CRAudioStream(&inf, Stream);
	if (!play)
		printf("error: %s\n", CRGetError(0));
	while (!end)
	{
		ClearCommandD4();
		GetCommandD4();
		if (!ProcessCommandD4(play))
			break;
	}
	CRAudioClose(play);

	CRFreeStructure(pcm, NULL);
	//uninit
	CRAudioUnInit();
	//
	return 0;
}