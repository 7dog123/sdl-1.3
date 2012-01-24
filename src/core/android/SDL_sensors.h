
#ifndef SDL_ANDROID_SENSORS_H_
#define SDL_ANDROID_SENSORS_H_

#include <SDL.h>
#include <android/sensor.h>
#include "SDL_subsystem.h"

namespace SDL{ namespace Android{
class Context;

class Sensors : public Subsystem
{
public:
	Sensors(Context& context, android_app* app);
//extends SurfaceView implements SurfaceHolder.Callback,
//    View.OnKeyListener, View.OnTouchListener, SensorEventListener
	void enableSensor(ASensor& sensor, bool enabled);
	void onAccuracyChanged(ASensor& sensor, int accuracy);

	void Init();
	void Quit();
	void Stop();
	void Resume();
private:
	ALooper* mLooper;
    // Sensors
	ASensorManager* mSensorManager;

	ASensor* mpSensor;

	ASensorEventQueue& getQueue(int type);
	ASensorEventQueue* mpQueue;

	void onSensorChanged(ASensorEvent* pEvent);
};
}}

#endif // SDL_ANDROID_SENSORS_H_
