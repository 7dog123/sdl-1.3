
#include "SDL_sensors.h"
#include "include.h"

// Startup
Sensors::Sensors(Context& context, android_app* app):
	Subsystem(context),
	mLooper(app->looper),
	mpSensor(nullptr)
{
	mSensorManager = ASensorManager_getInstance();
}

void Sensors::Init()
{
    // TODO: This uses getDefaultSensor - what if we have >1 accels?
    mpSensor = ASensorManager_getDefaultSensor(mSensorManager, ASENSOR_TYPE_ACCELEROMETER);
    enableSensor(*mpSensor, true);
}

void Sensors::Quit()
{
	Stop();
	mpSensor = nullptr;
}

void Sensors::Stop()
{
	if( !mpSensor )
		return;

	enableSensor(*mpSensor, false);
}

void Sensors::Resume()
{
	//TODO: Implement
}

ASensorEventQueue& Sensors::getQueue(int type)
{
	if( !mpQueue )
		mpQueue = ASensorManager_createEventQueue
		(mSensorManager, mLooper, 0, nullptr, nullptr);

	return *mpQueue;
}

// Sensor events
void Sensors::enableSensor(ASensor& sensor, bool enabled)
{
	auto sensorType = ASensor_getType(&sensor);
	auto queue = getQueue(sensorType);

	int result;
	if (enabled) {
		result = ASensorEventQueue_enableSensor(&queue, &sensor);
	} else {
		result = ASensorEventQueue_disableSensor(&queue, &sensor);
	}
}

void Sensors::onAccuracyChanged(ASensor& sensor, int accuracy)
{
	// TODO
}

void Sensors::onSensorChanged(ASensorEvent* pEvent)
{
	if( pEvent->type == ASENSOR_TYPE_ACCELEROMETER ) {
		const auto GRAV = ASENSOR_STANDARD_GRAVITY;
		mContext.onNativeAccel(pEvent->acceleration.x / GRAV,
		                       pEvent->acceleration.y / GRAV,
		                       pEvent->acceleration.z / GRAV);
	}
}

