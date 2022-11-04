/**
 * \file LIV.h
 * \brief Main entry point of the LIV SDK
 * \copyright 2020-2022 LIV Inc. S.r.o.
 *
 * This is the one and only file you actually need to include to use the LIV SDK.
 * It contains the API of the LIV Loader, and it will include all the necessary files to the C API,
 * data-structures and utility functions you'll need.
 */

#ifndef __LIV_H
#define __LIV_H

#include "LIV_Dynamic.h"
#include "LIV_CAPI.h"

#ifdef __cplusplus
extern "C"
{
#else
#include <stdbool.h>
#endif

	/// <summary>
	/// Load LIV Bridge functions
	/// </summary>
	///
	/// <returns>
	/// Is true upon success. Call liv_get_error() to know what went wrong otherwise.
	/// </returns>
	/// <remarks>This call is specifically not thread safe.
	/// A race condition can occur with other code invoking the dynamic loader of the operating system
	/// (LoadLibrary() or dlopen()). Exact behavior of this call is platform dependent.
	/// </remarks>
	LIV_DECLSPEC bool LIV_Load();

	/// <summary>
	/// Cleanup LIV Bridge functions
	/// </summary>
	///
	/// <remarks>
	/// This call is not thread safe.
	/// Calling this function without having called LIV_Load(), and received a successful state is undefined behavior.
	/// This call invalidates internal state of the LIV loader. Calling other LIV functions after this will fail,
	/// unless you call LIV_Load() again.
	/// </remarks>
	LIV_DECLSPEC void LIV_Unload();

	/// <summary>
	/// Get error message describing why something went wrong in the loader.
	/// This also "clear" the error state to "no error" after call.
	///
	/// The returned string is static and does't need to be freed.
	/// </summary>
	/// <returns>
	/// A null terminated character string describing the error. You can log that!
	/// </returns>
	LIV_DECLSPEC const char* LIV_GetError();

	/// <summary>
	/// Get a 64bit tag from a string that the LIV Bridge recognize
	///
	/// Useful string tags:
	/// - `"BGCTEX"` Background Compositor texture
	/// - `"FGCTEX"` Foreground Compositor texture
	/// </summary>
	///
	/// <param name="str">string identifier</param>
	/// <returns>The corresponding tag</returns>
	LIV_DECLSPEC uint64_t LIV_Tag(const char* str);

	//TODO document these better:

	/// <summary>
	/// Get the LIV Feature version string
	/// </summary>
	///
	/// <returns>Static string, do not free it.</returns>
	LIV_DECLSPEC const char* LIV_GetFeatureVersion();

	/// <summary>
	/// Get the LIV Native SDK version string
	/// </summary>
	///
	/// <returns>Static string, do not free it</returns>
	LIV_DECLSPEC const char* LIV_GetCSDKVersion();

	/// <summary>
	/// Debug feature for development. Tell the LIV app that the current process should be manually captured.
	/// Useful when integrating LIV to a environment/level editor. **Do not use in a game release**
	/// </summary>
	LIV_DECLSPEC void LIV_RequestCapture();

#ifdef __cplusplus
} // extern "C"
#endif

#endif // __LIV_H

//TODO This is hard to edit and maintain move this to an external markdown file or something similar
