
#ifndef SDL_ANDROID_CONTEXT_H_
#define SDL_ANDROID_CONTEXT_H_

#include "SDL_base.h"
namespace SDL{ namespace Android {
class Context : public Base
{
public:
	Context();
    static Context* mSingleton;

};
}}


#endif // SDL_ANDROID_CONTEXT_H_
