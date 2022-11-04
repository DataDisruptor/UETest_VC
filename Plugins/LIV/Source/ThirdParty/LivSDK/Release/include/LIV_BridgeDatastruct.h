/**
 * \file LIV_BridgeDatastruct.h
 * \copyright 2020-2022 LIV Inc. S.r.o.
 *
 * Define the data structures used as input/output on the "bridge":
 * the communication system between LIV runtime services and client applications
 */

#ifndef _LIV_SDK_STRUCT
#define _LIV_SDK_STRUCT

#ifndef __cplusplus
#include <stdint.h>
#else
#include <cstdint>
#endif

#ifndef _WIN32 //directx is only available on Windows
#define LIV_NO_DX11
#endif

#ifndef LIV_NO_DX11
#include <d3d11.h>
#include <dxgi.h>
#endif

#ifdef __cplusplus
#define LIV_64BIT_ENUM : unsigned long long
#define LIV_32BIT_ENUM : int
#else
#define LIV_64BIT_ENUM
#define LIV_32BIT_ENUM
#endif

/// The signed byte priority tag to be fed in LIV_InputFrame priority
#define LIV_GAME_PRIORITY ((signed char)(63))

/// Helper macro to define bitflag values
#define LIV_BIT(n) (1ULL << n)

typedef enum LIV_SDKTYPE
{
	LIV_SDKTYPE_Freespace = 0, // These objects are part of the free space pool
	LIV_SDKTYPE_String = 1,
	LIV_SDKTYPE_Buffer = 2,
	LIV_SDKTYPE_SharedHandle = 3,
	LIV_SDKTYPE_NOJSON = 4,
	LIV_SDKTYPE_Integer = 5,
	LIV_SDKTYPE_TextureDescriptor = 8,
	LIV_SDKTYPE_TextureBuffer = 9,
	LIV_SDKTYPE_CameraPosition = 10,
	LIV_SDKTYPE_PlayerPosition = 11,
	LIV_SDKTYPE_ControllerPosition = 12,
	LIV_SDKTYPE_Texture = 13,
	LIV_SDKTYPE_Pose = 14,
	LIV_SDKTYPE_trackedspace = 15,
	LIV_SDKTYPE_Frame = 16,
	LIV_SDKTYPE_Source = 17,
	LIV_SDKTYPE_SharedOpenGL = 18,
	LIV_SDKTYPE_Controller = 19,
	LIV_SDKTYPE_InputFramee = 20,
	LIV_SDKTYPE_OutputFrame = 21,
	LIV_SDKTYPE_CalibrationState = 22,
	LIV_SDKTYPE_SDKVector2 = 23,
	LIV_SDKTYPE_SDKVector3 = 24,
	LIV_SDKTYPE_SDKQuaternion = 25,
	LIV_SDKTYPE_SDKMatrix4x4 = 26,
	LIV_SDKTYPE_Float = 27,
} LIV_SDKTYPE;

