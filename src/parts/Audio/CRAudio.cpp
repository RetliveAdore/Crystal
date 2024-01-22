#include <Crystal.h>
#include <parts/Crbasic.h>
#include <parts/CrAudio.h>
#include <crerrors.h>
#include <malloc.h>
#include <stdio.h>

//播放循环周期0.01秒
#define PREFTIMES_PER_SEC 100000
#define CR_AUDIOMAGIC (CRUINT8)0x57

//这种管理模式是为了提高安全性采用的
static CRSTRUCTURE audioPool = nullptr;
static CRSTRUCTURE availableID = nullptr;
static CRUINT64 currentID = 1;
static CRBOOL inited = CRFALSE;

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

typedef struct
{
	CRUINT8 magic = CR_AUDIOMAGIC;
	IMMDevice* pDevice = nullptr;
	IAudioClient* pAudioClient = nullptr;
	IAudioRenderClient* pRenderClient = nullptr;
	CRWWINFO* inf = nullptr;
	CRSTRUCTURE dynPcm = nullptr;
	/*专用于数据流模式，其他模式中被忽略*/
	CRAudioStreamCbk cbk = nullptr;

	CRUINT32 hnsActualDuration = 0;
	CRUINT32 bufferFrameCount = 0;
	CRUINT32 numFramesAvailable = 0;
	CRUINT32 numFramesPadding = 0;
	CRUINT32 offs = 0;

	CRTHREAD idThis = 0;
	//symbols
	CRBOOL stop = CRFALSE;
	CRBOOL pause = CRFALSE;

	CRBOOL stream;
}AUTHRINF;

#elif defined CR_LINUX
#define ALSA_PCM_NEW_HW_PARAMS_API
#include <alsa/asoundlib.h>

typedef struct
{
	CRUINT8 magic = CR_AUDIOMAGIC;
	snd_pcm_t* handle = nullptr;
	snd_pcm_uframes_t frames = 0;
	/*专用于数据流模式，其他模式中被忽略*/
	CRAudioStreamCbk cbk = nullptr;

	CRWWINFO* inf = nullptr;
	CRSTRUCTURE dynPcm = nullptr;
	CRUINT32 offs = 0;

	CRTHREAD idThis = 0;
	//symbols
	CRBOOL stop = CRFALSE;
	CRBOOL pause = CRFALSE;
	int alsa_can_pause = 0;

	CRBOOL stream;
}AUTHRINF;

#endif

