
#ifndef SDL_ANDROID_AUDIO_H_
#define SDL_ANDROID_AUDIO_H_

/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
#include "SDL_subsystem.h"

namespace SDL{ namespace Android{

class Audio : public Subsystem
{
private:
    SDL_Thread* mAudioThread;
    bool mAbortThread;

	void StartThread();
	void StopThread();
protected:
	Audio(Context& context);
	virtual ~Audio();

	virtual void Run(std::function<void ()> func) = 0;
	virtual int Init(int sampleRate, bool is16Bit, Uint8 channelCount, int desiredBufferFrames) = 0;
	virtual void* GetBuffer() = 0;
	virtual void WriteBuffer() = 0;
};
}}

/* *INDENT-OFF* */
extern "C" {
/* *INDENT-ON* */
#endif //__cplusplus

// Audio support
int Android_OpenAudioDevice(int sampleRate, int is16Bit, int channelCount, int desiredBufferFrames);
void* Android_GetAudioBuffer();
void Android_WriteAudioBuffer();
void Android_CloseAudioDevice();

/* Ends C function definitions when using C++ */
#ifdef __cplusplus
/* *INDENT-OFF* */
}
/* *INDENT-ON* */
#endif


#endif // SDL_ANDROID_AUDIO_H_

