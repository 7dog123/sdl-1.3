#include "SDL_audio_osles.h"
#include "include.h"

OSLES::OSLES(Context& context) :
	Audio(context),
	mAudioThread(nullptr),
	mAbortThread(false)
{
}

size_t getMinBufferSize(size_t sampleRate, Uint8 channelCount, Uint8 bits)
{
	return sampleRate * channelCount * bits / 8;
}

int OSLES::Init(int sampleRate, bool is16Bit, Uint8 channelCount, int desiredFrames)
{
	LOGV("SDL audio: opening device");

	//TODO: Handle multiple Init calls

	auto isStereo = channelCount >= 2 ? true : false;

	mIs16Bit = is16Bit;
	mIsStereo = isStereo;

	const auto audioFormat = mIs16Bit ?
		SL_PCMSAMPLEFORMAT_FIXED_16 :
		SL_PCMSAMPLEFORMAT_FIXED_8;
	const auto frameSize = (isStereo ? 2 : 1) * (mIs16Bit ? 2 : 1);

	LOGVF("SDL audio: wanted %s %s %d kHz, %i frames buffer",
	    (isStereo ? "stereo" : "mono"),  (mIs16Bit ? "16-bit" : "8-bit"),
	    ((float)sampleRate / 1000.0f), desiredFrames);

	auto min =
	(
		getMinBufferSize(sampleRate, isStereo ? 2 : 1, audioFormat)
		+ frameSize - 1
	) / frameSize;

	// Let the user pick a larger buffer if they really want -- but ye
	// gods they probably shouldn't, the minimums are horrifyingly high
	// latency already
	desiredFrames = std::max<size_t>(desiredFrames, min);

    mBufferQueueLocator.locatorType = SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE;
    mBufferQueueLocator.numBuffers = 1;

#define SL_BYTEORDER_NATIVE \
    ((SDL_BYTEORDER == SDL_BIG_ENDIAN) ? SL_BYTEORDER_BIGENDIAN : SL_BYTEORDER_LITTLEENDIAN)

   	mSourceFormat.bitsPerSample = mIs16Bit ? SL_PCMSAMPLEFORMAT_FIXED_16 :
   	                                         SL_PCMSAMPLEFORMAT_FIXED_8;
   	mSourceFormat.containerSize = mSourceFormat.bitsPerSample;
   	mSourceFormat.channelMask   = SL_SPEAKER_FRONT_CENTER;
    mSourceFormat.endianness    = SL_BYTEORDER_NATIVE;
   	mSourceFormat.formatType    = SL_DATAFORMAT_PCM;
   	mSourceFormat.numChannels   = isStereo ? 2 : 1;
   	mSourceFormat.samplesPerSec = sampleRate;

    mDataSource.pFormat  = &mSourceFormat;
    mDataSource.pLocator = &mBufferQueueLocator;

    mOutputMixLocator.locatorType = SL_DATALOCATOR_OUTPUTMIX;
    mOutputMixLocator.outputMix   = &mOutputMix;

    mDataSink.pFormat  = nullptr;
    mDataSink.pLocator = &mOutputMixLocator;


	// create engine
    auto result = slCreateEngine(&mEngineObject, 0, nullptr, 0, nullptr, nullptr);

    if( hasFailed(result) )
    {
    	LOGW("SDL audio: didn't get back a good audio buffer!");
    	return -1;
    }

	try
	{
		throwResult(mEngineObject->Realize(mEngineObject, SL_BOOLEAN_FALSE));
		throwResult(mEngineObject->GetInterface(mEngineObject, SL_IID_ENGINE, &mEngine));
		throwResult(mEngine->CreateOutputMix(mEngine, &mOutputMix, 1, nullptr, nullptr));
		throwResult(mEngine->CreateAudioPlayer(mEngine, &mPlayerObject, &mDataSource, &mDataSink, 1, nullptr, nullptr));

		try
		{
			throwResult(mPlayerObject->Realize(mPlayerObject, SL_BOOLEAN_FALSE));
			throwResult(mPlayerObject->GetInterface(mPlayerObject, SL_IID_PLAY, &mPlayer));
			throwResult(mPlayerObject->GetInterface(mPlayerObject, SL_IID_BUFFERQUEUE, &mBufferQueue));
		}
		catch( std::exception& e )
		{
			mPlayerObject->Destroy(mPlayerObject);
		}
    }
    catch( std::exception& e )
    {
    	mEngineObject->Destroy(mEngineObject);
    	LOGW("SDL audio: didn't get back a good audio buffer!");
    	return -1;
    }
/*
        mAudioTrack = new AudioTrack(AudioManager.STREAM_MUSIC, sampleRate,
                channelConfig, audioFormat, desiredFrames * frameSize, AudioTrack.MODE_STREAM);
*/
	StartThread();

	LOGVF("SDL audio: got %s %s %d kHz, %i frames buffer",
	    (isStereo ? "stereo" : "mono"),  (mIs16Bit ? "16-bit" : "8-bit"),
	    ((float)sampleRate / 1000.0f), desiredFrames);

	mAudioBufferByteSize = desiredFrames * (isStereo ? 2 : 1);

	if( mIs16Bit )
		mAudioBuffer = new short[mAudioBufferByteSize];
	else
		mAudioBuffer = new Sint8[mAudioBufferByteSize];

    return desiredFrames;
};

