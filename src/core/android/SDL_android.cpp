/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2012 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
#include "SDL_config.h"
#include "SDL_stdinc.h"

#ifdef __ANDROID__

#include "SDL_android.h"

extern "C" {
#include "../../events/SDL_events_c.h"
#include "../../video/android/SDL_androidkeyboard.h"
#include "../../video/android/SDL_androidtouch.h"
#include "../../video/android/SDL_androidvideo.h"

#include <android/log.h>
#define LOG_TAG "SDL_android"
//#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
//#define LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#define LOGI(...) do {} while (false)
#define LOGE(...) do {} while (false)


/* Implemented in audio/android/SDL_androidaudio.c */
extern void Android_RunAudioThread();
} // C

/*******************************************************************************
 This file links the Java side of Android with libsdl
*******************************************************************************/
#include <jni.h>
#include <android/log.h>


/*******************************************************************************
                               Globals
*******************************************************************************/
static JNIEnv* mEnv = NULL;
static JNIEnv* mAudioEnv = NULL;
static JavaVM* mJavaVM;

// Main activity
static jclass mActivityClass;

// method signatures
static jmethodID midAudioInit;
static jmethodID midAudioWriteShortBuffer;
static jmethodID midAudioWriteByteBuffer;
static jmethodID midAudioQuit;

// Accelerometer data storage
static float fLastAccelerometer[3];
static bool bHasNewData;

/*******************************************************************************
                 Functions called by JNI
*******************************************************************************/

// Library init
extern "C" jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    JNIEnv *env;
    mJavaVM = vm;
    LOGI("JNI_OnLoad called");
    if (mJavaVM->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
        LOGE("Failed to get the environment using GetEnv()");
        return -1;
    }

    return JNI_VERSION_1_4;
}

// Called before SDL_main() to initialize JNI bindings
extern "C" void SDL_Android_Init(JNIEnv* env, jclass cls)
{
    __android_log_print(ANDROID_LOG_INFO, "SDL", "SDL_Android_Init()");

    mEnv = env;
    mActivityClass = (jclass)env->NewGlobalRef(cls);

    midCreateGLContext = mEnv->GetStaticMethodID(mActivityClass,
                                "createGLContext","(II)Z");
    midFlipBuffers = mEnv->GetStaticMethodID(mActivityClass,
                                "flipBuffers","()V");
    midAudioInit = mEnv->GetStaticMethodID(mActivityClass, 
                                "audioInit", "(IZZI)Ljava/lang/Object;");
    midAudioWriteShortBuffer = mEnv->GetStaticMethodID(mActivityClass,
                                "audioWriteShortBuffer", "([S)V");
    midAudioWriteByteBuffer = mEnv->GetStaticMethodID(mActivityClass,
                                "audioWriteByteBuffer", "([B)V");
    midAudioQuit = mEnv->GetStaticMethodID(mActivityClass,
                                "audioQuit", "()V");

    bHasNewData = false;

    if(!midCreateGLContext || !midFlipBuffers || !midAudioInit ||
       !midAudioWriteShortBuffer || !midAudioWriteByteBuffer || !midAudioQuit) {
        __android_log_print(ANDROID_LOG_WARN, "SDL", "SDL: Couldn't locate Java callbacks, check that they're named and typed correctly");
    }
    __android_log_print(ANDROID_LOG_INFO, "SDL", "SDL_Android_Init() finished!");
}

// Resize
extern "C" void Java_org_libsdl_app_SDLActivity_onNativeResize(
                                    JNIEnv* env, jclass jcls,
                                    jint width, jint height, jint format)
{
    Android_SetScreenResolution(width, height, format);
}

// Keydown
extern "C" void Java_org_libsdl_app_SDLActivity_onNativeKeyDown(
                                    JNIEnv* env, jclass jcls, jint keycode)
{
    Android_OnKeyDown(keycode);
}

