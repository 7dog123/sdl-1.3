package org.libsdl.android;

public class Audio
extends Base
{
    private static AudioTrack mAudioTrack;
    
	// Audio
	private static Object buf;

	public static Object init(int sampleRate, boolean is16Bit, boolean isStereo, int desiredFrames) {
	    int channelConfig = isStereo ? AudioFormat.CHANNEL_CONFIGURATION_STEREO : AudioFormat.CHANNEL_CONFIGURATION_MONO;
	    int audioFormat = is16Bit ? AudioFormat.ENCODING_PCM_16BIT : AudioFormat.ENCODING_PCM_8BIT;
	    int frameSize = (isStereo ? 2 : 1) * (is16Bit ? 2 : 1);
	    
	    LOGV("SDL audio: wanted " + (isStereo ? "stereo" : "mono") + " " + (is16Bit ? "16-bit" : "8-bit") + " " + ((float)sampleRate / 1000f) + "kHz, " + desiredFrames + " frames buffer");
	    
	    // Let the user pick a larger buffer if they really want -- but ye
	    // gods they probably shouldn't, the minimums are horrifyingly high
	    // latency already
	    desiredFrames = Math.max(desiredFrames, (AudioTrack.getMinBufferSize(sampleRate, channelConfig, audioFormat) + frameSize - 1) / frameSize);
	    
	    mAudioTrack = new AudioTrack(AudioManager.STREAM_MUSIC, sampleRate,
	            channelConfig, audioFormat, desiredFrames * frameSize, AudioTrack.MODE_STREAM);
	    
	    audioStartThread();
	    
	    LOGV("SDL audio: got " + ((mAudioTrack.getChannelCount() >= 2) ? "stereo" : "mono") + " " + ((mAudioTrack.getAudioFormat() == AudioFormat.ENCODING_PCM_16BIT) ? "16-bit" : "8-bit") + " " + ((float)mAudioTrack.getSampleRate() / 1000f) + "kHz, " + desiredFrames + " frames buffer");
	    
	    if (is16Bit) {
	        buf = new short[desiredFrames * (isStereo ? 2 : 1)];
	    } else {
	        buf = new byte[desiredFrames * (isStereo ? 2 : 1)]; 
	    }
	    return buf;
	}
	
	public static void writeBuffer(short[] buffer) {
	    for (int i = 0; i < buffer.length; ) {
	        int result = mAudioTrack.write(buffer, i, buffer.length - i);
	        if (result > 0) {
	            i += result;
	        } else if (result == 0) {
	            try {
	                Thread.sleep(1);
	            } catch(InterruptedException e) {
	                // Nom nom
	            }
	        } else {
	            LOGW("SDL audio: error return from write(short)");
	            return;
	        }
	    }
	}
	
	public static void writeBuffer(byte[] buffer) {
	    for (int i = 0; i < buffer.length; ) {
	        int result = mAudioTrack.write(buffer, i, buffer.length - i);
	        if (result > 0) {
	            i += result;
	        } else if (result == 0) {
	            try {
	                Thread.sleep(1);
	            } catch(InterruptedException e) {
	                // Nom nom
	            }
	        } else {
	            LOGW("SDL audio: error return from write(short)");
	            return;
	        }
	    }
	}
	
	public static void quit() {
	    if (mAudioThread != null) {
	        try {
	            mAudioThread.join();
	        } catch(Exception e) {
	            LOGV("Problem stopping audio thread: " + e);
	        }
	        mAudioThread = null;
	
	        //Log.v("SDL", "Finished waiting for audio thread");
	    }
	
	    if (mAudioTrack != null) {
	        mAudioTrack.stop();
	        mAudioTrack = null;
	    }
	}
}