CRAPI CRCODE CRAudioInit()
{
	if (inited)
		return 0;
	CRCODE code = CRBasicInit();
	if (code)
		return code;
	currentID = 1;
	audioPool = CRTree();
	if (!audioPool)
		return CRERR_OUTOFMEM;
	availableID = CRLinear();
	if (!availableID)
	{
		CRFreeStructure(audioPool, NULL);
		return CRERR_OUTOFMEM;
	}
	inited = CRTRUE;
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

static void _clear_callback_(void* data)
{
	AUTHRINF* pInner = (AUTHRINF*)data;
	pInner->stop = CRTRUE;
	CRWaitThread(pInner->idThis);
}

CRAPI void CRAudioUnInit()
{
	CRFreeStructure(audioPool, _clear_callback_);
	CRFreeStructure(availableID, NULL);
	inited = CRFALSE;
	audioPool = nullptr;
	availableID = nullptr;
	//
	CRBasicUninit();
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
	if (*offs > CRStructureSize(Src))
		*offs = CRStructureSize(Src);
}

CRAUDIOPLAY _generate_id_(AUTHRINF* thinf)
{
	CRAUDIOPLAY id;
	CRLinGet(availableID, &id, 0);
	if (!id)
		id = (CRAUDIOPLAY)currentID++;
	CRTreePut(audioPool, thinf, (CRINT64)id);
	return id;
}

#ifdef CR_WINDOWS

void _audio_thread_(void* data, CRTHREAD idThis)
{
	AUTHRINF* auinf = (AUTHRINF*)data;
	CRUINT8* pData = nullptr;
	CRBOOL paused = CRFALSE;
	while (auinf->offs < CRStructureSize(auinf->dynPcm) && !auinf->stop)
	{
		//休眠一半时间
		CRSleep(auinf->hnsActualDuration / 2);
		if (!paused)
		{
			if (auinf->pause)
			{
				auinf->pAudioClient->Stop();
				paused = CRTRUE;
			}
			else
			{
				//查询剩余空闲空间
				if (FAILED(auinf->pAudioClient->GetCurrentPadding(&auinf->numFramesPadding)))
					return;
				auinf->numFramesAvailable = auinf->bufferFrameCount - auinf->numFramesPadding;
				HRESULT hr = 0;
				if (FAILED(hr = auinf->pRenderClient->GetBuffer(auinf->numFramesAvailable, &pData)))
					return;
				_fill_buffer_(pData, auinf->dynPcm, auinf->numFramesAvailable, auinf->inf, &auinf->offs);
				if (FAILED(auinf->pRenderClient->ReleaseBuffer(auinf->numFramesAvailable, 0)))
					return;
			}
		}
		else
		{
			if (!auinf->pause)
			{
				auinf->pAudioClient->Start();
				paused = CRFALSE;
			}
		}
	}
	CRSleep(auinf->hnsActualDuration / 2);
	auinf->pAudioClient->Stop();
	auinf->pDevice->Release();
	auinf->pAudioClient->Release();
	auinf->pRenderClient->Release();
}

//实际上原本的音频方案就是按照动态流设计的，
//此处仅需要将“装填”部分向外开放即可
void _audio_stream_thread_(void* data, CRTHREAD idThis)
{
	AUTHRINF* auinf = (AUTHRINF*)data;
	CRUINT8* pData = nullptr;
	CRBOOL paused = CRTRUE;
	while (!auinf->stop)
	{
		//休眠一半时间
		CRSleep(auinf->hnsActualDuration / 2);
		if (!paused)
		{
			if (auinf->pause)
			{
				auinf->pAudioClient->Stop();
				paused = CRTRUE;
			}
			else
			{
				//查询剩余空闲空间
				if (FAILED(auinf->pAudioClient->GetCurrentPadding(&auinf->numFramesPadding)))
					return;
				auinf->numFramesAvailable = auinf->bufferFrameCount - auinf->numFramesPadding;
				HRESULT hr = 0;
				if (FAILED(hr = auinf->pRenderClient->GetBuffer(auinf->numFramesAvailable, &pData)))
					return;
				auinf->cbk(pData, auinf->numFramesAvailable, auinf->numFramesAvailable * auinf->inf->BlockAlign);
				if (FAILED(auinf->pRenderClient->ReleaseBuffer(auinf->numFramesAvailable, 0)))
					return;
			}
		}
		else
		{
			if (!auinf->pause)
			{
				auinf->pAudioClient->Start();
				paused = CRFALSE;
			}
		}
	}
	CRSleep(auinf->hnsActualDuration / 2);
	auinf->pAudioClient->Stop();
	auinf->pDevice->Release();
	auinf->pAudioClient->Release();
	auinf->pRenderClient->Release();
}

static inline CRBOOL _create_device_(AUTHRINF* thinf, CRWWINFO* inf)
{
	WAVEFORMATEX* pwfx = nullptr;
	if (FAILED(pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &thinf->pDevice)))
		goto Failed;
	if (FAILED(thinf->pDevice->Activate(IID_IAudioClient, CLSCTX_ALL, NULL, (void**)&thinf->pAudioClient)))
		goto Failed;
	if (FAILED(thinf->pAudioClient->GetMixFormat(&pwfx)))
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
	if (FAILED(thinf->pAudioClient->Initialize(
		AUDCLNT_SHAREMODE_SHARED,
		AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM | AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY,
		PREFTIMES_PER_SEC,
		0,
		pwfx,
		NULL
	)))
		goto Failed;
	if (FAILED(thinf->pAudioClient->GetBufferSize(&thinf->bufferFrameCount)))
		goto Failed;
	if (FAILED(thinf->pAudioClient->GetService(IID_IAudioRenderClient, (void**)&thinf->pRenderClient)))
		goto Failed;
	//
	
	CoTaskMemFree(pwfx);
	return CRTRUE;
Failed:
	if (pwfx)
		CoTaskMemFree(pwfx);
	return CRFALSE;
}

