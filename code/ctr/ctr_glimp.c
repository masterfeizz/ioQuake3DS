/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "../renderercommon/tr_common.h"
#include "../sys/sys_local.h"

#include <3ds.h>
#include <GL/picaGL.h>

extern qboolean isN3DS;

/*
===============
GLimp_Shutdown
===============
*/
void GLimp_Shutdown( void )
{
	ri.IN_Shutdown();
}

/*
===============
GLimp_Minimize

Minimize the game so that user is back at the desktop
===============
*/
void GLimp_Minimize( void )
{
}


/*
===============
GLimp_LogComment
===============
*/
extern void log2file(const char *format, ...);
void GLimp_LogComment( char *comment )
{
#ifndef RELEASE
	log2file(comment);
#endif
}

#define R_MODE_FALLBACK 3 // 640 * 480

/*
===============
GLimp_Init

This routine is responsible for initializing the OS specific portions
of OpenGL
===============
*/

void GLimp_Init( qboolean coreContext)
{
	static int pgl_initialized = 0;

	if(!pgl_initialized)
		pglInitEx(0x040000, 0x100000);

	pgl_initialized = 1;
	
	glConfig.vidWidth = 400;
	glConfig.vidHeight = 240;
	glConfig.colorBits = 32;
	glConfig.depthBits = 24;
	glConfig.stencilBits = 8;
	glConfig.displayFrequency = 60;
	glConfig.stereoEnabled = qfalse;
	glConfig.maxTextureSize = 256;
	
	glConfig.driverType = GLDRV_ICD;
	glConfig.hardwareType = GLHW_GENERIC;
	glConfig.deviceSupportsGamma = qfalse;
	glConfig.textureCompression = TC_NONE;
	glConfig.textureEnvAddAvailable = qtrue;
	glConfig.windowAspect = 400.0f / 240.0f;
	glConfig.isFullscreen = qtrue;
	
	strncpy(glConfig.vendor_string, glGetString(GL_VENDOR), sizeof(glConfig.vendor_string));
	strncpy(glConfig.renderer_string, glGetString(GL_RENDERER), sizeof(glConfig.renderer_string));
	strncpy(glConfig.version_string, glGetString(GL_VERSION), sizeof(glConfig.version_string));
	strncpy(glConfig.extensions_string, glGetString(GL_EXTENSIONS), sizeof(glConfig.extensions_string));
	
	qglClearColor( 0, 0, 0, 0 );
	qglClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	
}


/*
===============
GLimp_EndFrame

Responsible for doing a swapbuffers
===============
*/
void GLimp_EndFrame( void )
{
	pglSwapBuffersEx(true, false);
}

/*
===========================================================
SMP acceleration
===========================================================
*/

LightEvent	renderCommandsEvent;
LightEvent	renderActiveEvent;
//LightEvent	renderCompletedEvent;

void ( *glimpRenderThread )( void );

static void GLimp_RenderThreadWrapper(void* arg)
{
	glimpRenderThread();
}


/*
=======================
GLimp_SpawnRenderThread
=======================
*/
Thread renderThreadHandle;

qboolean GLimp_SpawnRenderThread( void ( *function )( void ) )
{
	if(isN3DS == false) return qfalse;
	
	LightEvent_Init(&renderCommandsEvent,  RESET_ONESHOT);
	LightEvent_Init(&renderActiveEvent,    RESET_ONESHOT);
	//LightEvent_Init(&renderCompletedEvent, RESET_ONESHOT);

	glimpRenderThread = function;

	renderThreadHandle = threadCreate(GLimp_RenderThreadWrapper, 0, 2 * 1024 * 1024, 0x18, isN3DS ? 2 : 1, true);

	return qtrue;
}

static void *smpData = NULL;

void *GLimp_RendererSleep( void )
{
	void  *data;

	// after this, the front end can exit GLimp_FrontEndSleep
	//LightEvent_Signal(&renderCompletedEvent);

	LightEvent_Wait (&renderCommandsEvent);

	data = smpData;

	LightEvent_Signal(&renderActiveEvent);

	return data;
}


void GLimp_FrontEndSleep( void )
{
	//LightEvent_Wait (&renderCompletedEvent);
}


void GLimp_WakeRenderer( void *data )
{
	smpData = data;

	LightEvent_Signal(&renderCommandsEvent);

	LightEvent_Wait (&renderActiveEvent);
}