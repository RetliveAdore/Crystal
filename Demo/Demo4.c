#include "Demo.h"
#include <parts/CrAudio.h>
#include <parts/Crbinary.h>
#include <parts/Crbasic.h>

int Demo4(int argc, char** argv)
{
	//init

	//
	CRWWINFO inf;
	CRSTRUCTURE pcm = CRDynamic();
	CRCODE code = CRLoadWave("./resource/au.wav", pcm, &inf);
	if (code)
		printf("Error: %s\n", CRGetError(code));

	printf("采样率：%d\n", inf.SampleRate);
	printf("声道数：%d\n", inf.NumChannels);
	printf("位宽：%d\n", inf.BitsPerSample);

	CRFreeStructure(pcm, NULL);

	//uninit

	//
	return 0;
}