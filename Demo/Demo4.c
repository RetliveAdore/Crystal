#include "Demo.h"
#include <parts/CrAudio.h>
#include <parts/Crbinary.h>
#include <parts/Crbasic.h>

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
	
	code = CRAudioPlay(pcm, &inf);
	if (code)
		printf("error: %s\n", CRGetError(code));

	CRFreeStructure(pcm, NULL);

	//uninit
	CRAudioUnInit();
	//
	return 0;
}