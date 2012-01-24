
#ifndef SDL_ANDROID_SUBSYSTEM_H_
#define SDL_ANDROID_SUBSYSTEM_H_

#include "SDL_base.h"
#include "SDL_context.h"

namespace SDL{ namespace Android{
class Subsystem : public Base
{
public:
	virtual void Quit() = 0;
	virtual void Stop() = 0;
	virtual void Resume() = 0;
protected:
	Subsystem(Context& context);
	virtual ~Subsystem();
};
}}


#endif // SDL_ANDROID_SUBSYSTEM_H_
