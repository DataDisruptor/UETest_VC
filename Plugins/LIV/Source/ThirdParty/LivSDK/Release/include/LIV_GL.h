/**
 * \file LIV_GL.h
 * \brief Header containing OpenGL utilities to communicate with LIV
 * \copyright 2020-2022 LIV Inc. S.r.o.
 */

#ifndef __LIV_GL
#define __LIV_GL

#include "LIV_BridgeDatastruct.h"
#include "LIV_CAPI.h"

#ifdef __cplusplus
extern "C"
{
#else
#include <stdbool.h>
#endif


/// <summary>
/// Convert a Vector3 to OpenGL space
/// </summary>
/// <param name="input">Vector in LIV space</param>
/// <returns>Vector in OpenGL default space</returns>
LIV_DECLSPEC LIV_Vector3 LIV_Vector3_GL(LIV_Vector3 input);

/// <summary>
/// Convert a Quaternion to OpenGL space
/// </summary>
/// <param name="input">Quaternion in LIV space</param>
/// <returns>Quaternion in OpenGL space</returns>
LIV_DECLSPEC LIV_Quaternion LIV_Quaternion_GL(LIV_Quaternion input);

/// <summary>
/// Convert a model matrix to OpenGL space
/// </summary>
/// <param name="input">Model matrix in LIV Space</param>
/// <returns>Model matrix in default OpenGL Space</returns>
LIV_DECLSPEC LIV_Matrix4x4 LIV_Matrix4x4_GL(const LIV_Matrix4x4* input);

/// <summary>
/// Initialize the OpenGL mode.
/// </summary>
/// <returns>
/// True upon successful initialization.
/// Cannot use the LIV_GL helper if that function was not called properly
/// </returns>
/// <remarks>
/// Can only be called with an already created and made current OpenGL context.
/// Will fail if driver do not support the WGL_NV_DX_interop extension.
/// If this call returns true, LIV_AddTexture() expect you to have filled in the LIV_Texture::gl_textureName
/// field with a valid OpenGL texture name instead of a DirectX11 texture pointer
/// </remarks>
LIV_DECLSPEC bool LIV_GL_Init();

/// <summary>
/// Cleanup everything in the OpenGL subsystem
/// </summary>
/// <remarks>LIV_Unload() will call this function for you! It also
/// can only be called after LIV_GL_Init() was successfully called.</remarks>
LIV_DECLSPEC void LIV_GL_Quit();

/// <summary>
/// Set an internal flag to force the usage of the OpenGL 3.0 code path
/// </summary>
LIV_DECLSPEC void LIV_GL_ForceFBO();

/// <summary>
/// Signal that the application is requesting it's textures to be converted to 8bit, instead of submitting regular RGBA textures to the LIV compositor.
/// </summary>
/// <remarks>
/// Must be called *after* a successful LIV_GL_Init() and *before* submitting any texture!
/// Please set `gl_pixelFormat` to `GL_RGBA` when calling LIV_AddTexture().
/// </remarks>
/// <returns>
/// True if this setting was applied properly.
/// </returns>
LIV_DECLSPEC bool LIV_GL_AutoConvertTo8Bit();

#ifdef __cplusplus
} // extern "C"
#endif
#endif
