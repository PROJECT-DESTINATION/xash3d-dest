/*
lib_posix.c - dynamic library code for POSIX systems
Copyright (C) 2018 Flying With Gauss

This program is free software: you can redistribute it and/sor modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/
#ifndef _PS3_AAAA
#define _PS3_AAAA
#endif
#include "platform/platform.h"
#include "common.h"
#include "library.h"
#include "filesystem.h"
#include "server.h"
#include <sys/prx.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "menu_int.h"

void *dlsym(void *handle, const char *symbol )
{
	Con_DPrintf( "dlsym( %p, \"%s\" ): stub\n", handle, symbol );
	return NULL;
}

void *dlopen(const char *name, int flag )
{
	Con_DPrintf( "dlopen( \"%s\", %d ): stub\n", name, flag );
	return NULL;
}

int dlclose(void *handle)
{
	Con_DPrintf( "dlsym( %p ): stub\n", handle );
	return 0;
}

char *dlerror( void )
{
	return "Loading ELF libraries not supported in this build!\n";
}


qboolean COM_CheckLibraryDirectDependency( const char *name, const char *depname, qboolean directpath )
{
	// TODO: implement
	return true;
}

void *COM_LoadLibrary( const char *dllname, int build_ordinals_table, qboolean directpath )
{
	dll_user_t *hInst = NULL;
	void *pHandle = NULL;
	char buf[MAX_VA_STRING];
	int mod = NULL;
	Q_snprintf(buf,MAX_VA_STRING,"/dev_hdd0/game/XASH10000/USRDIR/%s",dllname);
	COM_ResetLibraryError();

	// platforms where gameinfo mechanism is working goes here
	// and use FS_FindLibrary
	hInst = FS_FindLibrary( buf, directpath );
	if( !hInst )
	{
		// HACKHACK: direct load dll

		// try to find by linker(LD_LIBRARY_PATH, DYLD_LIBRARY_PATH, LD_32_LIBRARY_PATH and so on...)
		if( !pHandle )
		{
			mod = sys_prx_load_module(buf,0,0);
			if( mod )
			{
				sys_prx_start_module(mod,0,0,&pHandle,0,0);
				return pHandle;
			}
				

			Q_snprintf( buf, sizeof( buf ), "Failed to find library %s", dllname );
			COM_PushLibraryError( buf );
			COM_PushLibraryError( dlerror() );
			return NULL;
		}
	}

	if( hInst->custom_loader )
	{
		Q_snprintf( buf, sizeof( buf ), "Custom library loader is not available. Extract library %s and fix gameinfo.txt!", hInst->fullPath );
		COM_PushLibraryError( buf );
		Mem_Free( hInst );
		return NULL;
	}

	{
		if( !( mod = (void*)sys_prx_load_module(hInst->fullPath,0,0) ) )
		{
			COM_PushLibraryError( dlerror() );
			Mem_Free( hInst );
			return NULL;
		}
		sys_prx_start_module(mod,0,0,&hInst->hInstance,0,0);
	}

	pHandle = hInst->hInstance;

	Mem_Free( hInst );

	return pHandle;
}

void COM_FreeLibrary( void *hInstance )
{
	dlclose( hInstance );
}

void *COM_GetProcAddress( void *hInstance, const char *name )
{
	return dlsym( hInstance, name );
}

void *COM_FunctionFromName( void *hInstance, const char *pName )
{
	return COM_GetProcAddress( hInstance, pName );
}

const char *COM_NameForFunction( void *hInstance, void *function )
{
	return NULL;
}



void Posix_Daemonize( void )
{

}

#include <sys/sys_time.h>
#include <sys/timer.h>
double Platform_DoubleTime( void )
{
	sys_time_sec_t sec;
	sys_time_nsec_t nsec;
	sys_time_get_current_time(&sec,&nsec);
	return (double)sec + (double)nsec/1000000000.0;
}
void Platform_Sleep( int msec )
{
	sys_timer_usleep( msec * 1000 );
}


const char* getenv(const char* name)
{
	return NULL;
}


const char* getcwd(const char* name)
{
	return NULL;
}

#if XASH_DEDICATED
#include "ref_common.h"
struct ref_state_s ref;
#endif