/**
 * \file LIV_CAPI.h
 * \brief Header containing the C API to communicate with the LIV App for rendering.
 * \copyright 2020-2022 LIV Inc. S.r.o.
 *
 * These are the function entry point to do operations between your application and the LIV App.
 *
 * These functions cannot be called without firstly calling LIV_Load(). Failing to do so will most likely cause a crash.
 */
#ifndef __LIV_CAPI
#define __LIV_CAPI

#include "LIV_Dynamic.h"
#include "LIV_BridgeDatastruct.h"

#ifdef __cplusplus
extern "C"
{
#else
#include <stdbool.h>
#endif


#ifdef LIV_NO_DX11
 /// <summary>
 /// If LIV_NO_DX11 is defined, just treat this as a void*, else, you need to include DX11 headers
 /// </summary>
typedef void ID3D11Device;
#endif

/// <summary>
/// Return true if LIV is currently attached to program
/// </summary>
/// <returns>State of LIV connection</returns>
/// <remarks>This function can only be called after successful `LIV_Load()`</remarks>
LIV_DECLSPEC bool LIV_IsActive();

#ifdef _WIN32
/// <summary>
/// Initialize MR rendering with DirectX 11
/// </summary>
/// <param name="device">Pointer to a DirectX 11 device that owns texture you will give us</param>
/// <remarks>This function can only be called after successful `LIV_Load()`</remarks>
/// <returns>True if DirectX mode was initialized</returns>
LIV_DECLSPEC bool LIV_D3D11_Init(ID3D11Device* device);
#endif

/// <summary>
/// Obtain data to render the new frame. Also serve as input for advanced features like pose-control.
/// </summary>
/// <param name="inputRequest">Pointer to an input frame. The function will only read from this pointer.</param>
/// <returns>Pointer to a **read only** input frame with the correct parameters to render current MR frame</returns>
/// <remarks>
/// - This function can only be called after successful `LIV_Load()`
///
/// - The returned pointer is owned by the LIV Bridge, you can read and copy the data to a locally owned InputFrame structure.
///
/// - Writing to this pointer is undefined behavior. Freeing this pointer may cause a crash. Pointer is only guaranteed to point to valid data for the current frame.
///
/// - inputRequest may contain a requested pose for the camera. In some cases (e.g: user is using an Avatar camera and not a physical one), the virtual camera could be moved by the game.
/// These are merely requests, and you must obey the rendering parameter from the returned object. Who controls each parameter is negotiated using the `priority` field of the struct.
///
/// </remarks>
LIV_DECLSPEC LIV_InputFrame* LIV_UpdateInputFrame(const LIV_InputFrame* inputRequest);

/// <summary>
/// Get a up-to-date InputFrame structure
/// </summary>
/// <param name="output">Where the new data will be written. Content of the structure will be overwritten</param>
/// <returns>true if a new inputFrame was acquired</returns>
/// <remarks>This function can only be called after successful `LIV_Load()`</remarks>
LIV_DECLSPEC bool LIV_GetInputFrame(LIV_InputFrame* output);

/// <summary>
/// Signal to LIV that you are ready to send frames. Call once before adding textures
/// </summary>
/// <remarks>This function can only be called after successful `LIV_Load()`</remarks>
LIV_DECLSPEC void LIV_Start();

/// <summary>
/// Add textures to the next frame to submit
/// </summary>
/// <param name="desc">pointer to a texture descriptor</param>
/// <remarks>
///  - This function can only be called after successful `LIV_Load()`
///  - In GL mode, desc is modified: an internal GL -> DX interop subsystem will use the provided resources to create a DirectX
/// texture that correspond to your inputs and feed it to the LIV Bridge. The `desc` pointer will contains the DirectX fields that are passed
/// to AddTexture, not your original input.
/// You should explicitly fill-in all the required fields of LIV_Texture before calling.
/// </remarks>
LIV_DECLSPEC void LIV_AddTexture(LIV_Texture* desc);

/// <summary>
/// Send your frame to LIV!
/// </summary>
/// <remarks>This function can only be called after successful `LIV_Load()`</remarks>
LIV_DECLSPEC void LIV_Submit();

/// <summary>
/// Recycle an input frame to be ready for new input
/// </summary>
/// <param name="input">pointer to input frame to clear</param>
LIV_DECLSPEC void LIV_ClearInputFrame(LIV_InputFrame* input);

/// <summary>
/// Get the requested resolution by LIV Compositor
/// </summary>
/// <param name="output">Output structure</param>
/// <returns>True if resolution was written to output successful</returns>
/// <remarks>This function can only be called after successful `LIV_Load()`</remarks>
LIV_DECLSPEC bool LIV_GetResolution(LIV_Resolution* output);

/// <summary>
/// Store a version number
/// </summary>
typedef struct _LIV_Version
{
	uint8_t major, minor, patch;
} LIV_Version;

/// <summary>
/// Obtain the version number of the LIV_Bridge DLL that was loaded by the SDK
/// </summary>
/// <param name="version">Output parameter, will write the version number in this structure</param>
/// <returns>Returns true if the bridge is loaded, and a version was written. False otherwise</returns>
LIV_DECLSPEC bool LIV_GetBridgeVersion(LIV_Version* version);

/// <summary>
///	Represent the applicaiton information. Mostly for internal usage by other LIV SDK
/// </summary>
///	<remarks>
///	Any unused field **MUST** be NULL
///
/// sdkID is the SDK fingerprint from the LIV Developer Portal
///	sdkVersion is a string like "1.5.1". A correct string is returned by LIV_GetFeatureVersion() in this library.
///	</remarks>
typedef struct _LIV_ApplicationInformation
{
	char* applicationName;
	char* applicationVersion;
	char* engineName;
	char* engineVersion;
	char* graphicsAPI;
	char* sdkID;
	char* sdkVersion;
	char* xrDeviceName;
} LIV_ApplicationInformation;

///Submit Application information to the LIV Bridge
LIV_DECLSPEC void LIV_SubmitApplicationInformation(const LIV_ApplicationInformation* information);

#ifdef __cplusplus
} // extern "C"
#endif
#endif