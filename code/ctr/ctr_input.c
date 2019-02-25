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
#include <3ds.h>

#include "keyboardOverlay_bin.h"
#include "touchOverlay_bin.h"

#include "../client/client.h"
#include "../sys/sys_local.h"

static bool keyboard_enabled = false;
static bool shift_pressed = false;

static int button_pressed = 0;

static uint16_t *framebuffer;

static circlePosition cstick;
static circlePosition circlepad;
static touchPosition  touch, old_touch;

typedef struct {
	uint32_t button;
	int key;
} buttonMapping;

static buttonMapping buttonMap[14] =
{
	{ KEY_SELECT, K_ESCAPE },
	{ KEY_START, K_ENTER },
	{ KEY_DUP, K_UPARROW },
	{ KEY_DRIGHT, K_RIGHTARROW },
	{ KEY_DDOWN, K_DOWNARROW },
	{ KEY_DLEFT, K_LEFTARROW },
	{ KEY_L, K_AUX5 },
	{ KEY_R, K_AUX6 },
	{ KEY_ZL, K_AUX7 },
	{ KEY_ZR, K_AUX8 },
	{ KEY_A, K_AUX1 },
	{ KEY_B, K_AUX2 },
	{ KEY_X, K_AUX3 },
	{ KEY_Y, K_AUX4 },
};


static uint16_t keymap[14 * 6] = 
{
	K_ESCAPE , K_F1, K_F2, K_F3, K_F4, K_F5, K_F6, K_F7, K_F8, K_F9, K_F10, K_F11, K_F12, 0,
	'`', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\r',
	K_TAB, 'q' , 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '|',
	0, 'a' , 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', K_ENTER, K_ENTER,
	K_SHIFT, 'z' , 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, K_UPARROW, 0,
	0, 0 , 0, 0, K_SPACE, K_SPACE, K_SPACE, K_SPACE, K_SPACE, K_SPACE, 0, K_LEFTARROW, 	K_DOWNARROW, K_RIGHTARROW
};

static void DrawSubscreen()
{
	int x, y;
	uint16_t* overlay;

	if(keyboard_enabled)
		overlay = keyboardOverlay_bin;
	else
		overlay = touchOverlay_bin;

	memcpy(framebuffer, overlay, 240*320*2);

	if(keyboard_enabled && shift_pressed)
	{
		for(x = 20; x < 23; x++)
      		for(y = 152; y < 155; y++)
				framebuffer[(x*240 + (239 - y))] = RGB8_to_565(39, 174, 96);
	}

}

static void RescaleAnalog( int *x, int *y, float deadZone )
{
	float analogX = (float)*x;
	float analogY = (float)*y;
	float maximum = 180.0f;
	float magnitude = sqrtf( analogX * analogX + analogY * analogY );

	if( magnitude >= deadZone )
	{
		float scalingFactor = maximum / magnitude * ( magnitude - deadZone ) / ( maximum - deadZone );
		*x = (int)( analogX * scalingFactor );
		*y = (int)( analogY * scalingFactor );
	}
	else
	{
		*x = 0;
		*y = 0;
	}
}

static void UpdateTouch( void )
{
	if(hidKeysDown() & KEY_TOUCH)
	{
		if(touch.py < 42 && touch.py > 1)
		{
			button_pressed = K_AUX9 + (touch.px / 80);
		}

		if(keyboard_enabled && touch.py > 59 && touch.py < 193 && touch.px > 6 && touch.px < 314)
		{
			button_pressed = keymap[((touch.py - 59) / 22) * 14 + (touch.px - 6) / 22];
		}

		if(touch.py > 213 && touch.px > 135 && touch.px < 185)
		{
			keyboard_enabled = !keyboard_enabled;
			DrawSubscreen();
		}

		if(button_pressed)
		{
			if(button_pressed == K_SHIFT)
			{
				shift_pressed = !shift_pressed;
				Key_Event(K_SHIFT, shift_pressed, Sys_Milliseconds());
				DrawSubscreen();

				button_pressed = 0;
			}
			else if(!(button_pressed < 32 || button_pressed > 126 || button_pressed == '`'))
			{
				Com_QueueEvent(Sys_Milliseconds(), SE_CHAR, button_pressed, 0, 0, NULL);
			}

			Com_QueueEvent(Sys_Milliseconds(), SE_KEY, button_pressed, true, 0, NULL);
		}
	}
	else if(hidKeysHeld() & KEY_TOUCH)
	{
		if(!button_pressed)
		{
			Com_QueueEvent(time, SE_MOUSE, touch.px - old_touch.px, touch.py - old_touch.py, 0, NULL);
		}
	}
	else if(hidKeysUp() & KEY_TOUCH)
	{
		if(button_pressed)
		{
			Com_QueueEvent(Sys_Milliseconds(), SE_KEY, button_pressed, false, 0, NULL);
		}

		button_pressed = 0;
	}

	old_touch = touch;
}
/*
===============
IN_Frame
===============
*/

void Key_Event(int key, int value, int time)
{
	Com_QueueEvent(time, SE_KEY, key, value, 0, NULL);
}

void IN_Frame( void )
{
	hidScanInput();
	hidTouchRead(&touch);
	hidCstickRead(&cstick);

	for(int i = 0; i < 14; i++)
	{
		if( ( hidKeysDown() & buttonMap[i].button ) )
				Key_Event( buttonMap[i].key, true, Sys_Milliseconds());
		else if( ( hidKeysUp() & buttonMap[i].button ) )
				Key_Event( buttonMap[i].key, false, Sys_Milliseconds());
	}

	int right_x = cstick.dx;
	int right_y = -cstick.dy;

	RescaleAnalog( &right_x, &right_y, 25.0f );

	Com_QueueEvent(time, SE_MOUSE, right_x/4, right_y/4, 0, NULL);

	UpdateTouch();
}

/*
===============
IN_Init
===============
*/
void IN_Init( void *windowData )
{
	framebuffer = (uint16_t*)gfxGetFramebuffer(GFX_BOTTOM, GFX_LEFT, NULL, NULL);
	DrawSubscreen();
}

/*
===============
IN_Shutdown
===============
*/
void IN_Shutdown( void )
{
}

/*
===============
IN_Restart
===============
*/
void IN_Restart( void )
{
}