typedef enum LIV_COMPOSITOR_FEATURES LIV_64BIT_ENUM
{
	LIV_COMPOSITOR_FEATURES_NONE = 0, // Default - this field is not used or is undefined outside compositor bridge
	LIV_COMPOSITOR_FEATURES_DISABLED = 0, // Compositor is inactive - treat like old shm flag
	LIV_COMPOSITOR_FEATURES_ACTIVE = LIV_BIT(0), // Compositor is active
	LIV_COMPOSITOR_FEATURES_INITIALIZING = LIV_BIT(1), // Not yet used - will eventually be used as part of initialization when SDK_BRIDGE is deprecated
	LIV_COMPOSITOR_FEATURES_RESERVED1 = LIV_BIT(2), // Will likely be used for compositor state like flags
	LIV_COMPOSITOR_FEATURES_DISABLE_TELEMETRY = LIV_BIT(3), // Enable telemetry logging - unimplmented
	LIV_COMPOSITOR_FEATURES_SYNC_LIMIT = LIV_BIT(4), // Limit the rendering frame rate to the rate requested by app - unimplmented
	LIV_COMPOSITOR_FEATURES_ENABLE_VIEWFINDER = LIV_BIT(5), // unimplemented
	LIV_COMPOSITOR_FEATURES_DISABLE_VIVR = LIV_BIT(6), // unimplemented
	LIV_COMPOSITOR_FEATURES_DISABLE_BLOOM_EFFECTS = LIV_BIT(7), // Disable (expensive) bloom effects if present - unimplemented
	LIV_COMPOSITOR_FEATURES_DISABLE_BACKGROUND = LIV_BIT(12), // Disable background texture rendering (and ideally source) Compositor bit
	LIV_COMPOSITOR_FEATURES_DISABLE_FOREGROUND = LIV_BIT(13), // Disable foreground texture rendering (and ideally source)
	LIV_COMPOSITOR_FEATURE_FREEZE_COMPOSITION = LIV_BIT(15), // Stop getting updates from camera tracker

	// Source flags - flags related to sources other than (but possibly including?) compositor
	LIV_COMPOSITOR_FEATURES_LEGACY_SOURCE = LIV_BIT(16), // This source is a legacy 1.0 source
	LIV_COMPOSITOR_FEATURES_CAMERA_SOURCE = LIV_BIT(17), // Should we do this? from a debugging perspective, this is very useful - but not sure if this is the place for it.
	LIV_COMPOSITOR_FEATURES_VIVR_SOURCE = LIV_BIT(18), // This is VIVR
	LIV_COMPOSITOR_FEATURES_GAME_SOURCE = LIV_BIT(19), // This is a game

	LIV_COMPOSITOR_FEATURES_ENABLE_TESTCODE = LIV_BIT(47),
	LIV_COMPOSITOR_FEATURES_TEST_CLIPPLANE = LIV_BIT(48),
	LIV_COMPOSITOR_FEATURES_TEST_ORBITCAMERA = LIV_BIT(49),
	LIV_COMPOSITOR_FEATURES_TEST_WOBBLE = LIV_BIT(50) // Wobble test

} LIV_COMPOSITOR_FEATURES;

typedef enum LIV_FEATURES LIV_64BIT_ENUM
{
	LIV_FEATURES_UNDEFINED = 0,
	LIV_FEATURES_BACKGROUND_RENDER = LIV_BIT(0),
	LIV_FEATURES_FOREGROUND_RENDER = LIV_BIT(1),
	LIV_FEATURES_BOTH_RENDER = 3,
	LIV_FEATURES_COMPLEX_CLIP_PLANE = LIV_BIT(2),
	LIV_FEATURES_BACKGROUND_DEPTH_RENDER = LIV_BIT(3),
	LIV_FEATURES_OVERRIDE_POST_PROCESSING = LIV_BIT(4),
	LIV_FEATURES_FIX_FOREGROUND_ALPHA = LIV_BIT(5),
	LIV_FEATURES_GROUND_CLIP_PLANE = LIV_BIT(6),

	LIV_FEATURES_UPDATE_POSE = LIV_BIT(16),
	LIV_FEATURES_UPDATE_CLIPPLANE = LIV_BIT(17),
	LIV_FEATURES_UPDATE_STAGE = LIV_BIT(18),
	LIV_FEATURES_UPDATE_RESOLUTION = LIV_BIT(19),
	LIV_FEATURES_UPDATE_ALL = LIV_FEATURES_UPDATE_POSE | LIV_FEATURES_UPDATE_STAGE | LIV_FEATURES_UPDATE_RESOLUTION | LIV_FEATURES_UPDATE_CLIPPLANE,

	LIV_FEATURES_POSE_UPDATED = LIV_FEATURES_UPDATE_POSE << 8, // 24
	LIV_FEATURES_CLIPPLANE_UPDATED = LIV_FEATURES_UPDATE_CLIPPLANE << 8, // 25
	LIV_FEATURES_STAGE_UPDATED = LIV_FEATURES_UPDATE_STAGE << 8, // 26
	LIV_FEATURES_RESOLUTION_UPDATED = LIV_FEATURES_UPDATE_RESOLUTION << 8, // 27

	LIV_FEATURES_DEBUG_CLIP_PLANE = LIV_BIT(48)
} LIV_FEATURES;

