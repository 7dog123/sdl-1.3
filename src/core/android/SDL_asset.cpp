
#include "SDL_asset.h"
#include "SDL_context.h"
#include "include.h"

static int Android_FileOpen(SDL_RWops* ctx)
{
    int result = 0;

    const std::string& filename = *ctx->hidden.androidio.fileName;

    auto context = Context::GetContext();
    auto pAssetManager = context->GetAssets();

    int mode;
    auto pAsset = AAssetManager_open(pAssetManager, filename.c_str(), mode);

    if( pAsset )
    {
        ctx->hidden.androidio.pAsset = pAsset;
        ctx->hidden.androidio.size = AAsset_getLength(pAsset);
        ctx->hidden.androidio.position = 0;
        return 0;
    }
    else
    {
    	SDL_SetError("Could not open asset file '%s'", filename.c_str());
    	return -1;
    }
}

extern "C" int Android_FileOpen(SDL_RWops* ctx,
                                const char* fileName, const char*)
{
    if (!ctx) {
        return -1;
    }

    ctx->hidden.androidio.fileName = new std::string(fileName);

    return Android_FileOpen(ctx);
}

extern "C" size_t Android_FileRead(SDL_RWops* ctx, void* buffer,
        size_t size, size_t maxnum)
{
    int bytesRemaining = size * maxnum;
    int bytesRead = 0;

    while (bytesRemaining > 0) {

        int result = AAsset_read((AAsset*)ctx->hidden.androidio.pAsset, buffer, bytesRemaining);

        if (result < 0) {
            return 0;
        }

        bytesRemaining -= result;
        bytesRead += result;
        ctx->hidden.androidio.position += result;
    }

    return bytesRead / size;
}

extern "C" size_t Android_FileWrite(SDL_RWops* ctx, const void* buffer,
        size_t size, size_t num)
{
    SDL_SetError("Cannot write to Android package filesystem");
    return 0;
}

static int Android_FileClose(SDL_RWops* ctx, bool release)
{
    if (!ctx)
    	return 0;

	AAsset_close((AAsset*)ctx->hidden.androidio.pAsset);
	delete (std::string*)ctx->hidden.androidio.fileName;
	ctx->hidden.androidio.size = 0;
	ctx->hidden.androidio.position = -1;

	if (release) {
		SDL_FreeRW(ctx);
	}

    return 0;
}


extern "C" long Android_FileSeek(SDL_RWops* ctx, long offset, int whence)
{
    long newPosition;

    switch (whence) {
        case RW_SEEK_SET:
            newPosition = offset;
            break;
        case RW_SEEK_CUR:
            newPosition = ctx->hidden.androidio.position + offset;
            break;
        case RW_SEEK_END:
            newPosition = ctx->hidden.androidio.size + offset;
            break;
        default:
            SDL_SetError("Unknown value for 'whence'");
            return -1;
    }
    if (newPosition < 0) {
        newPosition = 0;
    }
    if (newPosition > ctx->hidden.androidio.size) {
        newPosition = ctx->hidden.androidio.size;
    }

    long movement = newPosition - ctx->hidden.androidio.position;

    if (movement > 0) {
        unsigned char buffer[1024];

        // The easy case where we're seeking forwards
        while (movement > 0) {
            long amount = (long) sizeof (buffer);
            if (amount > movement) {
                amount = movement;
            }
            size_t result = Android_FileRead(ctx, buffer, 1, amount);

            if (result <= 0) {
                // Failed to read/skip the required amount, so fail
                return -1;
            }

            movement -= result;
        }
    } else if (movement < 0) {
        // We can't seek backwards so we have to reopen the file and seek
        // forwards which obviously isn't very efficient
        Android_FileClose(ctx, false);
        Android_FileOpen(ctx);
        Android_FileSeek(ctx, newPosition, RW_SEEK_SET);
    }

    ctx->hidden.androidio.position = newPosition;

    return ctx->hidden.androidio.position;
}

extern "C" int Android_FileClose(SDL_RWops* ctx)
{
    return Android_FileClose(ctx, true);
}
