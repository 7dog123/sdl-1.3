
#include "SDL_audio_track.h"
#include "include.h"

AudioTrack::AudioTrack(Context& context, android_app* app) :
	Audio(context),
	JNI(app),
	mIs16Bit(JNI_FALSE),
	mIsStereo(JNI_FALSE),
	mAudioBuffer(nullptr),
	mpAudioBufferPinned(nullptr),
	midAudioInit(
		mEnv->GetStaticMethodID(mActivityClass,
	                            "init", "(IZZI)Ljava/lang/Object;")),
	midAudioWriteShortBuffer(
		mEnv->GetStaticMethodID(mActivityClass,
		                        "writeBuffer", "([S)V")),
	midAudioWriteByteBuffer(
		mEnv->GetStaticMethodID(mActivityClass,
								"writeBuffer", "([B)V")),
	midAudioPlay(
		mEnv->GetStaticMethodID(mActivityClass,
		                        "play", "()V")),
	midAudioQuit(
		mEnv->GetStaticMethodID(mActivityClass,
								"quit", "()V"))
{
	//static JNIEnv* mAudioEnv = NULL;

	LOGI("SDL_Android_Init()");

	if( !midAudioInit || !midAudioWriteShortBuffer ||
	    !midAudioWriteByteBuffer || !midAudioQuit )
		LOGW("SDL: Couldn't locate Java callbacks, check that they're named and typed correctly");

	LOGI("SDL_Android_Init() finished!");
}

int AudioTrack::Init(int sampleRate, bool is16Bit, Uint8 channelCount, int desiredBufferFrames)
{
    int audioBufferFrames;

    int status;
    JNIEnv *env;
    static bool isAttached = false;
    status = mJavaVM->GetEnv((void **) &env, JNI_VERSION_1_4);
    if(status < 0) {
        LOGE("callback_handler: failed to get JNI environment, assuming native thread");
        status = mJavaVM->AttachCurrentThread(&env, NULL);
        if(status < 0) {
            LOGE("callback_handler: failed to attach current thread");
            return 0;
        }
        isAttached = true;
    }


    __android_log_print(ANDROID_LOG_VERBOSE, "SDL", "SDL audio: opening device");
    mIs16Bit = is16Bit;
    mIsStereo = channelCount > 1;

    mAudioBuffer = env->CallStaticObjectMethod(mActivityClass, midAudioInit, sampleRate, mIs16Bit, mIsStereo, desiredBufferFrames);

    if (mAudioBuffer == NULL) {
        __android_log_print(ANDROID_LOG_WARN, "SDL", "SDL audio: didn't get back a good audio buffer!");
        return 0;
    }
    mAudioBuffer = env->NewGlobalRef(mAudioBuffer);

    jboolean isCopy = JNI_FALSE;
    if (mIs16Bit) {
        mpAudioBufferPinned = env->GetShortArrayElements((jshortArray)mAudioBuffer, &isCopy);
        audioBufferFrames = env->GetArrayLength((jshortArray)mAudioBuffer);
    } else {
    	mpAudioBufferPinned = env->GetByteArrayElements((jbyteArray)mAudioBuffer, &isCopy);
        audioBufferFrames = env->GetArrayLength((jbyteArray)mAudioBuffer);
    }
    if (mIsStereo) {
        audioBufferFrames /= 2;
    }

    if (isAttached) {
        mJavaVM->DetachCurrentThread();
    }

    return audioBufferFrames;
}

void AudioTrack::Run(std::function<void ()> func)
{
	// Start might get called from a special audio thread
	// so we need to get the thread environment

    jint status = mJavaVM->GetEnv((void **)&mAudioEnv, JNI_VERSION_1_4);
    if(status < 0) {
        LOGE("callback_handler: failed to get JNI environment, assuming native thread");
        status = mJavaVM->AttachCurrentThread(&mAudioEnv, NULL);
        if(status < 0) {
            LOGE("callback_handler: failed to attach current thread");
        }
    }

    mAudioEnv->CallStaticVoidMethod(mActivityClass, midAudioPlay, nullptr);

	func();
}

void* AudioTrack::GetBuffer()
{
    return mpAudioBufferPinned;
}

void AudioTrack::WriteBuffer()
{
    if (mIs16Bit) {
        mAudioEnv->ReleaseShortArrayElements((jshortArray)mAudioBuffer, (jshort *)mpAudioBufferPinned, JNI_COMMIT);
        mAudioEnv->CallStaticVoidMethod(mActivityClass, midAudioWriteShortBuffer, (jshortArray)mAudioBuffer);
    } else {
        mAudioEnv->ReleaseByteArrayElements((jbyteArray)mAudioBuffer, (jbyte *)mpAudioBufferPinned, JNI_COMMIT);
        mAudioEnv->CallStaticVoidMethod(mActivityClass, midAudioWriteByteBuffer, (jbyteArray)mAudioBuffer);
    }

    /* JNI_COMMIT means the changes are committed to the VM but the buffer remains pinned */
}

void AudioTrack::Stop()
{

}

void AudioTrack::Resume()
{

}

void AudioTrack::Quit()
{
    int status;
    JNIEnv *env;
    static bool isAttached = false;
    status = mJavaVM->GetEnv((void **) &env, JNI_VERSION_1_4);
    if(status < 0) {
        LOGE("callback_handler: failed to get JNI environment, assuming native thread");
        status = mJavaVM->AttachCurrentThread(&env, NULL);
        if(status < 0) {
            LOGE("callback_handler: failed to attach current thread");
            return;
        }
        isAttached = true;
    }

    env->CallStaticVoidMethod(mActivityClass, midAudioQuit);

    if (mAudioBuffer) {
        env->DeleteGlobalRef(mAudioBuffer);
        mAudioBuffer = NULL;
        mpAudioBufferPinned = NULL;
    }

    if (isAttached) {
        mJavaVM->DetachCurrentThread();
    }
}



