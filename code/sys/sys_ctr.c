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

#include "../qcommon/q_shared.h"
#include "../qcommon/qcommon.h"
#include "sys_local.h"

#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/time.h>
#include <pwd.h>
#include <libgen.h>
#include <fcntl.h>
#include <sys/wait.h>

#include <3ds.h>
#include <sys/select.h>

void log2file(const char *format, ...) {
	__gnuc_va_list arg;
	int done;
	va_start(arg, format);
	char msg[512];
	done = vsprintf(msg, format, arg);
	va_end(arg);
	int i;
	sprintf(msg, "%s\n", msg);
	CON_Print(msg);
}

#ifndef RELEASE
# define printf log2file
#endif

qboolean stdinIsATTY;

// Used to determine where to store user-specific files
static char homePath[MAX_OSPATH] = {0};

// Used to store the Steam Quake 3 installation path
static char steamPath[MAX_OSPATH] = {0};

// Used to store the GOG Quake 3 installation path
static char gogPath[MAX_OSPATH] = {0};

/*
==================
Sys_DefaultHomePath
==================
*/
char *Sys_DefaultHomePath(void) {
    char *p;

    if (!*homePath && com_homepath != NULL) {
        if ((p = getenv("HOME")) != NULL) {
            Com_sprintf(homePath, sizeof(homePath), "%s%c", p, PATH_SEP);
#ifdef __APPLE__
            Q_strcat(homePath, sizeof(homePath),
                "Library/Application Support/");

            if(com_homepath->string[0])
                Q_strcat(homePath, sizeof(homePath), com_homepath->string);
            else
                Q_strcat(homePath, sizeof(homePath), HOMEPATH_NAME_MACOSX);
#else
            if (com_homepath->string[0])
                Q_strcat(homePath, sizeof(homePath), com_homepath->string);
            else
                Q_strcat(homePath, sizeof(homePath), HOMEPATH_NAME_UNIX);
#endif
        }
    }

    return homePath;
}

/*
================
Sys_SteamPath
================
*/
char *Sys_SteamPath(void) {
    // Disabled since Steam doesn't let you install Quake 3 on Mac/Linux
#if 0 //#ifdef STEAMPATH_NAME
    char *p;

    if( ( p = getenv( "HOME" ) ) != NULL )
    {
#ifdef __APPLE__
        char *steamPathEnd = "/Library/Application Support/Steam/SteamApps/common/" STEAMPATH_NAME;
#else
        char *steamPathEnd = "/.steam/steam/SteamApps/common/" STEAMPATH_NAME;
#endif
        Com_sprintf(steamPath, sizeof(steamPath), "%s%s", p, steamPathEnd);
    }
#endif

    return steamPath;
}

/*
================
Sys_GogPath
================
*/
char *Sys_GogPath(void) {
    // GOG also doesn't let you install Quake 3 on Mac/Linux
    return gogPath;
}

/*
================
Sys_Milliseconds
================
*/
int curtime;
int Sys_Milliseconds (void)
{
    static u64  base;

    u64 time = osGetTime();
    
    if (!base)
    {
        base = time;
    }

    curtime = (int)(time - base);
    return curtime;
}

/*
==================
Sys_RandomBytes
==================
*/
qboolean Sys_RandomBytes(byte *string, int len) {
    FILE *fp;

    fp = fopen("/dev/urandom", "r");
    if (!fp)
        return qfalse;

    setvbuf(fp, NULL, _IONBF, 0); // don't buffer reads from /dev/urandom

    if (fread(string, sizeof(byte), len, fp) != len) {
        fclose(fp);
        return qfalse;
    }

    fclose(fp);
    return qtrue;
}

/*
==================
Sys_GetCurrentUser
==================
*/
char *Sys_GetCurrentUser(void) {
    return "nick";
}

#define MEM_THRESHOLD 96*1024*1024

/*
==================
Sys_LowPhysicalMemory

TODO
==================
*/
qboolean Sys_LowPhysicalMemory(void) {
    return qtrue;
}

/*
==================
Sys_Basename
==================
*/
const char *Sys_Basename(char *path) {
    char *p = strrchr(path, '/');
    return p ? p + 1 : (char *) path;
}

