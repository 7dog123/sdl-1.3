
#include "include.h"
#include "SDL_audio.h"

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

