/* sokol implementations need to live in it's own source file, because
on MacOS and iOS the implementation must be compiled as Objective-C, so there
must be a *.m file on MacOS/iOS, and *.c file everywhere else
*/
#define SOKOL_IMPL
#if defined(_WIN32)
#include <Windows.h>
#define SOKOL_LOG(s) OutputDebugStringA(s)
#endif
/* sokol 3D-API defines are provided by build options */
#if defined(SOKOL_GLES2)
#include <gles2/gl2.h>
#endif
#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_time.h"
#include "sokol_audio.h"
#include "sokol_args.h"