typedef enum LIV_TEXTURE_ID LIV_32BIT_ENUM
{
	LIV_TEXTURE_UNDEFINED = 0,
	LIV_TEXTURE_BACKGROUND_COLOR_BUFFER_ID = 10,
	LIV_TEXTURE_BACKGROUND_DEPTH_BUFFER_ID = 11,
	LIV_TEXTURE_BACKGROUND_NORMALS_BUFFER_ID = 12,
	LIV_TEXTURE_BACKGROUND_MOTION_VECTORS_BUFFER_ID = 13,
	LIV_TEXTURE_FOREGROUND_COLOR_BUFFER_ID = 20,
	LIV_TEXTURE_FOREGROUND_DEPTH_BUFFER_ID = 21,
	LIV_TEXTURE_FOREGROUND_NORMALS_BUFFER_ID = 22,
	LIV_TEXTURE_FOREGROUND_MOTION_VECTORS_BUFFER_ID = 23,
	LIV_TEXTURE_UI_COLOR_BUFFER_ID = 30,
	LIV_TEXTURE_LIGHT_PROBE_COLOR_BUFFER_ID = 40,
	LIV_TEXTURE_CUBE_PROBE_COLOR_BUFFER_ID = 50,
	LIV_TEXTURE_CUBE_PROBE_DEPTH_BUFFER_ID = 51
} LIV_TEXTURE_ID;

typedef enum LIV_TEXTURE_TYPE LIV_32BIT_ENUM
{
	LIV_TEXTURE_TYPE_UNDEFINED = 0,
	LIV_TEXTURE_TYPE_COLOR_BUFFER = 1,
	LIV_TEXTURE_TYPE_DEPTH_BUFFER = 2,
	LIV_TEXTURE_TYPE_NORMAL_BUFFER = 3,
	LIV_TEXTURE_TYPE_MOTION_VECTOR_BUFFER = 4
} LIV_TEXTURE_TYPE;

typedef enum LIV_TEXTURE_FORMAT LIV_32BIT_ENUM
{
	LIV_TEXTURE_FORMAT_UNDEFINED = 0,
	LIV_TEXTURE_FORMAT_ARGB32 = 10,
	LIV_TEXTURE_FORMAT_ARGB64 = 11,
	LIV_TEXTURE_FORMAT_ARGBHalf = 12,
	LIV_TEXTURE_FORMAT_ARGBFloat = 13,
	LIV_TEXTURE_FORMAT_DEPTH16 = 20,
	LIV_TEXTURE_FORMAT_DEPTH24_STENCIL = 21,
	LIV_TEXTURE_FORMAT_DEPTH32_STENCIL = 22,
} LIV_TEXTURE_FORMAT;

typedef enum LIV_TEXTURE_DEVICE LIV_32BIT_ENUM
{
	LIV_TEXTURE_DEVICE_UNDEFINED = 0,
	LIV_TEXTURE_DEVICE_RAW = 1,
	LIV_TEXTURE_DEVICE_DIRECTX = 2,
	LIV_TEXTURE_DEVICE_OPENGL = 3,
	LIV_TEXTURE_DEVICE_VULKAN = 4,
	LIV_TEXTURE_DEVICE_METAL = 5
} LIV_TEXTURE_DEVICE;

typedef enum LIV_TEXTURE_COLOR_SPACE
{
	LIV_TEXTURE_COLOR_SPACE_UNDEFINED = 0,
	LIV_TEXTURE_COLOR_SPACE_LINEAR = 1,
	LIV_TEXTURE_COLOR_SPACE_SRGB = 2,
} LIV_TEXTURE_COLOR_SPACE;

typedef enum LIV_RENDERING_PIPELINE
{
	LIV_RENDERING_PIPELINE_UNDEFINED = 0,
	LIV_RENDERING_PIPELINE_FORWARD = 1,
	LIV_RENDERING_PIPELINE_DEFERRED = 2,
	LIV_RENDERING_PIPELINE_VERTEX_LIT = 3,
} LIV_RENDERING_PIPELINE;

typedef enum LIV_CUBE_SIDE
{
	LIV_CUBE_RIGHT = 0,
	LIV_CUBE_LEFT = 1,
	LIV_CUBE_UP = 2,
	LIV_CUBE_DOWN = 3,
	LIV_CUBE_FORWARD = 4,
	LIV_CUBE_BACK = 5
} LIV_CUBE_SIDE;

typedef unsigned long long LIV_FEATURES_ENUM; //< value from LIV_FEATURES
typedef int LIV_TEXTURE_COLOR_SPACE_ENUM; //< value from LIV_TEXTURE_COLOR_SPACE
typedef int LIV_TEXTURE_TYPE_ENUM; //< value from LIV_TEXTURE_TYPE
typedef int LIV_TEXTURE_ID_ENUM; //< value from LIV_TEXTURE_ID
typedef int LIV_RENDERING_PIPELINE_ENUM;

/// <summary>
/// 4D vectors, typically represent transform matrices columns
/// </summary>
typedef struct LIV_Vector4
{
	union
	{
		float data[4];
		struct
		{
			float x, y, z, w;
		};
	};
} LIV_Vector4;

