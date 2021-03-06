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
#include "SDL_context.h"
#include "include.h"

extern "C" {
#include "../../events/SDL_events_c.h"
#include "../../video/android/SDL_androidkeyboard.h"
#include "../../video/android/SDL_androidtouch.h"
#include "../../video/android/SDL_androidvideo.h"

#include <android/log.h>

/* Implemented in audio/android/SDL_androidaudio.c */
extern void Android_RunAudioThread();
} // C

#include <android/log.h>


/*******************************************************************************
                               Globals
*******************************************************************************/

// Accelerometer data storage
static float fLastAccelerometer[3];
static bool bHasNewData;

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


    bHasNewData = false;
}

// Resize
void Context::onNativeResize(int width, int height, int format)
{
    Android_SetScreenResolution(width, height, format);
}

// Keydown
void Context::onNativeKeyDown(AInputEvent* pEvent)
{
	const auto keycode = AKeyEvent_getKeyCode(pEvent);
    Android_OnKeyDown(keycode);
}

// Keyup
void Context::onNativeKeyUp(AInputEvent* pEvent)
{
	const auto keycode = AKeyEvent_getKeyCode(pEvent);
    Android_OnKeyUp(keycode);
}

// Touch
void Context::onNativeTouch(int32_t touch_device_id_in, int32_t pointer_finger_id_in,
                            int32_t action, float x, float y, float p)
{
    Android_OnTouch(touch_device_id_in, pointer_finger_id_in, action, x, y, p);
}

// Accelerometer
void Context::onNativeAccel(float x, float y, float z)
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

extern "C" SDL_bool Android_CreateContext(int majorVersion, int minorVersion)
{
    return Context::GetContext()->createGLContext(majorVersion, minorVersion);
}

extern "C" void Android_SwapWindow()
{
    Context::GetContext()->flipBuffers();
}

extern "C" void Android_SetActivityTitle(const char *title)
{
    Context::GetContext()->setActivityTitle(title);
}

extern "C" SDL_bool Android_GetAccelerometerValues(float values[3])
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

#endif /* __ANDROID__ */

/* vi: set ts=4 sw=4 expandtab: */
