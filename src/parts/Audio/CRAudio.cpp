#include <Crystal.h>
#include <parts/Crbasic.h>
#include <parts/CrAudio.h>
#include <crerrors.h>
#include <malloc.h>
#include <stdio.h>

//播放循环周期0.1秒
#define PREFTIMES_PER_SEC 1000000

#ifdef CR_WINDOWS
#include <Windows.h>
#include <audiopolicy.h>
#include <Audioclient.h>
#include <mmdeviceapi.h>

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
const IID IID_IAudioClient = __uuidof(IAudioClient);
const IID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);

IMMDeviceEnumerator* pEnumerator = NULL;

#elif defined CR_LINUX
#define ALSA_PCM_NEW_HW_PARAMS_API
#include <alsa/asoundlib.h>
#endif

CRAPI CRCODE CRAudioInit()
{
#ifdef CR_WINDOWS
	HRESULT hr;
	hr = CoInitializeEx(NULL, COINIT_SPEED_OVER_MEMORY | COINIT_MULTITHREADED);
	if (FAILED(CoCreateInstance(CLSID_MMDeviceEnumerator,
		NULL, CLSCTX_ALL, IID_IMMDeviceEnumerator,
		(void**)&pEnumerator)))
	{
		CRThrowError(CRERR_AUDIO_FAILEDCOM, CRDES_AUDIO_FAILEDCOM);
		return CRERR_AUDIO_FAILEDCOM;
	}
#elif defined CR_LINUX
#endif
	return 0;
}

CRAPI void CRAudioUnInit()
{
#ifdef CR_WINDOWS
	if (pEnumerator)
		pEnumerator->Release();
	CoUninitialize();
#elif defined CR_LINUX
#endif
}

void _fill_buffer_(CRUINT8* Dst, CRSTRUCTURE Src, CRUINT32 frameCount, CRWWINFO* inf, CRUINT32* offs)
{
	CRUINT32 size = frameCount * inf->BlockAlign;
	for (int i = 0; i < size; i++)
		CRDynSeek(Src, (CRUINT8*)&Dst[i], *offs + i);
	*offs += size;
}

#ifdef CR_WINDOWS

//再稍微改一改，由另外一个线程负责buffer就大功告成了
CRAPI CRCODE CRAudioPlay(CRSTRUCTURE dynPcm, CRWWINFO* inf)
{
	if (!pEnumerator)
		return CRERR_UNINITED;
	if (!inf)
		return CRERR_INVALID;
	IMMDevice* pDevice = NULL;
	IAudioClient* pAudioClient = NULL;
	IAudioRenderClient* pRenderClient = NULL;
	WAVEFORMATEX* pwfx;
	CRUINT32 hnsActualDuration;

	CRUINT32 bufferFrameCount;
	CRUINT32 numFramesAvailable;
	CRUINT32 numFramesPadding;
	CRUINT32 offs = 0;
	BYTE* pData = nullptr;
	//
	
	if (FAILED(pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice)))
		goto Failed;
	if (FAILED(pDevice->Activate(IID_IAudioClient, CLSCTX_ALL, NULL, (void**)&pAudioClient)))
		goto Failed;
	if (FAILED(pAudioClient->GetMixFormat(&pwfx)))
		goto Failed;
	//设置音频流数据信息
	pwfx->wFormatTag = inf->AudioFormat;
	pwfx->nChannels = inf->NumChannels;
	pwfx->nSamplesPerSec = inf->SampleRate;
	pwfx->nAvgBytesPerSec = inf->ByteRate;
	pwfx->nBlockAlign = inf->BlockAlign;
	pwfx->wBitsPerSample = inf->BitsPerSample;
	pwfx->cbSize = 0;
	//
	if (FAILED(pAudioClient->Initialize(
		AUDCLNT_SHAREMODE_SHARED,
		AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM | AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY,
		PREFTIMES_PER_SEC,
		0,
		pwfx,
		NULL
	)))
		goto Failed;
	if (FAILED(pAudioClient->GetBufferSize(&bufferFrameCount)))
		goto Failed;
	if (FAILED(pAudioClient->GetService(IID_IAudioRenderClient, (void**)&pRenderClient)))
		goto Failed;
	//
	if (FAILED(pRenderClient->GetBuffer(bufferFrameCount, &pData)))
		goto Failed;
	_fill_buffer_(pData, dynPcm, bufferFrameCount, inf, &offs);
	if (FAILED(pRenderClient->ReleaseBuffer(bufferFrameCount, 0)))
		goto Failed;
	//
	hnsActualDuration = bufferFrameCount * 1000 / inf->SampleRate;
	if (FAILED(pAudioClient->Start()))
		goto Failed;
	while (offs < CRStructureSize(dynPcm))
	{
		//休眠一半时间
		CRSleep(hnsActualDuration / 2);
		//查询剩余空闲空间
		if (FAILED(pAudioClient->GetCurrentPadding(&numFramesPadding)))
			goto Failed;
		numFramesAvailable = bufferFrameCount - numFramesPadding;
		HRESULT hr = 0;
		if (FAILED(hr = pRenderClient->GetBuffer(numFramesAvailable, &pData)))
			goto Failed;
		_fill_buffer_(pData, dynPcm, numFramesAvailable, inf, &offs);
		if (FAILED(pRenderClient->ReleaseBuffer(numFramesAvailable, 0)))
			goto Failed;
	}
	CRSleep(hnsActualDuration / 2);
	pAudioClient->Stop();
	//
