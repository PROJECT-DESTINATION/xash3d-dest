// Reads .VPK files
#pragma once
#ifndef H_VPK
#define H_VPK
#include "const.h"
#include "filesystem_internal.h"

typedef struct VPKEntry_s
{
    dword crc;
	word bytes;
	word archive_index;
	dword entry_offset;
    dword entry_length;

	word terminator;
} VPKEntry_t;

searchpath_t* FS_AddVPK_Fullpath(const char* vpkfile, int flags);

#endif
