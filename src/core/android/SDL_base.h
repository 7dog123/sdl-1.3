
#ifndef SDL_ANDROID_BASE_H_
#define SDL_ANDROID_BASE_H_

namespace SDL{ namespace Android{
class Context;

class Base
{
protected:
	Base(Context& context);

	static void LOGV(const char* message);
	static void LOGE(const char* message);
	static void LOGW(const char* message);
	static void LOGI(const char* message);

	static void LOGV(const std::string& message);
	static void LOGE(const std::string& message);
	static void LOGW(const std::string& message);
	static void LOGI(const std::string& message);

	static void LOGVF(const char *fmt, ...);
	static void LOGEF(const char *fmt, ...);
	static void LOGWF(const char *fmt, ...);
	static void LOGIF(const char *fmt, ...);

	Context& mContext;

private:
	static void Log(int prio, const char* message);
	static void Log(int prio, const char* fmt, va_list ap);
};
}}


#endif // SDL_ANDROID_BASE_H_
