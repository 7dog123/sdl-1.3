
#include "SDL_context.h"
#include "include.h"
#include <android/input.h>

Context::Commands::Commands(Context& context):
	mContext(context)
{

}

int inputCallback(int fd, int events, void* data)
{
	return 1;
}

//
// Command from main thread: the AInputQueue has changed.  Upon processing
// this command, android_app->inputQueue will be updated to the new queue
// (or NULL).
//
void Context::Commands::inputQueueChanged()
{
	auto inputQueue = mpApp->inputQueue;
}

//
// Command from main thread: a new ANativeWindow is ready for use.  Upon
// receiving this command, android_app->window will contain the new window
// surface.
//
void Context::Commands::windowInit()
{
	LOGV("windowInit()");
	mContext.createEGLSurface();

    mSensors.Init();
    mContext.onCreate(mpApp);
}

//
// Command from main thread: the existing ANativeWindow needs to be
// terminated.  Upon receiving this command, android_app->window still
// contains the existing window; after calling android_app_exec_cmd
// it will be set to NULL.
//
void Context::Commands::windowTerminating()
{
	LOGV("windowTerminating()");

	//TODO: Check whether cleanup in activityDestroy is correct
}

//
// Command from main thread: the current ANativeWindow has been resized.
// Please redraw with its new size.
//
void Context::Commands::windowResized()
{
	LOGV("windowResized()");

	auto window = mpApp->window;
	ANativeWindow_acquire(window);

	auto width  = ANativeWindow_getWidth (window);
	auto height = ANativeWindow_getHeight(window);
	auto format = ANativeWindow_getFormat(window);

	int sdlFormat = SDL_PIXELFORMAT_RGB565; // default
	switch (format) {
#if 0
	//TODO: Check why these are not in NDK but in Java
	case PixelFormat.A_8:
		LOGV("pixel format A_8"); break;
	case PixelFormat.LA_88:
		LOGV("pixel format LA_88"); break;
	case PixelFormat.L_8:
		LOGV("pixel format L_8"); break;
#endif
	case SDL_PIXELFORMAT_RGBA4444:
		LOGV("pixel format RGBA_4444"); break;
	case SDL_PIXELFORMAT_RGBA5551:
		LOGV("pixel format RGBA_5551"); break;
	case SDL_PIXELFORMAT_RGBA8888:
		LOGV("pixel format RGBA_8888"); break;
	case SDL_PIXELFORMAT_RGBX8888:
		LOGV("pixel format RGBX_8888"); break;
	case SDL_PIXELFORMAT_RGB332:
		LOGV("pixel format RGB_332"); break;
	case SDL_PIXELFORMAT_RGB565:
		LOGV("pixel format RGB_565"); break;
	case SDL_PIXELFORMAT_RGB888:
		LOGV("pixel format RGB_888"); break;
		// Not sure this is right, maybe SDL_PIXELFORMAT_RGB24 instead?
	default:
		LOGV("pixel format unknown " + format); break;
	}

	switch( format )
	{
		case SDL_PIXELFORMAT_RGBA4444:
		case SDL_PIXELFORMAT_RGBA5551:
		case SDL_PIXELFORMAT_RGBA8888:
		case SDL_PIXELFORMAT_RGBX8888:
		case SDL_PIXELFORMAT_RGB332:
		case SDL_PIXELFORMAT_RGB565:
		case SDL_PIXELFORMAT_RGB888:
			sdlFormat = format; // SDL_PIXELFORMAT_RGB888
			break;
	}

	mContext.onNativeResize(width, height, sdlFormat);

	ANativeWindow_release(window);

	LOGVF("Window size: %ix%i", width, height);

	mContext.startApp();
}

//
// Command from main thread: the system needs that the current ANativeWindow
// be redrawn.  You should redraw the window before handing this to
// android_app_exec_cmd() in order to avoid transient drawing glitches.
//
void Context::Commands::windowNeedsRedraw()
{

}

//
// Command from main thread: the content area of the window has changed,
// such as from the soft input window being shown or hidden.  You can
// find the new content rect in android_app::contentRect.
//
void Context::Commands::windowContentAreaChanged()
{

}