void* OSLES::GetBuffer()
{
	return mAudioBuffer;
};

void OSLES::Play()
{
	auto result = mPlayer->SetPlayState(mPlayer, SL_PLAYSTATE_PLAYING);
}

void OSLES::Quit()
{
	StopThread();
		//FIXME: enable error reporting again
		//Log.v("SDL", "Problem stopping audio thread: " + e);

		//Log.v("SDL", "Finished waiting for audio thread");

	auto result = mBufferQueue->Clear(mBufferQueue);
	auto error = getMessage(result);

	if( !error.empty() )
	{
		LOGW("Cleaning up buffer queue failed: " + error);
		return;
	}

	result = mPlayer->SetPlayState(mPlayer, SL_PLAYSTATE_STOPPED);
	error = getMessage(result);

	if( !error.empty() )
	{
		LOGW("Player could not be stopped: " + error);
		return;
	}

	result = mPlayerObject->Destroy(mPlayerObject);
	error = getMessage(result);

	if( !error.empty() )
	{
		LOGW("Destroying Player failed: " + error);
		return;
	}


	result = mEngineObject->Destroy(mEngineObject);
	error = getMessage(result);

	if( !error.empty() )
	{
		LOGW("Destroying Engine failed: " + error);
		return;
	}

	if( mAudioBuffer )
	{
		if( mIs16Bit )
			delete[] (short*)mAudioBuffer;
		else
			delete[] (Sint8*)mAudioBuffer;

		mAudioBuffer = nullptr;
	}

	mAudioBufferByteSize = 0;
}

void OSLES::Stop()
{
	auto result = mPlayer->SetPlayState(mPlayer, SL_PLAYSTATE_PAUSED);
}

void OSLES::Resume()
{
	Play();
}

void OSLES::WriteBuffer()
{
	if( mIs16Bit )
		WriteBuffer<short>((short*)mAudioBuffer, mAudioBufferByteSize);
	else
		WriteBuffer<Sint8>((Sint8*)mAudioBuffer, mAudioBufferByteSize);
};

bool OSLES::hasFailed(SLresult result)
{
	return result != SL_RESULT_SUCCESS;
}

std::string&& OSLES::getMessage(SLresult result)
{
	switch( result )
	{
	case SL_RESULT_SUCCESS:
		return std::string;
	case SL_RESULT_PRECONDITIONS_VIOLATED:
		return "Preconditions were violated";
	case SL_RESULT_PARAMETER_INVALID:
		return "Parameter is invalid";
	case SL_RESULT_MEMORY_FAILURE:
		return "Memory is failure";
	case SL_RESULT_RESOURCE_ERROR:
		return "Resource error";
	case SL_RESULT_RESOURCE_LOST:
		return "Lost the resource";
	case SL_RESULT_IO_ERROR:
		return "IO Error";
	case SL_RESULT_BUFFER_INSUFFICIENT:
		return "Buffer is insufficient";
	case SL_RESULT_CONTENT_CORRUPTED:
		return "Content is corrupted";
	case SL_RESULT_CONTENT_UNSUPPORTED:
		return "Content is unsupported";
	case SL_RESULT_CONTENT_NOT_FOUND:
		return "Content was not found";
	case SL_RESULT_PERMISSION_DENIED:
		return "Permission was denied";
	case SL_RESULT_FEATURE_UNSUPPORTED:
		return "Feature is unsupported";
	case SL_RESULT_INTERNAL_ERROR:
		return "Internal error occurred";
	case SL_RESULT_UNKNOWN_ERROR:
		return "Unknown error occurred";
	case SL_RESULT_OPERATION_ABORTED:
		return "Operation was aborted";
	case SL_RESULT_CONTROL_LOST:
		return "Control was lost";
	default:
		return "Unknown failure";
	}
}

void OSLES::throwResult(SLresult result)
{
	std::string& error = getMessage(result);

	if( !error.empty() )
		throw std::runtime_error(error);
}

void OSLES::Run(std::function<void ()>func)
{
	Play();
	func();
}




