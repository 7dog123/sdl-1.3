#include "SDL_context.h"
#include "include.h"

Context::Context() :
	mSDLThread(nullptr)
{
}

    // Setup
void Context::onCreate(android_app* app)
{
	//LOGV("onCreate()");
	mSingleton = this;
}
// Java functions called from C
bool Context::createGLContext(int majorVersion, int minorVersion)
{
	return initEGL(majorVersion, minorVersion);
}

void Context::flipBuffers()
{
        flipEGL();
}


Context* Context::GetContext()
{
	return mSingleton;
}
bool Context::eglLog(const char* name)
{
	LOGE(name);
	return false;
}

// EGL functions
bool Context::initEGL(int majorVersion, int minorVersion)
{
	if (mEGLDisplay) {
		createEGLSurface();
		return true;
	}
	//Log.v("SDL", "Starting up OpenGL ES " + majorVersion + "." + minorVersion);

	auto egl = eglGetCurrentContext();

	if( egl == EGL_NO_CONTEXT )
		return eglLog("No context.");

	auto dpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);

	if( dpy == EGL_NO_DISPLAY )
		return eglLog("No display.");

	int version[2];
	auto result = eglInitialize(dpy, version, version + 1);

	if( result == EGL_FALSE )
	{
		auto error = eglGetError();

		if( error == EGL_NOT_INITIALIZED )
			return eglLog("EGL initialize failed.");
		else if( error == EGL_BAD_DISPLAY )
			return eglLog("No valid display connection.");
		else
			return eglLog("Undefined error.");
	}

	int renderableType = 0;
	if (majorVersion == 2) {
		renderableType = EGL_OPENGL_ES2_BIT;
	} else if (majorVersion == 1) {
		renderableType = EGL_OPENGL_ES_BIT;
	}
	const int configSpec[] = {
		//EGL10.EGL_DEPTH_SIZE,   16,
		EGL_RENDERABLE_TYPE, renderableType,
		EGL_NONE
	};

	EGLConfig configs[1];
	int num_config[1];

	if (!eglChooseConfig(dpy, configSpec, configs, 1, num_config) || num_config[0] == 0)
		return eglLog("No EGL config available");

	auto config = configs[0];

                /*int EGL_CONTEXT_CLIENT_VERSION=0x3098;
                int contextAttrs[] = new int[] { EGL_CONTEXT_CLIENT_VERSION, majorVersion, EGL10.EGL_NONE };
                EGLContext ctx = egl.eglCreateContext(dpy, config, EGL10.EGL_NO_CONTEXT, contextAttrs);

                if (ctx == EGL10.EGL_NO_CONTEXT) {
                    Log.e("SDL", "Couldn't create context");
                    return false;
                }
                SDLActivity.mEGLContext = ctx;*/
	mEGLDisplay = dpy;
	mEGLConfig = config;
	mGLMajor = majorVersion;
	mGLMinor = minorVersion;

	return createEGLSurface();
}

bool Context::createEGLContext()
{
	auto egl = eglGetCurrentContext();
	int contextAttrs[] = new int[] { EGL_CONTEXT_CLIENT_VERSION, mGLMajor, EGL_NONE };
	mEGLContext = eglCreateContext(mEGLDisplay, mEGLConfig, EGL_NO_CONTEXT, contextAttrs);
	if (mEGLContext == EGL_NO_CONTEXT)
		return eglLog("Couldn't create context");

	return true;
}

bool Context::createEGLSurface(android_app* app)
{
	if (!mEGLDisplay || !mEGLConfig)
		return false;

	auto egl = eglGetCurrentContext();
	if (!mEGLContext) createEGLContext();

	LOGV("Creating new EGL Surface");
	auto surface = eglCreateWindowSurface(mEGLDisplay, mEGLConfig, (EGLNativeWindowType)app->window, nullptr);
	if (surface == EGL_NO_SURFACE)
		return eglLog("Couldn't create surface");

	if (!eglMakeCurrent(mEGLDisplay, surface, surface, mEGLContext))
	{
		eglLog("Old EGL Context does not work, trying with a new one");
		createEGLContext();
		if (!eglMakeCurrent(mEGLDisplay, surface, surface, mEGLContext))
			return eglLog("Failed making EGL Context current");
	}

	mEGLSurface = surface;
	return true;
}

    // EGL buffer flip
