/*
 * SDL_assert.h
 *
 *  Created on: 22 Jan 2012
 *      Author: andreas
 */

#ifndef SDL_ANDROID_ASSERT_H_
#define SDL_ANDROID_ASSERT_H_

/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
/* *INDENT-OFF* */
extern "C" {
/* *INDENT-ON* */
#endif

#include "SDL_rwops.h"

int Android_FileOpen(SDL_RWops* ctx, const char* fileName, const char* mode);
long Android_FileSeek(SDL_RWops* ctx, long offset, int whence);
size_t Android_FileRead(SDL_RWops* ctx, void* buffer, size_t size, size_t maxnum);
size_t Android_FileWrite(SDL_RWops* ctx, const void* buffer, size_t size, size_t num);
int Android_FileClose(SDL_RWops* ctx);

/* Ends C function definitions when using C++ */
#ifdef __cplusplus
/* *INDENT-OFF* */
}
/* *INDENT-ON* */
#endif

#endif // SDL_ANDROID_ASSERT_H_
