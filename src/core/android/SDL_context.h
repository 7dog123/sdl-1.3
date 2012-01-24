
#ifndef SDL_ANDROID_CONTEXT_H_
#define SDL_ANDROID_CONTEXT_H_

#include <EGL/egl.h>
#include "SDL_base.h"
#include "SDL_sensors.h"
namespace SDL{ namespace Android {
class Context : public Base
{
public:
	Context();

	void flipEGL();
	bool createEGLSurface();
	bool createEGLContext();
	bool initEGL(int majorVersion, int minorVersion);
	void startApp();
	void setActivityTitle(const std::string& title);
	void flipBuffers();
	bool createGLContext(int majorVersion, int minorVersion);
	static Context* GetContext();
	void nativeInit();
    void nativeQuit();
    void nativePause();
    void nativeResume();
    void nativeRunAudioThread();
    void onNativeResize (int x, int y, int format);
    void onNativeKeyDown(AInputEvent* pEvent);
    void onNativeKeyUp  (AInputEvent* pEvent);
    void onNativeTouch  (int32_t touchDevId, int32_t pointerFingerId,
                         int32_t action, float x,
                         float y, float p);
    void onNativeAccel  (float x, float y, float z);

    Audio* GetAudio();
    void SetAudio(Audio* pAudio);
protected:
	void onCreate(android_app* app);
	void onStart(android_app* app);
	void onPause(android_app* app);
	void onResume(android_app* app);
	void onDestroy(android_app* app);
private:

    static Context* mSingleton;

    // This is what SDL runs in. It invokes SDL_main(), eventually
    SDL_Thread* mSDLThread;

    // Audio
    Audio* mpAudio;
    Sensors mSensors;

    // EGL private objects
    EGLContext  mEGLContext;
    EGLSurface  mEGLSurface;
    EGLDisplay  mEGLDisplay;
    EGLConfig   mEGLConfig;
    int mGLMajor, mGLMinor;

    bool createEGLSurface(android_app* app);
    bool createEGLContext();
    bool eglLog(const char* name);

    class Commands : public Base
    {
    public:
    	Commands(Context& context);

        void inputQueueChanged();
        void windowInit();
        void windowTerminating();
        void windowResized();
        void windowNeedsRedraw();
        void windowContentAreaChanged();
        void activityFocus();
        void activityUnfocus();
        void configurationChanged();
        void systemMemoryLow();
        void activityStarted();
        void activityResumed();
        void stateSave();
        void activityPaused();
        void activityStopped();
        void activityDestroy();

        bool keyInput(AInputEvent* event);
        bool touchInput(AInputEvent* event);

        android_app* mpApp;
    private:
        Context& mContext;
        void _activateInput();
        void _deactivateInput();
    };

    Commands mCommands;

};
}}


#endif // SDL_ANDROID_CONTEXT_H_