/// <summary>
/// 3D vectors, typically represent positions
/// </summary>
typedef struct LIV_Vector3
{
	union
	{
		float data[3];
		struct
		{
			float x, y, z;
		};
	};
} LIV_Vector3;

/// <summary>
/// TODO MAYBEUNUSED
/// </summary>
typedef struct LIV_Vector2
{
	union
	{
		float data[2];
		struct
		{
			float x, y;
		};
	};
} LIV_Vector2;

/// <summary>
/// Quaternion, should have been normalized, typically represent a rotation
/// </summary>
typedef struct LIV_Quaternion
{
	union
	{
		float data[4];
		struct
		{
			float x, y, z, w;
		};
	};
} LIV_Quaternion;

/// <summary>
/// 4x4 float matrix. Typically represent a homogeneous transformation matrix
/// </summary>
typedef struct LIV_Matrix4x4
{
	union
	{
		float data[16];
		LIV_Vector4 vectors[4];
		struct
		{
			float m00, m01, m02, m03,
				m10, m11, m12, m13,
				m20, m21, m22, m23,
				m30, m31, m32, m33;
		};
	};
} LIV_Matrix4x4;

typedef struct LIV_Timing
{
	int frameindex;
	long time;
} LIV_Timing;

typedef struct LIV_RigidTransform
{
	LIV_Vector3 pos;
	LIV_Quaternion rot;
} LIV_RigidTransform;

/// <summary>
/// TODO exist in bridge but is not useful for Game?
/// </summary>
typedef struct LIV_CameraCalibration
{
	LIV_RigidTransform camera;
	LIV_Vector3 position;
	float yaw;
	float pitch;
	float roll;
	float VerticalFieldOfView;
	float fov_override;
	float nearClipPlane;
	float farClipPlane;
} LIV_CameraCalibration;

/// <summary>
/// MR Camera pose information
/// </summary>
typedef struct LIV_Pose
{
	/// <summary>
	/// A projection matrix, TODO what conventions? Seems to be wrong both for OpenGL and DirectX
	/// </summary>
	LIV_Matrix4x4 projectionMatrix;

	/// <summary>
	/// Position vector of the MR camera
	/// </summary>
	LIV_Vector3 local_position;

	/// <summary>
	/// Roation quaternion of the MR camera
	/// </summary>
	LIV_Quaternion local_rotation;

	/// <summary>
	/// Vertical opening angle of the MR camera
	/// </summary>
	float verticalFieldOfView;

	/// <summary>
	/// Near clipping plane of the MR camera frustum ("minimum Z distance")
	/// </summary>
	float nearClipPlane;

	/// <summary>
	/// Far clipping plane of the MR camera frustum ("maximum Z distance")
	/// </summary>
	float farClipPlane;

	/// <summary>
	/// Width of the requested camera render target
	/// </summary>
	int width;

	/// <summary>
	/// Height of the requested camera render target
	/// </summary>
	int height;
} LIV_Pose;

typedef struct LIV_Transform
{
	LIV_Vector3 trackedSpaceStagePosition;
	LIV_Quaternion trackedSpaceStageRotation;
	LIV_Vector3 trackedSpaceLocalScale;
} LIV_Transform;

typedef struct LIV_TrackedSpace
{
	LIV_Vector3 trackedSpaceStagePosition;
	LIV_Quaternion trackedSpaceStageRotation;
	LIV_Vector3 trackedSpaceLocalScale;
	LIV_Matrix4x4 trackedSpaceLocalToStageMatrix;
	LIV_Matrix4x4 trackedSpaceStageToLocalMatrix;
} LIV_TrackedSpace;

typedef struct LIV_ClipPlane
{
	LIV_Matrix4x4 transform;
	int width;
	int height;
	float tesselation;
} LIV_ClipPlane;

