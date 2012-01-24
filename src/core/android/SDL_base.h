
#ifndef SDL_ANDROID_BASE_H_
#define SDL_ANDROID_BASE_H_

namespace SDL{ namespace Android{
class Base
{
protected:
	Base(Context& context);

	void LOGV(const char* message);
	void LOGE(const char* message);
	void LOGW(const char* message);

	void LOGV(const std::string& message);
	void LOGE(const std::string& message);
	void LOGW(const std::string& message);

	void LOGVF(const char *fmt, ...);
	void LOGEF(const char *fmt, ...);
	void LOGWF(const char *fmt, ...);

	Context& mContext;

private:
	void Log(int prio, const char* message);
	void Log(int prio, const char* fmt, va_list ap);
};
}}


#endif // SDL_ANDROID_BASE_H_