//再稍微改一改，由另外一个线程负责buffer就大功告成了
//已经大功告成了
CRAPI CRAUDIOPLAY CRAudioBuffer(CRSTRUCTURE dynPcm, CRWWINFO* inf)
{
	if (!inited)
	{
		CRThrowError(CRERR_UNINITED, NULL);
		return nullptr;
	}
	if (!inf)
	{
		CRThrowError(CRERR_INVALID, NULL);
		return nullptr;
	}
	BYTE* pData = nullptr;
	AUTHRINF* thinf = new AUTHRINF;
	thinf->stream = CRFALSE;
	if (!thinf)
	{
		CRThrowError(CRERR_OUTOFMEM, NULL);
		return nullptr;
	}
	thinf->dynPcm = dynPcm;
	thinf->inf = inf;
	//
	if (!_create_device_(thinf, inf))
		goto Failed;
	//
	if (FAILED(thinf->pRenderClient->GetBuffer(thinf->bufferFrameCount, &pData)))
		goto Failed;
	_fill_buffer_(pData, thinf->dynPcm, thinf->bufferFrameCount, inf, &thinf->offs);
	if (FAILED(thinf->pRenderClient->ReleaseBuffer(thinf->bufferFrameCount, 0)))
		goto Failed;
	//
	thinf->hnsActualDuration = thinf->bufferFrameCount * 1000 / inf->SampleRate;
	if (FAILED(thinf->pAudioClient->Start()))
		goto Failed;
	//开启线程
	thinf->idThis = CRThread(_audio_thread_, thinf);
End:
	return _generate_id_(thinf);
Failed:
	if (thinf->pDevice)
		thinf->pDevice->Release();
	if (thinf->pAudioClient)
		thinf->pAudioClient->Release();
	if (thinf->pRenderClient)
		thinf->pRenderClient->Release();
	CRThrowError(CRERR_AUDIO_FAILEDCOM, CRDES_AUDIO_FAILEDCOM);
	return nullptr;
}

CRAPI CRAUDIOPLAY CRAudioStream(CRWWINFO* inf, CRAudioStreamCbk func)
{
	if (!inited)
	{
		CRThrowError(CRERR_UNINITED, NULL);
		return nullptr;
	}
	if (!inf || !func)
	{
		CRThrowError(CRERR_INVALID, NULL);
		return nullptr;
	}
	BYTE* pData = nullptr;
	AUTHRINF* thinf = new AUTHRINF;
	if (!thinf)
	{
		CRThrowError(CRERR_OUTOFMEM, NULL);
		return nullptr;
	}
	thinf->stream = CRTRUE;
	thinf->cbk = func;
	thinf->inf = inf;
	thinf->pause = CRTRUE;
	//
	if (!_create_device_(thinf, inf))
		goto Failed;
	//
	if (FAILED(thinf->pRenderClient->GetBuffer(thinf->bufferFrameCount, &pData)))
		goto Failed;
	thinf->cbk(pData, thinf->numFramesAvailable, thinf->numFramesAvailable * thinf->inf->BlockAlign);
	if (FAILED(thinf->pRenderClient->ReleaseBuffer(thinf->bufferFrameCount, 0)))
		goto Failed;
	//
	thinf->hnsActualDuration = thinf->bufferFrameCount * 1000 / inf->SampleRate;
	//开启线程
	thinf->idThis = CRThread(_audio_stream_thread_, thinf);
End:
	return _generate_id_(thinf);
Failed:
	if (thinf->pDevice)
		thinf->pDevice->Release();
	if (thinf->pAudioClient)
		thinf->pAudioClient->Release();
	if (thinf->pRenderClient)
		thinf->pRenderClient->Release();
	CRThrowError(CRERR_AUDIO_FAILEDCOM, CRDES_AUDIO_FAILEDCOM);
	return nullptr;
}

#elif defined CR_LINUX

