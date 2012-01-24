
#ifndef SDL_ANDROID_AUDIO_OSLES_H_
#define SDL_ANDROID_AUDIO_OSLES_H_

#include <SDL.h>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_AndroidConfiguration.h>
#include <SLES/OpenSLES_Android.h>

#include "SDL_audio.h"

namespace SDL{ namespace Android{
class OSLES : public Audio
{
protected:
	bool hasFailed(SLresult result);
	std::string&& getMessage(SLresult result);
	void throwResult(SLresult result);
private:
    SLObjectItf mEngineObject;
    SLObjectItf mPlayerObject;

    SLEngineItf mEngine;
    SLPlayItf   mPlayer;
    SLDataSource mDataSource;
    SLDataSink   mDataSink;
    SLDataLocator_AndroidSimpleBufferQueue mBufferQueueLocator;
    SLDataFormat_PCM_ mSourceFormat;

    SLDataLocator_OutputMix mOutputMixLocator;

    SLOutputMixItf mOutputMix;

    SLBufferQueueItf mBufferQueue;

    bool mIs16Bit;
    bool mIsStereo;

    void* mAudioBuffer;
    size_t mAudioBufferByteSize;

	template<typename T>
	void WriteBuffer(T* buffer, size_t buffer_size)
	{
		auto result = mBufferQueue->Enqueue(mBufferQueue, buffer, buffer_size);

		if( hasFailed(result) )
			return;

#if 0
	        for (int i = 0; i < buffer_size; ) {
	            int result = mAudioTrack->write(buffer, i, buffer_size - i);
	            if (result > 0) {
	                i += result;
	            } else if (result == 0) {
	                sleep(1);
	            } else {
	                LOGW("SDL audio: error return from write(short)");
	                return;
	            }
	        }
#endif
	};
	void Play();
public:
    OSLES(Context& context);

	int Init(int sampleRate, bool is16Bit, Uint8 channelCount, int desiredBufferFrames);
    void Run(std::function<void ()> func);

	void* GetBuffer();
	void WriteBuffer();

	void Quit();
	void Stop();
	void Resume();
};
}}

#endif // SDL_ANDROID_AUDIO_OSLES_H_
