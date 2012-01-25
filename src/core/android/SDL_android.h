/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2012 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
#ifndef SDL_ANDROID_ANDROID_H_
#define SDL_ANDROID_ANDROID_H_
#include "SDL_config.h"
#include "SDL_asset.h"
#include "SDL_audio.h"

/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus


/* *INDENT-OFF* */
extern "C" {
/* *INDENT-ON* */
#endif


SDL_bool Android_CreateContext(int majorVersion, int minorVersion);
void Android_SwapWindow();
void Android_SetActivityTitle(const char *title);
SDL_bool Android_GetAccelerometerValues(float values[3]);

/* Ends C function definitions when using C++ */
#ifdef __cplusplus
/* *INDENT-OFF* */
}
/* *INDENT-ON* */
#endif

#endif // SDL_ANDROID_ANDROID_H_

/* vi: set ts=4 sw=4 expandtab: */
