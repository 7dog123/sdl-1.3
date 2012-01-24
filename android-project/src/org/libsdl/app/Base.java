package org.libsdl.android;

class Base
{
	protected static void LOGV(string message)
	{
		Log.v("SDL", message);
	}
    
	protected static void LOGW(string message)
	{
		Log.w("SDL", message);
	}
}