// Keyup
extern "C" void Java_org_libsdl_app_SDLActivity_onNativeKeyUp(
                                    JNIEnv* env, jclass jcls, jint keycode)
{
    Android_OnKeyUp(keycode);
}

// Touch
extern "C" void Java_org_libsdl_app_SDLActivity_onNativeTouch(
                                    JNIEnv* env, jclass jcls,
                                    jint touch_device_id_in, jint pointer_finger_id_in,
                                    jint action, jfloat x, jfloat y, jfloat p)
{
    Android_OnTouch(touch_device_id_in, pointer_finger_id_in, action, x, y, p);
}

// Accelerometer
extern "C" void Java_org_libsdl_app_SDLActivity_onNativeAccel(
                                    JNIEnv* env, jclass jcls,
                                    jfloat x, jfloat y, jfloat z)
{
    fLastAccelerometer[0] = x;
    fLastAccelerometer[1] = y;
    fLastAccelerometer[2] = z;
    bHasNewData = true;
}

// Quit
void Context::nativeQuit()
{
    // Inject a SDL_QUIT event
    SDL_SendQuit();
}

// Pause
void Context::nativePause()
{
    if (!Android_Window)
        return;
    SDL_SendWindowEvent(Android_Window, SDL_WINDOWEVENT_FOCUS_LOST, 0, 0);
    SDL_SendWindowEvent(Android_Window, SDL_WINDOWEVENT_MINIMIZED, 0, 0);
}

// Resume
void Context::nativeResume()
{
    if (!Android_Window)
        return;
    SDL_SendWindowEvent(Android_Window, SDL_WINDOWEVENT_FOCUS_GAINED, 0, 0);
    SDL_SendWindowEvent(Android_Window, SDL_WINDOWEVENT_RESTORED, 0, 0);
}

extern "C" void Java_org_libsdl_app_SDLActivity_nativeRunAudioThread(
                                    JNIEnv* env, jclass cls)
{
    /* This is the audio thread, with a different environment */
    mAudioEnv = env;

    Android_RunAudioThread();
}


/*******************************************************************************
             Functions called by SDL into Java
*******************************************************************************/
extern "C" SDL_bool Android_JNI_CreateContext(int majorVersion, int minorVersion)
{
    return Context::GetContext()->createGLContext(majorVersion, minorVersion);
}

extern "C" void Android_JNI_SwapWindow()
{
    Context::GetContext()->flipBuffers();
}

extern "C" void Android_JNI_SetActivityTitle(const char *title)
{
    jmethodID mid;

    mid = mEnv->GetStaticMethodID(mActivityClass,"setActivityTitle","(Ljava/lang/String;)V");
    if (mid) {
        mEnv->CallStaticVoidMethod(mActivityClass, mid, mEnv->NewStringUTF(title));
    }
}

extern "C" SDL_bool Android_JNI_GetAccelerometerValues(float values[3])
{
    int i;
    SDL_bool retval = SDL_FALSE;

    if (bHasNewData) {
        for (i = 0; i < 3; ++i) {
            values[i] = fLastAccelerometer[i];
        }
        bHasNewData = false;
        retval = SDL_TRUE;
    }

    return retval;
}

//
// Audio support
//
static jboolean audioBuffer16Bit = JNI_FALSE;
static jboolean audioBufferStereo = JNI_FALSE;
static jobject audioBuffer = NULL;
static void* audioBufferPinned = NULL;

