#include <Crystal.h>
#include <parts/Crbasic.h>
#include <parts/CrAudio.h>

#ifdef CR_WINDOWS
#include <Windows.h>
#include <audiopolicy.h>
#include <Audioclient.h>
#include <mmdeviceapi.h>

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
const IID IID_IAudioClient = __uuidof(IAudioClient);
const IID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);

#elif defined CR_LINUX

#endif

CRAPI CRCODE CRAudioInit()
{
	return 0;
}

CRAPI void CRAudioUnInit()
{

}