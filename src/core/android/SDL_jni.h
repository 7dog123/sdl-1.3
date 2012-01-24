#ifndef SDL_ANDROID_JNI_H_
#define SDL_ANDROID_JNI_H_

#include <jni.h>

namespace SDL{ namespace Android{

class JNI
{
public:
	static bool HasExceptionOccurred();
protected:
	JNI(android_app* app);
	JNIEnv* mEnv;
	JavaVM* mJavaVM;
	// Main activity
	jclass mActivityClass;
};

}}

#endif // SDL_ANDROID_JNI_H_