void _audio_thread_(void* data, CRTHREAD idThis)
{
	AUTHRINF* auinf = (AUTHRINF*)data;
	CRBOOL paused = CRFALSE;
	CRINT32 rc;
	CRUINT8* pData = new CRUINT8[auinf->frames * auinf->inf->BlockAlign];
	if (!pData)
		return;
	while (auinf->offs < CRStructureSize(auinf->dynPcm) && !auinf->stop)
	{
		if (!paused)
		{
			if (auinf->pause)
			{
				if (auinf->alsa_can_pause)
					snd_pcm_pause(auinf->handle, 1);
				else
					snd_pcm_drop(auinf->handle);
				paused = CRTRUE;
			}
			else
			{
				_fill_buffer_(pData, auinf->dynPcm, auinf->frames, auinf->inf, &auinf->offs);
				while (rc = snd_pcm_writei(auinf->handle, pData, auinf->frames) < 0)
				{
					if (rc == -EPIPE)
						snd_pcm_prepare(auinf->handle);
				}
			}
		}
		else
		{
			if (!auinf->pause)
			{
				if (auinf->alsa_can_pause)
					snd_pcm_pause(auinf->handle, 0);
				else
					snd_pcm_prepare(auinf->handle);
				paused = CRFALSE;
			}
			else
				CRSleep(1);
		}
	}
	snd_pcm_drain(auinf->handle);
	snd_pcm_close(auinf->handle);
	delete[] pData;
}

void _audio_stream_thread_(void* data, CRTHREAD idThis)
{
	AUTHRINF* auinf = (AUTHRINF*)data;
	CRBOOL paused = CRTRUE;
	CRINT32 rc;
	CRUINT8* pData = new CRUINT8[auinf->frames * auinf->inf->BlockAlign];
	if (!pData)
		return;
	while (!auinf->stop)
	{
		if (!paused)
		{
			if (auinf->pause)
			{
				if (auinf->alsa_can_pause)
					snd_pcm_pause(auinf->handle, 1);
				else
					snd_pcm_drop(auinf->handle);
				paused = CRTRUE;
			}
			else
			{
				auinf->cbk(pData, auinf->frames, auinf->frames * auinf->inf->BlockAlign);
				while (rc = snd_pcm_writei(auinf->handle, pData, auinf->frames) < 0)
				{
					if (rc == -EPIPE)
						snd_pcm_prepare(auinf->handle);
				}
			}
		}
		else
		{
			if (!auinf->pause)
			{
				if (auinf->alsa_can_pause)
					snd_pcm_pause(auinf->handle, 0);
				else
					snd_pcm_prepare(auinf->handle);
				paused = CRFALSE;
			}
			else
				CRSleep(1);
		}
	}
	snd_pcm_drain(auinf->handle);
	snd_pcm_close(auinf->handle);
	delete[] pData;
}