void Context::Commands::_activateInput()
{
	auto inputQueue = mpApp->inputQueue;
	AInputQueue_attachLooper(inputQueue, mpApp->looper,
	                         0, inputCallback,
	                         nullptr);
}

void Context::Commands::_deactivateInput()
{
	auto inputQueue = mpApp->inputQueue;
	AInputQueue_detachLooper(inputQueue);
}

//
// Command from main thread: the app's activity window has gained
// input focus.
//
void Context::Commands::activityFocus()
{
	_activateInput();
}

//
// Command from main thread: the app's activity window has lost
// input focus.
//
void Context::Commands::activityUnfocus()
{
	_deactivateInput();
}

//
// Command from main thread: the current device configuration has changed.
//
void Context::Commands::configurationChanged()
{

}

//
// Command from main thread: the system is running low on memory.
// Try to reduce your memory use.
//
void Context::Commands::systemMemoryLow()
{

}

//
// Command from main thread: the app's activity has been started.
//
void Context::Commands::activityStarted()
{
	_activateInput();
	mContext.onStart(mpApp);
}

//
// Command from main thread: the app's activity has been resumed.
//
void Context::Commands::activityResumed()
{
	_activateInput();
	mContext.onResume(mpApp);
}

//
// Command from main thread: the app should generate a new saved state
// for itself, to restore from later if needed.  If you have saved state,
// allocate it with malloc and place it in android_app.savedState with
// the size in android_app.savedStateSize.  The will be freed for you
// later.
//
void Context::Commands::stateSave()
{

}

//
// Command from main thread: the app's activity has been paused.
//
void Context::Commands::activityPaused()
{
	_deactivateInput();
	mSensors.Stop();
	mContext.onPause(mpApp);
}

//
// Command from main thread: the app's activity has been stopped.
//
void Context::Commands::activityStopped()
{
	_deactivateInput();
}

//
// Command from main thread: the app's activity is being destroyed,
// and waiting for the app thread to clean up and exit before proceeding.
//
void Context::Commands::activityDestroy()
{
	mContext.onDestroy(mpApp);
	delete &mContext;
}

// Key events
bool Context::Commands::keyInput(AInputEvent* pEvent)
{
	const auto action = AKeyEvent_getAction(pEvent);

	if( action == AKEY_EVENT_ACTION_DOWN )
		//Log.v("SDL", "key down: " + keyCode);
		mContext.onNativeKeyDown(pEvent);
	else if( action == AKEY_EVENT_ACTION_UP)
		//Log.v("SDL", "key up: " + keyCode);
		mContext.onNativeKeyUp(pEvent);
	else
		return false; // Unhandled

	return true;
}

// Touch events
bool Context::Commands::touchInput(AInputEvent* pEvent)
{
	const auto touchDevId = AInputEvent_getDeviceId(pEvent);
	const auto pointerCount = AMotionEvent_getPointerCount(pEvent);
	// touchId, pointerId, action, x, y, pressure
	const auto action = AMotionEvent_getAction(pEvent) & AMOTION_EVENT_ACTION_MASK;
	const auto actionPointerIndex = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
	const auto pointerFingerId = AMotionEvent_getPointerId(pEvent, actionPointerIndex);
	const float x;
	const float y;
	const float p;

	if (action == AMOTION_EVENT_ACTION_MOVE && pointerCount > 1) {
		// TODO send motion to every pointer if its position has
		// changed since prev event.
		for (int i = 0; i < pointerCount; i++) {
			pointerFingerId = AMotionEvent_getPointerId(pEvent, i);
			x =  AMotionEvent_getX(pEvent, i);
			y = AMotionEvent_getY(pEvent, i);
			p = AMotionEvent_getPressure(pEvent, i);
			mContext.onNativeTouch(touchDevId, pointerFingerId, action, x, y, p);
		}
	} else {
		x = AMotionEvent_getX(pEvent, actionPointerIndex);
		y = AMotionEvent_getY(pEvent, actionPointerIndex);
		p = AMotionEvent_getPressure(pEvent, actionPointerIndex);
		mContext.onNativeTouch(touchDevId, pointerFingerId, action, x, y, p);
	}

	return true;
}

