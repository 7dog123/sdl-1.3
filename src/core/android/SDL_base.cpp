
#include "include.h"
#include "SDL_base.h"
#include <android/log.h>

Base::Base(Context& context):
	mContext(context)
{
}

void Base::LOGV(const char* message)
{
	Log(ANDROID_LOG_VERBOSE, message);
}

void Base::LOGE(const char* message)
{
	Log(ANDROID_LOG_ERROR, message);
}

void Base::LOGW(const char* message)
{
	Log(ANDROID_LOG_WARN, message);
}

void Base::LOGI(const char* message)
{
	Log(ANDROID_LOG_INFO, message);
}

void Base::LOGV(const std::string& message)
{
	LOGV(message.c_str());
}

void Base::LOGE(const std::string& message)
{
	LOGE(message.c_str());
}

void Base::LOGW(const std::string& message)
{
	LOGW(message.c_str());
}

void Base::LOGI(const std::string& message)
{
	LOGI(message.c_str());
}

void Base::LOGVF(const char *fmt, ...)
{
	va_list vl;
	va_start(vl, fmt);
	Log(ANDROID_LOG_VERBOSE, fmt, vl);
	va_end(vl);
}

void Base::LOGEF(const char *fmt, ...)
{
	va_list vl;
	va_start(vl, fmt);
	Log(ANDROID_LOG_ERROR, fmt, vl);
	va_end(vl);
}

void Base::LOGWF(const char *fmt, ...)
{
	va_list vl;
	va_start(vl, fmt);
	Log(ANDROID_LOG_WARN, fmt, vl);
	va_end(vl);
}

void Base::LOGIF(const char *fmt, ...)
{
	va_list vl;
	va_start(vl, fmt);
	Log(ANDROID_LOG_INFO, fmt, vl);
	va_end(vl);
}

void Base::Log(int prio, const char* message)
{
	__android_log_print(prio, "SDL", message);
}

void Base::Log(int prio, const char* fmt, va_list ap)
{
	__android_log_vprint(prio, "SDL", fmt, ap);
}