/// <summary>
/// Main communication structure, contains data about a frame to be rendered and composited
/// </summary>
typedef struct LIV_InputFrame
{
	/// <summary>
	/// Camera pose information
	/// </summary>
	LIV_Pose pose;

	/// <summary>
	/// Vertical clip plane information
	/// </summary>
	LIV_ClipPlane clipPlane;

	/// <summary>
	/// TODO
	/// </summary>
	LIV_Transform StageTransform;

	/// <summary>
	/// Feature bits
	/// </summary>
	LIV_FEATURES_ENUM features;

	/// <summary>
	/// Horizontal clipping plane
	/// </summary>
	LIV_ClipPlane GroundPlane;

	unsigned long long frameid;
	unsigned long long refrenceframe;

	/// <summary>
	/// Priority weights
	/// </summary>
	union
	{
		/// <summary>
		/// Priority tag as a 64bit binary blob
		/// </summary>
		unsigned long long priority;
		struct
		{
			/// <summary>
			/// Determine who control the pose
			/// </summary>
			signed char pose_priority;

			/// <summary>
			/// Determine who control the clip plane
			/// </summary>
			signed char clipplane_priority;

			/// <summary>
			/// Determine who control the stage transform
			/// </summary>
			signed char stage_priority;

			/// <summary>
			/// Determine who control the texture resolution
			/// </summary>
			signed char resolution_priority;

			/// <summary>
			/// Determine who control the feature bits
			/// </summary>
			signed char feature_priority;

			/// <summary>
			/// Determine who control the clipping planes
			/// </summary>
			signed char NearFarAdjustment_priority;

			/// <summary>
			/// Determine who control the horizontal clipping plane
			/// </summary>
			signed char groundplane_priority;
		};
	};
} LIV_InputFrame;


typedef struct LIV_OutputFrame
{
	LIV_Timing timing;
	LIV_Pose pose;
	LIV_FEATURES_ENUM features;
	LIV_RENDERING_PIPELINE_ENUM renderingPipeline;
	LIV_TrackedSpace trackedSpace;
} LIV_OutputFrame;

/// <summary>
/// Carrier of a texture that can be added to a LIV Compositor frame. Pass this to LIV_AddTexture(), then call LIV_Submit() once the frame is ready.
/// </summary>
typedef struct LIV_Texture
{
	/// <summary>
	/// Texture ID from the enumeration. Specify background/foreground or any other type in this
	/// </summary>
	LIV_TEXTURE_ID_ENUM id;

	/// <summary>
	/// GFX resource carrier. the field you can populate here is GFX API dependent.
	/// </summary>
	union
	{
#ifndef LIV_NO_DX11
		/// <summary>
		/// DirectX 11.x Texture Pointer
		/// </summary>
		ID3D11Texture2D* d3d11_texturePtr;
#else
		/// <summary>
		/// Address of a DirectX 11.x Texture Pointer
		/// </summary>
		uintptr_t d3d11_texturePtr; //Should be a ID3D11Texture2D*
#endif
		/// <summary>
		/// OpenGL texture name. Only valid if the OpenGL subsystem was started successfully
		/// </summary>
		unsigned int gl_textureName;

		/// <summary>
		/// This element only exist to ensure correct structure alignment and should be ignored. Do not read or assign from it.
		/// </summary>
		uintptr_t __ensure_padding_8_bytes;
	};

#ifndef LIV_NO_DX11
	HANDLE ShareableHandle;
#else
	uintptr_t ShareableHandle; // Should be a HANDLE
#endif

	unsigned long long timestamp;
	LIV_TEXTURE_TYPE_ENUM type;

	union
	{
#ifndef LIV_NO_DX11
		/// <summary>
		/// Format of the texture, when using a DirectX 11 input path
		/// </summary>
		DXGI_FORMAT dxgi_pixelFormat;
#else
		/// <summary>
		/// Format of the texture, when using a DirectX 11 input path
		/// </summary>
		unsigned int dxgi_pixelFormat; // Should be a DXGI_FORMAT value
#endif
		/// <summary>
		/// Format of the texture when using the OpenGL input path
		/// </summary>
		unsigned int gl_pixelFormat;

		//unsigned int __ensure_padding_4_bytes; <-- Actually not needed because DXGI_FORMAT and GLuint are the same thing
	};

	/// <summary>
	/// Color space this texture has been rendered to
	/// </summary>
	LIV_TEXTURE_COLOR_SPACE_ENUM colorSpace;

	/// <summary>
	/// size in horizontal axis
	/// </summary>
	int width;

	/// <summary>
	/// Size in vertical axis
	/// </summary>
	int height;

	/// <summary>
	/// TODO (maybe unused)
	/// </summary>
	int deviceType;
} LIV_Texture;

typedef struct LIV_Stats
{
	LIV_Timing hmdRenderStart;
	LIV_Timing hmdRenderStop;
} LIV_Stats;


typedef struct LIV_Resolution
{
	int width;
	int height;
} LIV_Resolution;


#endif //_LIV_SDK_STRUCT
