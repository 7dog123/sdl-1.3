
#include "SDL_subsystem.h"
#include "include.h"

Subsystem::Subsystem(Context& context):
	Base(context)
{
}

Subsystem::~Subsystem()
{
	Quit();
}