End:
	pDevice->Release();
	pAudioClient->Release();
	pRenderClient->Release();
	CoTaskMemFree(pwfx);
	return 0;
Failed:
	if (pDevice)
		pDevice->Release();
	if (pAudioClient)
		pAudioClient->Release();
	if (pRenderClient)
		pRenderClient->Release();
	if (pwfx)
		CoTaskMemFree(pwfx);
	CRThrowError(CRERR_AUDIO_FAILEDCOM, CRDES_AUDIO_FAILEDCOM);
	return CRERR_AUDIO_FAILEDCOM;
}

#elif defined CR_LINUX

//反复试验之后，发现，原生的24位音频数据是不支持的，会直接变成噪音（旋律的话能出来一点点）。。。
//试过16位之后恍然大悟，明显就是溢出了。。。
//这个是我遇到过的最操蛋的bug
CRAPI CRCODE CRAudioPlay(CRSTRUCTURE dynPcm, CRWWINFO* inf)
{
	if (!inf || inf->BitsPerSample > 16)
		return CRERR_INVALID;
	CRUINT32 offs = 0;
	CRINT32 rc;
	CRINT32 dir = 0;
	CRUINT8* tbuffer;
	snd_pcm_uframes_t frames;
	snd_pcm_uframes_t periodsize;
	snd_pcm_t* handle = nullptr;
	snd_pcm_hw_params_t* params = nullptr;
	CRUINT32 val = 0;

	//获取设备句柄
	if (snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0) < 0)
		goto Failed;
	snd_pcm_hw_params_malloc(&params);
	snd_pcm_hw_params_any(handle, params);
	snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
	//
	if (inf->BitsPerSample == 8)
		snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_U8);
	else
		snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE);
	//
	snd_pcm_hw_params_set_channels(handle, params, inf->NumChannels);
	val = inf->SampleRate;
	snd_pcm_hw_params_set_rate_near(handle, params, &val, &dir);
	periodsize = 1024;
	snd_pcm_hw_params_set_period_size(handle, params, periodsize, 0);
	frames = periodsize << 1;
	snd_pcm_hw_params_set_buffer_size(handle, params, frames);
	if (snd_pcm_hw_params(handle, params) < 0)
		goto Failed;
	//
	tbuffer = new CRUINT8[frames * inf->BlockAlign];
	if (!tbuffer)
	{
		snd_pcm_close(handle);
		return CRERR_OUTOFMEM;
	}
	//
	while (offs < CRStructureSize(dynPcm))
	{
		_fill_buffer_(tbuffer, dynPcm, frames, inf, &offs);
		while (rc = snd_pcm_writei(handle, tbuffer, frames) < 0)
		{
			if (rc == -EPIPE)
				snd_pcm_prepare(handle);
		}
	}
	//
	snd_pcm_drain(handle);
	snd_pcm_close(handle);
	snd_pcm_hw_params_free(params);
	delete[] tbuffer;
	return 0;
Failed:
	if (handle)
		snd_pcm_close(handle);
	if (params)
		snd_pcm_hw_params_free(params);
	CRThrowError(CRERR_AUDIO_FAILEDALSA, CRDES_AUDIO_FAILEDALSA);
	return CRERR_AUDIO_FAILEDALSA;
}
#endif