static inline CRBOOL _create_device_(AUTHRINF* thinf, CRWWINFO* inf)
{
	CRINT32 dir = 0;
	CRUINT32 val = 0;
	snd_pcm_uframes_t periodsize;
	snd_pcm_hw_params_t* params = nullptr;
	//获取设备句柄
	if (snd_pcm_open(&thinf->handle, "default", SND_PCM_STREAM_PLAYBACK, 0) < 0)
		goto Failed;
	snd_pcm_hw_params_malloc(&params);
	snd_pcm_hw_params_any(thinf->handle, params);
	thinf->alsa_can_pause = snd_pcm_hw_params_can_pause(params);
	snd_pcm_hw_params_set_access(thinf->handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
	//
	if (inf->BitsPerSample == 8)
		snd_pcm_hw_params_set_format(thinf->handle, params, SND_PCM_FORMAT_U8);
	else
		snd_pcm_hw_params_set_format(thinf->handle, params, SND_PCM_FORMAT_S16_LE);
	//
	snd_pcm_hw_params_set_channels(thinf->handle, params, inf->NumChannels);
	val = inf->SampleRate;
	snd_pcm_hw_params_set_rate_near(thinf->handle, params, &val, &dir);
	periodsize = 1024;
	snd_pcm_hw_params_set_period_size(thinf->handle, params, periodsize, 0);
	thinf->frames = periodsize << 1;
	snd_pcm_hw_params_set_buffer_size(thinf->handle, params, thinf->frames);
	if (snd_pcm_hw_params(thinf->handle, params) < 0)
		goto Failed;
	snd_pcm_hw_params_free(params);
	return CRTRUE;
Failed:
	if (params)
		snd_pcm_hw_params_free(params);
	return CRFALSE;
}

//反复试验之后，发现，原生的24位音频数据是不支持的，会直接变成噪音（旋律的话能出来一点点）。。。
//试过16位之后恍然大悟，明显就是溢出了。。。
//这个是我遇到过的最操蛋的bug
CRAPI CRAUDIOPLAY CRAudioBuffer(CRSTRUCTURE dynPcm, CRWWINFO* inf)
{
	if (!inited)
	{
		CRThrowError(CRERR_UNINITED, NULL);
		return nullptr;
	}
	if (!inf)
	{
		CRThrowError(CRERR_INVALID, NULL);
		return nullptr;
	}
	AUTHRINF* thinf = new AUTHRINF;
	if (!thinf)
	{
		CRThrowError(CRERR_OUTOFMEM, NULL);
		return nullptr;
	}
	thinf->stream = CRFALSE;
	thinf->dynPcm = dynPcm;
	thinf->inf = inf;
	if (!_create_device_(thinf, inf))
		goto Failed;
	thinf->idThis = CRThread(_audio_thread_, thinf);
	return _generate_id_(thinf);
Failed:
	if (thinf->handle)
		snd_pcm_close(thinf->handle);
	CRThrowError(CRERR_AUDIO_FAILEDALSA, CRDES_AUDIO_FAILEDALSA);
	return nullptr;
}

CRAPI CRAUDIOPLAY CRAudioStream(CRWWINFO* inf, CRAudioStreamCbk func)
{
	if (!inited)
	{
		CRThrowError(CRERR_UNINITED, NULL);
		return nullptr;
	}
	if (!inf || !func)
	{
		CRThrowError(CRERR_INVALID, NULL);
		return nullptr;
	}
	AUTHRINF* thinf = new AUTHRINF;
	if (!thinf)
	{
		CRThrowError(CRERR_OUTOFMEM, NULL);
		return nullptr;
	}
	thinf->stream = CRTRUE;
	thinf->cbk = func;
	thinf->inf = inf;
	thinf->pause = CRTRUE;
	if (!_create_device_(thinf, inf))
		goto Failed;
	thinf->idThis = CRThread(_audio_stream_thread_, thinf);
	return _generate_id_(thinf);
Failed:
	if (thinf->handle)
		snd_pcm_close(thinf->handle);
	CRThrowError(CRERR_AUDIO_FAILEDALSA, CRDES_AUDIO_FAILEDALSA);
	return nullptr;
}

#endif

CRAPI CRCODE CRAudioClose(CRAUDIOPLAY play)
{
	AUTHRINF* pInner = nullptr;
	CRTreeGet(audioPool, (CRLVOID*)&pInner, (CRINT64)play);
	if (!pInner || pInner->magic != CR_AUDIOMAGIC)
		return CRERR_INVALID;
	pInner->stop = CRTRUE;
	CRWaitThread(pInner->idThis);
	delete pInner;
	return 0;
}

CRAPI CRCODE CRAudioWait(CRAUDIOPLAY play)
{
	AUTHRINF* pInner = nullptr;
	CRTreeGet(audioPool, (CRLVOID*)&pInner, (CRINT64)play);
	if (!pInner || pInner->magic != CR_AUDIOMAGIC)
		return CRERR_INVALID;
	if (pInner->stream)  //对于即时音频流，不要做等待
		pInner->stop = CRTRUE;
	CRWaitThread(pInner->idThis);
	delete pInner;
	return 0;
}

CRAPI CRCODE CRAudioPause(CRAUDIOPLAY play)
{
	AUTHRINF* pInner = nullptr;
	CRTreeSeek(audioPool, (CRLVOID*)&pInner, (CRINT64)play);
	if (!pInner || pInner->magic != CR_AUDIOMAGIC)
		return CRERR_INVALID;
	pInner->pause = CRTRUE;
	return 0;
}

CRAPI CRCODE CRAudioResume(CRAUDIOPLAY play)
{
	AUTHRINF* pInner = nullptr;
	CRTreeSeek(audioPool, (CRLVOID*)&pInner, (CRINT64)play);
	if (!pInner || pInner->magic != CR_AUDIOMAGIC)
		return CRERR_INVALID;
	pInner->pause = CRFALSE;
	return 0;
}

CRAPI double CRAudioCheckProgress(CRAUDIOPLAY play)
{
	AUTHRINF* pInner = nullptr;
	CRTreeSeek(audioPool, (CRLVOID*)&pInner, (CRINT64)play);
	if (!pInner || pInner->magic != CR_AUDIOMAGIC)
		return -1;
	if (pInner->stream)
		return 0.0f;
	return (double)pInner->offs / (double)CRStructureSize(pInner->dynPcm);
}