/*
==================
Sys_Dirname
==================
*/
const char *Sys_Dirname(char *path) {
    static const char dot[] = ".";
    char *last_slash;

    /* Find last '/'.  */
    last_slash = path != NULL ? strrchr(path, '/') : NULL;

    if (last_slash != NULL && last_slash != path && last_slash[1] == '\0') {
        /* Determine whether all remaining characters are slashes.  */
        char *runp;

        for (runp = last_slash; runp != path; --runp)
            if (runp[-1] != '/')
                break;

        /* The '/' is the last character, we have to look further.  */
        if (runp != path)
            last_slash = memrchr(path, '/', runp - path);
    }

    if (last_slash != NULL) {
        /* Determine whether all remaining characters are slashes.  */
        char *runp;

        for (runp = last_slash; runp != path; --runp)
            if (runp[-1] != '/')
                break;

        /* Terminate the path.  */
        if (runp == path) {
            /* The last slash is the first character in the string.  We have to
            return "/".  As a special case we have to return "//" if there
            are exactly two slashes at the beginning of the string.  See
            XBD 4.10 Path Name Resolution for more information.  */
            if (last_slash == path + 1)
                ++last_slash;
            else
                last_slash = path + 1;
        } else
            last_slash = runp;

        last_slash[0] = '\0';
    } else
        /* This assignment is ill-designed but the XPG specs require to
        return a string containing "." in any case no directory part is
        found and so a static and constant string is required.  */
        path = (char *) dot;

    return path;
}

/*
==============
Sys_FOpen
==============
*/
FILE *Sys_FOpen(const char *ospath, const char *mode) {
    struct stat buf;

    // check if path exists and is a directory
    if (!stat(ospath, &buf) && S_ISDIR(buf.st_mode))
        return NULL;

    return fopen(ospath, mode);
}

/*
==================
Sys_Mkdir
==================
*/
qboolean Sys_Mkdir(const char *path) {

    mkdir(path, 0777);

    return qtrue;
}

/*
==================
Sys_Mkfifo
==================
*/
FILE *Sys_Mkfifo(const char *ospath) {
    return NULL;
}

/*
==================
Sys_Cwd
==================
*/
char *Sys_Cwd(void) {
    return DEFAULT_BASEDIR;
}

/*
==============================================================

DIRECTORY SCANNING

==============================================================
*/

#define MAX_FOUND_FILES 0x1000

/*
==================
Sys_ListFilteredFiles
==================
*/
void Sys_ListFilteredFiles(const char *basedir, char *subdirs, char *filter, char **list, int *numfiles) {
    char search[MAX_OSPATH], newsubdirs[MAX_OSPATH];
    char filename[MAX_OSPATH];
    DIR *fdir;
    struct dirent *d;
    struct stat st;

    if (*numfiles >= MAX_FOUND_FILES - 1) {
        return;
    }

    if (strlen(subdirs)) {
        Com_sprintf(search, sizeof(search), "%s/%s", basedir, subdirs);
    } else {
        Com_sprintf(search, sizeof(search), "%s", basedir);
    }

    if ((fdir = opendir(search)) == NULL) {
        return;
    }

    while ((d = readdir(fdir)) != NULL) {
        Com_sprintf(filename, sizeof(filename), "%s/%s", search, d->d_name);
        if (stat(filename, &st) == -1)
            continue;

        if (st.st_mode & S_IFDIR) {
            if (Q_stricmp(d->d_name, ".") && Q_stricmp(d->d_name, "..")) {
                if (strlen(subdirs)) {
                    Com_sprintf(newsubdirs, sizeof(newsubdirs), "%s/%s", subdirs, d->d_name);
                } else {
                    Com_sprintf(newsubdirs, sizeof(newsubdirs), "%s", d->d_name);
                }
                Sys_ListFilteredFiles(basedir, newsubdirs, filter, list, numfiles);
            }
        }
        if (*numfiles >= MAX_FOUND_FILES - 1) {
            break;
        }
        Com_sprintf(filename, sizeof(filename), "%s/%s", subdirs, d->d_name);
        if (!Com_FilterPath(filter, filename, qfalse))
            continue;
        list[*numfiles] = CopyString(filename);
        (*numfiles)++;
    }

    closedir(fdir);
}

