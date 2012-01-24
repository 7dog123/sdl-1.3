
#include "include.h"
#include "SDL_audio.h"
Audio::Audio(Context& context) :
	Subsystem(context),
	mAudioThread(nullptr),
	mAbortThread(false)
{
}

int SDLCALL run(void* data)
{
	// I'd take REALTIME if I could get it!
	SDL_SetThreadPriority(SDL_THREAD_PRIORITY_HIGH);

	auto pAudio = (Audio*)data;

	try
	{
		std::function<void ()> func = Android_RunAudioThread;
		pAudio->Run(func);
	}
	catch( ... )
	{
		Base::LOGW("AudioTrack::Run aborted because received exception.");
	}
	return 0;
}

void Audio::StartThread()
{
	StopThread();
	mAudioThread = SDL_CreateThread( run,
	                                 "audio",
	                                 mContext.GetAudio());
}

void Audio::StopThread()
{
	if( !mAudioThread )
		return;

	mAbortThread = true;
	int status;
	SDL_WaitThread(mAudioThread, &status);
	mAbortThread = false;
	mAudioThread = nullptr;
}

extern "C" int Android_OpenAudioDevice(int sampleRate, int is16Bit, Uint8 channelCount, int desiredBufferFrames)
{
	Audio* pAudio;
	Context& context = Context::GetContext();

	context.SetAudio(pAudio);

	return pAudio->Init(sampleRate, is16Bit, channelCount, desiredBufferFrames);
}

Audio* GetAudio()
{
	return Context::GetContext()->GetAudio();
}

extern "C" void * Android_GetAudioBuffer()
{
	return GetAudio()->GetBuffer();
}

extern "C" void Android_WriteAudioBuffer()
{
	GetAudio()->WriteBuffer();
}

extern "C" void Android_CloseAudioDevice()
{
	Context& context = Context::GetContext();
	Audio* pAudio = context.GetAudio();
	pAudio->Quit();
	context.SetAudio(nullptr);
	delete pAudio;
}

