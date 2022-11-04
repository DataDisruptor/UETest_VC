/**
 * \file LIV_Pathfinder.h
 * \brief API to access Pathfinder
 * \copyright 2021-2022 LIV Inc. S.r.o.
 *
 * This provide the application access to the LIV Pathfinder data sharing scheme
 *
 */

#ifndef __LIV_PATHFINDER_H
#define __LIV_PATHFINDER_H

#include <inttypes.h>

#include "LIV_Dynamic.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

	typedef enum LIV_Pathfinder_Type_
	{
		LIV_PF_Int = 9,
		LIV_PF_Function = 9,
		LIV_PF_Long = 9,
		LIV_PF_Texture = 27,
		LIV_PF_String = 36,
		LIV_PF_BinaryBlob = 252,
	} LIV_Pathfinder_Type;

	/// <summary>
	/// Set data on Pathfinder
	/// </summary>
	/// <param name="path">A pathfinder path</param>
	/// <param name="data">Pointer to a data object to be read</param>
	/// <param name="len">Size of data object</param>
	/// <param name="type">Object type</param>
	/// <returns>true in case of success</returns>
	LIV_DECLSPEC bool LIV_Pathfinder_Set(const char* path, const void* data, int len, LIV_Pathfinder_Type type);

	/// <summary>
	/// Get data from Pathfinder
	/// </summary>
	/// <param name="path">A pathfinder path</param>
	/// <param name="data">pointer to object to write to</param>
	/// <param name="len">size of the object to write to</param>
	/// <param name="type">Object type</param>
	/// <returns>true in case of success</returns>
	LIV_DECLSPEC bool LIV_Pathfinder_Get(const char* path, void* data, int len, LIV_Pathfinder_Type type);

#ifdef __cplusplus
} //extern "C"
#endif

#endif