/*
==================
Sys_ListFiles
==================
*/
char **Sys_ListFiles(const char *directory, const char *extension, char *filter, int *numfiles, qboolean wantsubs) {
    struct dirent *d;
    DIR *fdir;
    qboolean dironly = wantsubs;
    char search[MAX_OSPATH];
    int nfiles;
    char **listCopy;
    char *list[MAX_FOUND_FILES];
    int i;
    struct stat st;

    int extLen;

    if (filter) {

        nfiles = 0;
        Sys_ListFilteredFiles(directory, "", filter, list, &nfiles);

        list[nfiles] = NULL;
        *numfiles = nfiles;

        if (!nfiles)
            return NULL;

        listCopy = Z_Malloc((nfiles + 1) * sizeof(*listCopy));
        for (i = 0; i < nfiles; i++) {
            listCopy[i] = list[i];
        }
        listCopy[i] = NULL;

        return listCopy;
    }

    if (!extension)
        extension = "";

    if (extension[0] == '/' && extension[1] == 0) {
        extension = "";
        dironly = qtrue;
    }

    extLen = strlen(extension);

    // search
    nfiles = 0;

    if ((fdir = opendir(directory)) == NULL) {
        *numfiles = 0;
        return NULL;
    }

    while ((d = readdir(fdir)) != NULL) {
        Com_sprintf(search, sizeof(search), "%s/%s", directory, d->d_name);
        if (stat(search, &st) == -1)
            continue;
        if ((dironly && !(st.st_mode & S_IFDIR)) ||
            (!dironly && (st.st_mode & S_IFDIR)))
            continue;

        if (*extension) {
            if (strlen(d->d_name) < extLen ||
                Q_stricmp(
                        d->d_name + strlen(d->d_name) - extLen,
                        extension)) {
                continue; // didn't match
            }
        }

        if (nfiles == MAX_FOUND_FILES - 1)
            break;
        list[nfiles] = CopyString(d->d_name);
        nfiles++;
    }

    list[nfiles] = NULL;

    closedir(fdir);

    // return a copy of the list
    *numfiles = nfiles;

    if (!nfiles) {
        return NULL;
    }

    listCopy = Z_Malloc((nfiles + 1) * sizeof(*listCopy));
    for (i = 0; i < nfiles; i++) {
        listCopy[i] = list[i];
    }
    listCopy[i] = NULL;

    return listCopy;
}

/*
==================
Sys_FreeFileList
==================
*/
void Sys_FreeFileList(char **list) {
    int i;

    if (!list) {
        return;
    }

    for (i = 0; list[i]; i++) {
        Z_Free(list[i]);
    }

    Z_Free(list);
}

/*
==================
Sys_Sleep

Block execution for msec or until input is received.
==================
*/
void Sys_Sleep(int msec) {

}

/*
==============
Sys_ErrorDialog

Display an error message
==============
*/
void Sys_ErrorDialog(const char *error) {
    char buffer[1024];
    unsigned int size;
    int f = -1;
    const char *homepath = Cvar_VariableString("fs_homepath");
    const char *gamedir = Cvar_VariableString("fs_game");
    const char *fileName = "crashlog.txt";
    char *dirpath = FS_BuildOSPath(homepath, gamedir, "");
    char *ospath = FS_BuildOSPath(homepath, gamedir, fileName);

    Sys_Print(va("%s\n", error));

#ifndef DEDICATED
    Sys_Dialog(DT_ERROR, va("%s. See \"%s\" for details.", error, ospath), "Error");
#endif

    // Make sure the write path for the crashlog exists...

    if (!Sys_Mkdir(homepath)) {
        Com_Printf("ERROR: couldn't create path '%s' for crash log.\n", homepath);
        return;
    }

    if (!Sys_Mkdir(dirpath)) {
        Com_Printf("ERROR: couldn't create path '%s' for crash log.\n", dirpath);
        return;
    }

    // We might be crashing because we maxed out the Quake MAX_FILE_HANDLES,
    // which will come through here, so we don't want to recurse forever by
    // calling FS_FOpenFileWrite()...use the Unix system APIs instead.
    f = open(ospath, O_CREAT | O_TRUNC | O_WRONLY, 0640);
    if (f == -1) {
        Com_Printf("ERROR: couldn't open %s\n", fileName);
        return;
    }

    #ifndef __3DS__
    // We're crashing, so we don't care much if write() or close() fails.
    while ((size = CON_LogRead(buffer, sizeof(buffer))) > 0) {
        if (write(f, buffer, size) != size) {
            Com_Printf("ERROR: couldn't fully write to %s\n", fileName);
            break;
        }
    }

    #endif

    close(f);
}

