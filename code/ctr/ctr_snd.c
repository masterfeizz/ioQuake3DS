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

#include <stdlib.h>
#include <stdio.h>
#include <3ds.h>

#include "../qcommon/q_shared.h"
#include "../client/snd_local.h"

#define SAMPLE_RATE   	22050
#define NUM_SAMPLES 	4096
#define SAMPLE_SIZE		4
#define BUFFER_SIZE 	NUM_SAMPLES * SAMPLE_SIZE

static int sound_initialized = 0;
static byte *audio_buffer;
static ndspWaveBuf wave_buf;

qboolean SNDDMA_Init(void)
{
	sound_initialized = 0;

  	if(ndspInit() != 0)
    	return false;

    audio_buffer = linearAlloc(BUFFER_SIZE);

    ndspSetOutputMode(NDSP_OUTPUT_STEREO);
	ndspChnReset(0);
	ndspChnWaveBufClear(0);
	ndspChnSetInterp(0, NDSP_INTERP_LINEAR);
	ndspChnSetRate(0, (float)SAMPLE_RATE);
	ndspChnSetFormat(0, NDSP_FORMAT_STEREO_PCM16);

	memset(&wave_buf, 0, sizeof(wave_buf));
	wave_buf.data_vaddr = audio_buffer;
	wave_buf.nsamples 	= BUFFER_SIZE / 4;
	wave_buf.looping	= 1;

	/* Fill the audio DMA information block */
	dma.samplebits = 16;
	dma.speed = SAMPLE_RATE;
	dma.channels = 2;
	dma.samples = BUFFER_SIZE / 2;
	dma.fullsamples = dma.samples / dma.channels;
	dma.submission_chunk = 1;
	dma.buffer = audio_buffer;

	ndspChnWaveBufAdd(0, &wave_buf);

	sound_initialized = 1;

	return true;
}

int SNDDMA_GetDMAPos(void)
{
	if(!sound_initialized)
		return 0;

	return ndspChnGetSamplePos(0) * 2;
}

void SNDDMA_Shutdown(void)
{
	if(!sound_initialized)
		return;

	ndspChnWaveBufClear(0);
	ndspExit();
	linearFree(audio_buffer);

	sound_initialized = 0;
}

/*
==============
SNDDMA_Submit
Send sound to device if buffer isn't really the dma buffer
===============
*/
void SNDDMA_Submit(void)
{
	if(sound_initialized)
		DSP_FlushDataCache(audio_buffer, BUFFER_SIZE);
}

void SNDDMA_BeginPainting(void)
{    
}