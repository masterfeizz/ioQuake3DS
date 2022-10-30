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

#include <signal.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/types.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#ifndef DEDICATED
#include <3ds.h>
#endif

#include "sys_local.h"
#include "sys_loadlib.h"

#include "../qcommon/q_shared.h"
#include "../qcommon/qcommon.h"

int __stacksize__ = 1536 * 1024;

qboolean isN3DS = qfalse;

int ctr_zoneMegs  = 0;
int ctr_hunkMegs  = 0;
int ctr_soundMegs = 0;

static char binaryPath[MAX_OSPATH] = {0};
static char installPath[MAX_OSPATH] = {0};

mode_t umask(mode_t mask) {
    return 0;
}

/*
=================
Sys_SetBinaryPath
=================
*/
void Sys_SetBinaryPath(const char *path) {
    Q_strncpyz(binaryPath, path, sizeof(binaryPath));
}

/*
=================
Sys_BinaryPath
=================
*/
char *Sys_BinaryPath(void) {
    return binaryPath;
}

/*
=================
Sys_SetDefaultInstallPath
=================
*/
void Sys_SetDefaultInstallPath(const char *path) {
    Q_strncpyz(installPath, path, sizeof(installPath));
}

/*
=================
Sys_DefaultInstallPath
=================
*/
char *Sys_DefaultInstallPath(void) {
    if (*installPath)
        return installPath;
    else
        return Sys_Cwd();
}

/*
=================
Sys_DefaultAppPath
=================
*/
char *Sys_DefaultAppPath(void) {
    return Sys_BinaryPath();
}

/*
=================
Sys_In_Restart_f

Restart the input subsystem
=================
*/
void Sys_In_Restart_f(void) {
    IN_Restart();
}

/*
=================
Sys_ConsoleInput

Handle new console input
=================
*/
char *Sys_ConsoleInput(void) {
    return CON_Input();
}

/*
==================
Sys_GetClipboardData
==================
*/
char *Sys_GetClipboardData(void) {
    return NULL;
}

void Sys_InitPIDFile(const char *gamedir) {}

void Sys_RemovePIDFile(const char *gamedir) {}

/*
=================
Sys_Exit

Single exit point (regular exit or in case of error)
=================
*/
static __attribute__ ((noreturn)) void Sys_Exit(int exitCode) {
    CON_Shutdown();

    //SDL_Quit();

    NET_Shutdown();

    Sys_PlatformExit();

    exit(exitCode);
}

/*
=================
Sys_Quit
=================
*/
void Sys_Quit(void) {
    Sys_Exit(0);
}

/*
=================
Sys_GetProcessorFeatures
=================
*/
cpuFeatures_t Sys_GetProcessorFeatures(void) {
    return (cpuFeatures_t) 0;
}

/*
=================
Sys_Init
=================
*/
void Sys_Init(void) {
    Cmd_AddCommand("in_restart", Sys_In_Restart_f);
    Cvar_Set("arch", OS_STRING " " ARCH_STRING);
    Cvar_Set("username", Sys_GetCurrentUser());
}

/*
=================
Sys_Print
=================
*/
void Sys_Print(const char *msg) {
    printf("%s", msg);
}

/*
=================
Sys_Error
=================
*/
void Sys_Error(const char *error, ...) {
    va_list argptr;
    char string[1024];

    consoleInit(GFX_BOTTOM, NULL);

    va_start (argptr, error);
    Q_vsnprintf(string, sizeof(string), error, argptr);
    va_end (argptr);

    Sys_ErrorDialog(string);

    while(!(hidKeysDown() & KEY_START))
        hidScanInput();

    Sys_Exit(3);
}

/*
============
Sys_FileTime

returns -1 if not present
============
*/
int Sys_FileTime(char *path) {
    struct stat buf;

    if (stat(path, &buf) == -1)
        return -1;

    return buf.st_mtime;
}

/*
=================
Sys_UnloadDll
=================
*/
void Sys_UnloadDll(void *dllHandle) {
    Com_Printf("Sys_UnloadDll: not implemented\n");
    return;
}

/*
=================
Sys_LoadDll

First try to load library name from system library path,
from executable path, then fs_basepath.
=================
*/

void *Sys_LoadDll(const char *name, qboolean useSystemLib) {
    Com_Printf("Sys_LoadDll: not implemented\n");
    return NULL;
}