#ifndef __APPLE__
static char execBuffer[1024];
static char *execBufferPointer;
static char *execArgv[16];
static int execArgc;

/*
==============
Sys_ClearExecBuffer
==============
*/
static void Sys_ClearExecBuffer(void) {
    execBufferPointer = execBuffer;
    Com_Memset(execArgv, 0, sizeof(execArgv));
    execArgc = 0;
}

/*
==============
Sys_AppendToExecBuffer
==============
*/
static void Sys_AppendToExecBuffer(const char *text) {
    size_t size = sizeof(execBuffer) - (execBufferPointer - execBuffer);
    int length = strlen(text) + 1;

    if (length > size || execArgc >= ARRAY_LEN(execArgv))
        return;

    Q_strncpyz(execBufferPointer, text, size);
    execArgv[execArgc++] = execBufferPointer;

    execBufferPointer += length;
}

/*
==============
Sys_Exec
==============
*/
static int Sys_Exec(void) {
    return -1;
}

/*
==============
Sys_ZenityCommand
==============
*/
static void Sys_ZenityCommand(dialogType_t type, const char *message, const char *title) {
    Sys_ClearExecBuffer();
    Sys_AppendToExecBuffer("zenity");

    switch (type) {
        default:
        case DT_INFO:
            Sys_AppendToExecBuffer("--info");
            break;
        case DT_WARNING:
            Sys_AppendToExecBuffer("--warning");
            break;
        case DT_ERROR:
            Sys_AppendToExecBuffer("--error");
            break;
        case DT_YES_NO:
            Sys_AppendToExecBuffer("--question");
            Sys_AppendToExecBuffer("--ok-label=Yes");
            Sys_AppendToExecBuffer("--cancel-label=No");
            break;

        case DT_OK_CANCEL:
            Sys_AppendToExecBuffer("--question");
            Sys_AppendToExecBuffer("--ok-label=OK");
            Sys_AppendToExecBuffer("--cancel-label=Cancel");
            break;
    }

    Sys_AppendToExecBuffer(va("--text=%s", message));
    Sys_AppendToExecBuffer(va("--title=%s", title));
}

/*
==============
Sys_KdialogCommand
==============
*/
static void Sys_KdialogCommand(dialogType_t type, const char *message, const char *title) {
    Sys_ClearExecBuffer();
    Sys_AppendToExecBuffer("kdialog");

    switch (type) {
        default:
        case DT_INFO:
            Sys_AppendToExecBuffer("--msgbox");
            break;
        case DT_WARNING:
            Sys_AppendToExecBuffer("--sorry");
            break;
        case DT_ERROR:
            Sys_AppendToExecBuffer("--error");
            break;
        case DT_YES_NO:
            Sys_AppendToExecBuffer("--warningyesno");
            break;
        case DT_OK_CANCEL:
            Sys_AppendToExecBuffer("--warningcontinuecancel");
            break;
    }

    Sys_AppendToExecBuffer(message);
    Sys_AppendToExecBuffer(va("--title=%s", title));
}

/*
==============
Sys_XmessageCommand
==============
*/
static void Sys_XmessageCommand(dialogType_t type, const char *message, const char *title) {
    Sys_ClearExecBuffer();
    Sys_AppendToExecBuffer("xmessage");
    Sys_AppendToExecBuffer("-buttons");

    switch (type) {
        default:
            Sys_AppendToExecBuffer("OK:0");
            break;
        case DT_YES_NO:
            Sys_AppendToExecBuffer("Yes:0,No:1");
            break;
        case DT_OK_CANCEL:
            Sys_AppendToExecBuffer("OK:0,Cancel:1");
            break;
    }

    Sys_AppendToExecBuffer("-center");
    Sys_AppendToExecBuffer(message);
}

