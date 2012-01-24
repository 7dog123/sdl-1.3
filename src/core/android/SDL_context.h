
#ifndef SDL_ANDROID_CONTEXT_H_
#define SDL_ANDROID_CONTEXT_H_

#include "SDL_base.h"
namespace SDL{ namespace Android {
class Context : public Base
{
public:
	Context();
private:

    static Context* mSingleton;


    class Commands : public Base
    {
    public:
    	Commands(Context& context);

        void inputQueueChanged();
        void windowInit();
        void windowTerminating();
        void windowResized();
        void windowNeedsRedraw();
        void windowContentAreaChanged();
        void activityFocus();
        void activityUnfocus();
        void configurationChanged();
        void systemMemoryLow();
        void activityStarted();
        void activityResumed();
        void stateSave();
        void activityPaused();
        void activityStopped();
        void activityDestroy();

        bool keyInput(AInputEvent* event);
        bool touchInput(AInputEvent* event);

        android_app* mpApp;
    private:
        Context& mContext;
        void _activateInput();
        void _deactivateInput();
    };

    Commands mCommands;

};
}}


#endif // SDL_ANDROID_CONTEXT_H_