/*
=================
Sys_LoadGameDll

Used to load a development dll instead of a virtual machine
=================
*/
void *Sys_LoadGameDll(const char *name,
                      intptr_t (QDECL **entryPoint)(int, ...),
                      intptr_t (*systemcalls)(intptr_t, ...)) {
    Com_Printf("Sys_LoadGameDll: not implemented\n");
    return NULL;
}

/*
=================
Sys_ParseArgs
=================
*/
void Sys_ParseArgs(int argc, char **argv) {
    if (argc == 2) {
        if (!strcmp(argv[1], "--version") ||
            !strcmp(argv[1], "-v")) {
            const char *date = PRODUCT_DATE;
#ifdef DEDICATED
            fprintf( stdout, Q3_VERSION " dedicated server (%s)\n", date );
#else
            fprintf(stdout, Q3_VERSION " client (%s)\n", date);
#endif
            Sys_Exit(0);
        }
    }
}

#ifndef DEFAULT_BASEDIR
#		define DEFAULT_BASEDIR Sys_BinaryPath()
#endif

/*
=================
Sys_SigHandler
=================
*/
void Sys_SigHandler(int signal) {
    static qboolean signalcaught = qfalse;

    if (signalcaught) {
        fprintf(stderr, "DOUBLE SIGNAL FAULT: Received signal %d, exiting...\n",
                signal);
    } else {
        signalcaught = qtrue;
        VM_Forced_Unload_Start();
#ifndef DEDICATED
        CL_Shutdown(va("Received signal %d", signal), qtrue, qtrue);
#endif
        SV_Shutdown(va("Received signal %d", signal));
        VM_Forced_Unload_Done();
    }

    if (signal == SIGTERM || signal == SIGINT)
        Sys_Exit(1);
    else
        Sys_Exit(2);
}

/*
=================
Sys_CalcMemSizes
=================
*/
void Sys_CalcMemSizes()
{
    int totalMegs = envGetHeapSize() >> 20;

    if(totalMegs > 88)
    {
        ctr_hunkMegs = 52;
        ctr_zoneMegs = 26;
        ctr_soundMegs = 6;
    }
    else if(totalMegs > 50)
    {
        ctr_hunkMegs = 50;
        ctr_zoneMegs = 24;
        ctr_soundMegs = 4;
    }
    else
    {
        ctr_hunkMegs = 26;
        ctr_zoneMegs = 10;
        ctr_soundMegs = 4;
    }
}

/*
=================
main
=================
*/

int main(int argc, char **argv) 
{

    osSetSpeedupEnable(true);

    APT_CheckNew3DS(&isN3DS);

    if(isN3DS)
    {
        #if OPENARENA
        gfxInit(GSP_RGB565_OES, GSP_RGB565_OES, false);
        #else
        gfxInit(GSP_BGR8_OES, GSP_RGB565_OES, false);
        #endif

        gfxSetWide(true);
    }
    else
    {
        gfxInit(GSP_RGB565_OES, GSP_RGB565_OES, false);
    }

    gfxSetDoubleBuffering(GFX_BOTTOM, false);

    int i;
    char** argvv = (char**)argv;
    char commandLine[MAX_STRING_CHARS] = {0};

    Sys_CalcMemSizes();

    Sys_PlatformInit();

    // Set the initial time base
    Sys_Milliseconds();

    //Sys_ParseArgs(argc, argv);
    mkdir(DEFAULT_BASEDIR, 777);
    Sys_SetBinaryPath(DEFAULT_BASEDIR);
    Sys_SetDefaultInstallPath(DEFAULT_BASEDIR);

    // Concatenate the command line for passing to Com_Init
    for (i = 1; i < argc; i++) {
        const qboolean containsSpaces = strchr(argvv[i], ' ') != NULL;
        if (containsSpaces)
            Q_strcat(commandLine, sizeof(commandLine), "\"");

        Q_strcat(commandLine, sizeof(commandLine), argvv[i]);

        if (containsSpaces)
            Q_strcat(commandLine, sizeof(commandLine), "\"");

        Q_strcat(commandLine, sizeof(commandLine), " ");
    }

    CON_Init();
    Com_Init(commandLine);
    NET_Init();
    IN_Init(NULL);
    
    while (aptMainLoop())
    {
        Com_Frame();
    }

    return 0;
}