/*
==============
Sys_Dialog

Display a *nix dialog box
==============
*/
dialogResult_t Sys_Dialog(dialogType_t type, const char *message, const char *title) {
    typedef enum {
        NONE = 0,
        ZENITY,
        KDIALOG,
        XMESSAGE,
        NUM_DIALOG_PROGRAMS
    } dialogCommandType_t;
    typedef void (*dialogCommandBuilder_t)(dialogType_t, const char *, const char *);

    const char *session = getenv("DESKTOP_SESSION");
    qboolean tried[NUM_DIALOG_PROGRAMS] = {qfalse};
    dialogCommandBuilder_t commands[NUM_DIALOG_PROGRAMS] = {NULL};
    dialogCommandType_t preferredCommandType = NONE;
    int i;

    commands[ZENITY] = &Sys_ZenityCommand;
    commands[KDIALOG] = &Sys_KdialogCommand;
    commands[XMESSAGE] = &Sys_XmessageCommand;

    // This may not be the best way
    if (!Q_stricmp(session, "gnome"))
        preferredCommandType = ZENITY;
    else if (!Q_stricmp(session, "kde"))
        preferredCommandType = KDIALOG;

    for (i = NONE + 1; i < NUM_DIALOG_PROGRAMS; i++) {
        if (preferredCommandType != NONE && preferredCommandType != i)
            continue;

        if (!tried[i]) {
            int exitCode;

            commands[i](type, message, title);
            exitCode = Sys_Exec();

            if (exitCode >= 0) {
                switch (type) {
                    case DT_YES_NO:
                        return exitCode ? DR_NO : DR_YES;
                    case DT_OK_CANCEL:
                        return exitCode ? DR_CANCEL : DR_OK;
                    default:
                        return DR_OK;
                }
            }

            tried[i] = qtrue;

            // The preference failed, so start again in order
            if (preferredCommandType != NONE) {
                preferredCommandType = NONE;
                i = NONE + 1;
            }
        }
    }

    Com_DPrintf(S_COLOR_YELLOW "WARNING: failed to show a dialog\n");
    return DR_OK;
}

#endif

/*
==============
Sys_GLimpSafeInit

Unix specific "safe" GL implementation initialisation
==============
*/
void Sys_GLimpSafeInit(void) {
    // NOP
}

/*
==============
Sys_GLimpInit

Unix specific GL implementation initialisation
==============
*/
void Sys_GLimpInit(void) {
    // NOP
}

void Sys_SetFloatEnv(void) {
    // rounding toward nearest
    // fesetround(FE_TONEAREST);
    // TODO: PSP2 ?
}

/*
==============
Sys_PlatformInit

Unix specific initialisation
==============
*/
void Sys_PlatformInit(void) {
}

/*
==============
Sys_PlatformExit

Unix specific deinitialisation
==============
*/
void Sys_PlatformExit(void) {
}

/*
==============
Sys_SetEnv

set/unset environment variables (empty value removes it)
==============
*/

void Sys_SetEnv(const char *name, const char *value) {
    if (value && *value)
        setenv(name, value, 1);
    else
        unsetenv(name);
}

/*
==============
Sys_PID
==============
*/
int Sys_PID(void) {
    return getpid();
}

/*
==============
Sys_PIDIsRunning
==============
*/
qboolean Sys_PIDIsRunning(int pid) {
    return kill(pid, 0) == 0;
}

/*
=================
Sys_DllExtension

Check if filename should be allowed to be loaded as a DLL.
=================
*/
qboolean Sys_DllExtension(const char *name) {
    const char *p;
    char c = 0;

    if (COM_CompareExtension(name, DLL_EXT)) {
        return qtrue;
    }

    // Check for format of filename.so.1.2.3
    p = strstr(name, DLL_EXT ".");

    if (p) {
        p += strlen(DLL_EXT);

        // Check if .so is only followed for periods and numbers.
        while (*p) {
            c = *p;

            if (!isdigit(c) && c != '.') {
                return qfalse;
            }

            p++;
        }

        // Don't allow filename to end in a period. file.so., file.so.0., etc
        if (c != '.') {
            return qtrue;
        }
    }

    return qfalse;
}
