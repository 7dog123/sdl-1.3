
#ifndef SDL_AUDIO_TRACK_H_
#define SDL_AUDIO_TRACK_H_

#include "SDL_audio.h"
#include "SDL_jni.h"
#include <jni.h>

namespace SDL{ namespace Android{

class AudioTrack : public Audio, public JNI
{
public:
	AudioTrack(Context& context, android_app* app);

	int Init(int sampleRate, bool is16Bit, Uint8 channelCount, int desiredBufferFrames);
	void Run(std::function<void ()> func);
	void Stop();
	void Resume();
	void Quit();
	void* GetBuffer();
	void WriteBuffer();
private:
	JNIEnv* mAudioEnv;

	jboolean mIs16Bit;
	jboolean mIsStereo;
	jarray mAudioBuffer;
	void* mpAudioBufferPinned;

	// method signatures
	jmethodID midAudioInit;
	jmethodID midAudioPlay;
	jmethodID midAudioStop;
	jmethodID midAudioResume;
	jmethodID midAudioWriteShortBuffer;
	jmethodID midAudioWriteByteBuffer;
	jmethodID midAudioQuit;
};

}}


#endif /* SDL_AUDIO_TRACK_H_ */
