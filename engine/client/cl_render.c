/*
cl_render.c - RenderAPI loader & implementation
Copyright (C) 2019 a1batross

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#ifndef XASH_DEDICATED

#include "common.h"
#include "client.h"
#include "library.h"

static int R_FatPVS( const vec3_t org, float radius, byte *visbuffer, qboolean merge, qboolean fullvis )
{
	return Mod_FatPVS( org, radius, visbuffer, world.visbytes, merge, fullvis );
}

static lightstyle_t *CL_GetLightStyle( int number )
{
	Assert( number >= 0 && number < MAX_LIGHTSTYLES );
	return &cl.lightstyles[number];
}

static const ref_overview_t *GL_GetOverviewParms( void )
{
	return &clgame.overView;
}

static void *R_Mem_Alloc( size_t cb, const char *filename, const int fileline )
{
	return _Mem_Alloc( cls.mempool, cb, true, filename, fileline );
}

static void R_Mem_Free( void *mem, const char *filename, const int fileline )
{
	if( !mem ) return;
	_Mem_Free( mem, filename, fileline );
}

/*
=========
pfnGetFilesList

=========
*/
static char **pfnGetFilesList( const char *pattern, int *numFiles, int gamedironly )
{
	static search_t	*t = NULL;

	if( t ) Mem_Free( t ); // release prev search

	t = FS_Search( pattern, true, gamedironly );

	if( !t )
	{
		if( numFiles ) *numFiles = 0;
		return NULL;
	}

	if( numFiles ) *numFiles = t->numfilenames;
	return t->filenames;
}

static uint pfnFileBufferCRC32( const void *buffer, const int length )
{
	uint	modelCRC = 0;

	if( !buffer || length <= 0 )
		return modelCRC;

	CRC32_Init( &modelCRC );
	CRC32_ProcessBuffer( &modelCRC, buffer, length );
	return CRC32_Final( modelCRC );
}

/*
=============
CL_GenericHandle

=============
*/
const char *CL_GenericHandle( int fileindex )
{
	if( fileindex < 0 || fileindex >= MAX_CUSTOM )
		return 0;
	return cl.files_precache[fileindex];
}

static render_api_t gRenderAPI =
{
	NULL, // GL_RenderGetParm,
	NULL, // R_GetDetailScaleForTexture,
	NULL, // R_GetExtraParmsForTexture,
	CL_GetLightStyle,
	CL_GetDynamicLight,
	CL_GetEntityLight,
	LightToTexGamma,
	NULL, // R_GetFrameTime,
	NULL, // R_SetCurrentEntity,
	NULL, // R_SetCurrentModel,
	R_FatPVS,
	NULL, // R_StoreEfrags,
	NULL, // GL_FindTexture,
	NULL, // GL_TextureName,
	NULL, // GL_TextureData,
	NULL, // GL_LoadTexture,
	NULL, // GL_CreateTexture,
	NULL, // GL_LoadTextureArray,
	NULL, // GL_CreateTextureArray,
	NULL, // GL_FreeTexture,
	NULL, // DrawSingleDecal,
	NULL, // R_DecalSetupVerts,
	NULL, // R_EntityRemoveDecals,
	(void*)AVI_LoadVideo,
	(void*)AVI_GetVideoInfo,
	(void*)AVI_GetVideoFrameNumber,
	(void*)AVI_GetVideoFrame,
	NULL, // R_UploadStretchRaw,
	(void*)AVI_FreeVideo,
	(void*)AVI_IsActive,
	S_StreamAviSamples,
	NULL,
	NULL,
	NULL, // GL_Bind,
	NULL, // GL_SelectTexture,
	NULL, // GL_LoadTexMatrixExt,
	NULL, // GL_LoadIdentityTexMatrix,
	NULL, // GL_CleanUpTextureUnits,
	NULL, // GL_TexGen,
	NULL, // GL_TextureTarget,
	NULL, // GL_SetTexCoordArrayMode,
	NULL, // GL_GetProcAddress,
	NULL, // GL_UpdateTexSize,
	NULL,
	NULL,
	NULL, // CL_DrawParticlesExternal,
	NULL, // R_EnvShot,
	pfnSPR_LoadExt,
	NULL, // R_LightVec,
	NULL, // R_StudioGetTexture,
	GL_GetOverviewParms,
	CL_GenericHandle,
	NULL,
	NULL,
	R_Mem_Alloc,
	R_Mem_Free,
	pfnGetFilesList,
	pfnFileBufferCRC32,
	COM_CompareFileTime,
	Host_Error,
	(void*)CL_ModelHandle,
	pfnTime,
	Cvar_Set,
	S_FadeMusicVolume,
	COM_SetRandomSeed,
};

/*
===============
R_InitRenderAPI

Initialize client external rendering
===============
*/
qboolean R_InitRenderAPI( void )
{
	// make sure what render functions is cleared
	memset( &clgame.drawFuncs, 0, sizeof( clgame.drawFuncs ));

	if( clgame.dllFuncs.pfnGetRenderInterface )
	{
		if( clgame.dllFuncs.pfnGetRenderInterface( CL_RENDER_INTERFACE_VERSION, &gRenderAPI, &clgame.drawFuncs ))
		{
			Con_Reportf( "CL_LoadProgs: ^2initailized extended RenderAPI ^7ver. %i\n", CL_RENDER_INTERFACE_VERSION );
			return true;
		}

		// make sure what render functions is cleared
		memset( &clgame.drawFuncs, 0, sizeof( clgame.drawFuncs ));

		return false; // just tell user about problems
	}

	// render interface is missed
	return true;
}

#endif // XASH_DEDICATED