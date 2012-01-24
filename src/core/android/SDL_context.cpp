#include "SDL_context.h"
#include "include.h"

Context::Context() :
	mSDLThread(nullptr)
{
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