extern "C" int Android_JNI_OpenAudioDevice(int sampleRate, int is16Bit, int channelCount, int desiredBufferFrames)
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
    audioBuffer16Bit = is16Bit;
    audioBufferStereo = channelCount > 1;

    audioBuffer = env->CallStaticObjectMethod(mActivityClass, midAudioInit, sampleRate, audioBuffer16Bit, audioBufferStereo, desiredBufferFrames);

    if (audioBuffer == NULL) {
        __android_log_print(ANDROID_LOG_WARN, "SDL", "SDL audio: didn't get back a good audio buffer!");
        return 0;
    }
    audioBuffer = env->NewGlobalRef(audioBuffer);

    jboolean isCopy = JNI_FALSE;
    if (audioBuffer16Bit) {
        audioBufferPinned = env->GetShortArrayElements((jshortArray)audioBuffer, &isCopy);
        audioBufferFrames = env->GetArrayLength((jshortArray)audioBuffer);
    } else {
        audioBufferPinned = env->GetByteArrayElements((jbyteArray)audioBuffer, &isCopy);
        audioBufferFrames = env->GetArrayLength((jbyteArray)audioBuffer);
    }
    if (audioBufferStereo) {
        audioBufferFrames /= 2;
    }
 
    if (isAttached) {
        mJavaVM->DetachCurrentThread();
    }

    return audioBufferFrames;
}

extern "C" void * Android_JNI_GetAudioBuffer()
{
    return audioBufferPinned;
}

extern "C" void Android_JNI_WriteAudioBuffer()
{
    if (audioBuffer16Bit) {
        mAudioEnv->ReleaseShortArrayElements((jshortArray)audioBuffer, (jshort *)audioBufferPinned, JNI_COMMIT);
        mAudioEnv->CallStaticVoidMethod(mActivityClass, midAudioWriteShortBuffer, (jshortArray)audioBuffer);
    } else {
        mAudioEnv->ReleaseByteArrayElements((jbyteArray)audioBuffer, (jbyte *)audioBufferPinned, JNI_COMMIT);
        mAudioEnv->CallStaticVoidMethod(mActivityClass, midAudioWriteByteBuffer, (jbyteArray)audioBuffer);
    }

    /* JNI_COMMIT means the changes are committed to the VM but the buffer remains pinned */
}

extern "C" void Android_JNI_CloseAudioDevice()
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

    if (audioBuffer) {
        env->DeleteGlobalRef(audioBuffer);
        audioBuffer = NULL;
        audioBufferPinned = NULL;
    }

    if (isAttached) {
        mJavaVM->DetachCurrentThread();
    }
}

// Test for an exception and call SDL_SetError with its detail if one occurs
static bool Android_JNI_ExceptionOccurred()
{
    jthrowable exception = mEnv->ExceptionOccurred();
    if (exception != NULL) {
        jmethodID mid;

        // Until this happens most JNI operations have undefined behaviour
        mEnv->ExceptionClear();

        jclass exceptionClass = mEnv->GetObjectClass(exception);
        jclass classClass = mEnv->FindClass("java/lang/Class");

        mid = mEnv->GetMethodID(classClass, "getName", "()Ljava/lang/String;");
        jstring exceptionName = (jstring)mEnv->CallObjectMethod(exceptionClass, mid);
        const char* exceptionNameUTF8 = mEnv->GetStringUTFChars(exceptionName, 0);

        mid = mEnv->GetMethodID(exceptionClass, "getMessage", "()Ljava/lang/String;");
        jstring exceptionMessage = (jstring)mEnv->CallObjectMethod(exception, mid);

        if (exceptionMessage != NULL) {
            const char* exceptionMessageUTF8 = mEnv->GetStringUTFChars(
                    exceptionMessage, 0);
            SDL_SetError("%s: %s", exceptionNameUTF8, exceptionMessageUTF8);
            mEnv->ReleaseStringUTFChars(exceptionMessage, exceptionMessageUTF8);
            mEnv->DeleteLocalRef(exceptionMessage);
        } else {
            SDL_SetError("%s", exceptionNameUTF8);
        }

        mEnv->ReleaseStringUTFChars(exceptionName, exceptionNameUTF8);
        mEnv->DeleteLocalRef(exceptionName);
        mEnv->DeleteLocalRef(classClass);
        mEnv->DeleteLocalRef(exceptionClass);
        mEnv->DeleteLocalRef(exception);

        return true;
    }

    return false;
}

#endif /* __ANDROID__ */

/* vi: set ts=4 sw=4 expandtab: */