void Context::flipEGL()
{
	auto pContext = eglGetCurrentContext();
	auto result = eglWaitNative(EGL_CORE_NATIVE_ENGINE);

	if( result )
		// drawing here
		result = eglWaitGL();

	if( result )
		result = eglSwapBuffers(mEGLDisplay, mEGLSurface);

	if( !result )
	{ // Error occured
		auto error = eglGetError();
		LOGVF("flipEGL(): %d" + error);
	}
}

void onAppCmd(android_app* app, int32_t cmd)
{
	auto pContext = (Context*)app->userData;

	pContext->mCommands.mpApp = app;
#define CMD pContext->mCommands

	switch( cmd )
	{
	case APP_CMD_INPUT_CHANGED:        CMD.inputQueueChanged();        break;
	case APP_CMD_INIT_WINDOW:          CMD.windowInit();               break;
	case APP_CMD_TERM_WINDOW:          CMD.windowTerminating();        break;
	case APP_CMD_WINDOW_RESIZED:       CMD.windowResized();            break;
	case APP_CMD_WINDOW_REDRAW_NEEDED: CMD.windowNeedsRedraw();        break;
	case APP_CMD_CONTENT_RECT_CHANGED: CMD.windowContentAreaChanged(); break;
	case APP_CMD_GAINED_FOCUS:         CMD.activityFocus();            break;
	case APP_CMD_LOST_FOCUS:           CMD.activityUnfocus();          break;
	case APP_CMD_CONFIG_CHANGED:       CMD.configurationChanged();     break;
	case APP_CMD_LOW_MEMORY:           CMD.systemMemoryLow();          break;
	case APP_CMD_START:                CMD.activityStarted();          break;
	case APP_CMD_RESUME:               CMD.activityResumed();          break;
	case APP_CMD_SAVE_STATE:           CMD.stateSave();                break;
	case APP_CMD_PAUSE:                CMD.activityPaused();           break;
	case APP_CMD_STOP:                 CMD.activityStopped();          break;

#undef CMD


#if 0
	case APP_CMD_SAVE_STATE:
		// We have to save the current state

		hibernate(*pContext);
		break;
	case APP_CMD_INIT_WINDOW:
		// The window is being shown, get it ready.
		if (engine->app->window != NULL)
		{
			engine_init_display(engine);
			engine_draw_frame(engine);
		}
		break;
	case APP_CMD_TERM_WINDOW:
		// The window is being hidden or closed, clean it up.
		hibernate(*pContext);
		//engine_term_display(engine);
		break;
	case APP_CMD_GAINED_FOCUS:
		wakeup(*pContext);
		break;
	case APP_CMD_LOST_FOCUS:
		hibernate(*pContext);
/*
	            // When our app loses focus, we stop monitoring the accelerometer.
	            // This is to avoid consuming battery while not being used.
	            if (engine->accelerometerSensor != NULL) {
	                ASensorEventQueue_disableSensor(engine->sensorEventQueue,
	                        engine->accelerometerSensor);
	            }
	            // Also stop animating.
	            engine->animating = 0;
	            engine_draw_frame(engine);
	            break;
	    }
*/
		break;
#endif
	}

	pContext->mCommands.mpApp = nullptr;
}

int32_t onInputEvent(struct android_app* app, AInputEvent* event)
{
	auto pContext = (Context*)app->userData;

	pContext->mCommands.mpApp = app;
	auto type = AInputEvent_getType(event);
#define CMD pContext->mCommands

	switch( type )
	{
	case AINPUT_EVENT_TYPE_KEY:
		return CMD.keyInput(event);
	case AINPUT_EVENT_TYPE_MOTION:
		return CMD.touchInput(event);
	default:
		pContext->LOGWF("Unknown Event: %i", type);
		return 0;
	}
#undef CMD
}
