
#include "SDL_jni.h"
#include "include.h"
#include <jni.h>

JNI::JNI(android_app* app):
	mEnv(app->activity->env),
	mJavaVM(app->activity->vm),
	mActivityClass((jclass)mEnv->NewGlobalRef(app->activity->clazz))
{

}

// Test for an exception and call SDL_SetError with its detail if one occurs
bool JNI::HasExceptionOccurred()
{
    jthrowable exception = mEnv->ExceptionOccurred();
    if (exception != nullptr) {
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

        if (exceptionMessage != nullptr) {
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

