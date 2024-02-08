#include "crtlib.h"
#include "common/com_strings.h"
#include "map/map.h"
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

typedef struct vpk_filepos_s
{
    fs_offset_t data_offset;
    fs_offset_t data_size;
    word part_id;
} vpk_fileEntry_t;

typedef map_t(vpk_fileEntry_t) map_vpk_filepos_t;

struct vpk_s
{
    map_vpk_filepos_t file_entries;
    word parts_count;
    dword file_count;
#ifdef XASH_REDUCE_FD
    string prefix;
#else
    file_t **parts; // 0 is always *_dir.vpk
#endif
    poolhandle_t mempool;
};

int ReadString(file_t *file, char *out, int maxlen)
{
    out[0] = '\x00';
    int len = 0;
    for (; len < maxlen; ++len)
    {
        FS_Read(file, &out[len], 1);
        if (!out[len])
        {
            return len;
        }
    }
    return maxlen;
}

static vpk_t *VPK_Open(const char *filename)
{
    vpk_t *vpk = (vpk_t *) Mem_Calloc(fs_mempool, sizeof(vpk_t));
    string buf;
    string vpk_path;
    string vpk_filename;
    string vpk_extension;
    string vpk_prefix;
    dword len;

    len = strlen(filename);
    if (len < 4)
    {
        Con_Reportf(S_ERROR "VPK_Open: what the hell is wrong with you???? %s has less than 4 chars\n", filename);
        return 0;
    }
    Q_strncpy(vpk_prefix, filename, sizeof(vpk_prefix));
    vpk_prefix[len - 4] = '\x00';

    Q_snprintf(buf, sizeof(buf), "%s_dir.vpk", vpk_prefix);
    if (!FS_FileExists(buf, false))
    {
        Con_Reportf(S_ERROR "VPK_Open: %s does not exist.\n", buf);
        return 0;
    }

    qboolean found;
    do
    {
        Q_snprintf(buf, sizeof(buf), "%s_%03d.vpk", vpk_prefix, vpk->parts_count);
        found = FS_FileExists(buf, false);
        ++vpk->parts_count;
    } while (found);

    Q_snprintf(buf, sizeof(buf), "%s_dir.vpk", vpk_prefix);
    file_t *vpk_dir = FS_Open(buf, "rb", false);

    dword sig;
    FS_Read(vpk_dir, &sig, 4);
    if (sig != 0x55AA1234)
    {
        FS_Close(vpk_dir);
        return 0;
    }
    word ver[2];
    FS_Read(vpk_dir, &ver[0], 4);
    if ((ver[0] != 1 && ver[1] != 0) || (ver[0] != 2 && ver[1] != 0))
    {
        Con_DPrintf("VPK_Open: VPK version(%i, %i) is unsupported.\n", ver[0], ver[1]);
        FS_Close(vpk_dir);
        return 0;
    }
    FS_Seek(vpk_dir, 4, SEEK_CUR);
    if (ver[0] == 2 && ver[1] == 0)
    {
        FS_Seek(vpk_dir, 4 * 4, SEEK_CUR);
    }

#ifdef XASH_REDUCE_FD
    Q_strncpy(vpk->prefix, vpk_prefix, sizeof(vpk->prefix));
#else
    vpk->parts = (file_t **) Mem_Calloc(fs_mempool, sizeof(file_t *) * (vpk->parts_count + 1));

    Q_snprintf(buf, sizeof(buf), "%s_dir.vpk", vpk_prefix);
    vpk->parts[0] = FS_Open(buf, "rb", false);

    for (int i = 0; i < vpk->parts_count; ++i)
    {
        Q_snprintf(buf, sizeof(buf), "%s_%03d.vpk", vpk_prefix, i);
        vpk->parts[i + 1] = FS_Open(buf, "rb", false);
    }
#endif

    map_init(&vpk->file_entries);
    while (true)
    {
        ReadString(vpk_dir, vpk_extension, sizeof(vpk_extension));
        if (!vpk_extension[0])
        {
            break;
        }
        while (true)
        {
            ReadString(vpk_dir, vpk_path, sizeof(vpk_path));
            if (!vpk_path[0])
            {
                break;
            }
            while (true)
            {
                ReadString(vpk_dir, vpk_filename, sizeof(vpk_filename));
                if (!vpk_filename[0])
                {
                    break;
                }
                Q_snprintf(buf, sizeof(buf), "%s/%s.%s", vpk_path, vpk_filename, vpk_extension);

                VPKEntry_t vpk_entry;
                FS_Read(vpk_dir, &vpk_entry, 18);

                vpk_fileEntry_t vpk_tempfilepos;
                vpk_tempfilepos.data_offset = (fs_offset_t) vpk_entry.entry_offset;
                vpk_tempfilepos.part_id = vpk_entry.archive_index + 1;
                vpk_tempfilepos.data_size = (fs_offset_t) vpk_entry.entry_length;
                map_set(&vpk->file_entries, buf, vpk_tempfilepos);
                vpk->file_count++;
            }
        }
    }
    FS_Close(vpk_dir);
    return vpk;
}


/*
===========
VPK_FileTime

===========
*/
static int VPK_FileTime(searchpath_t *search, const char *filename)
{
    (void) search;
    (void) filename;
    return 0;
}

/*
===========
VPK_PrintInfo

===========
*/
static void VPK_PrintInfo(searchpath_t *search, char *dst, size_t size)
{
    Q_snprintf(dst, size, "%s (%i files)", search->filename, search->vpk->file_count);
}


/*
===========
VPK_Search

===========
*/
static void VPK_Search(searchpath_t *search, stringlist_t *list, const char *pattern, int caseinsensitive)
{
    Con_DPrintf("VPK_Search(%s, %i)\n", pattern, caseinsensitive);
    map_iter_t iter = map_iter(&search->vpk->file_entries);
    const char *key, *slash, *backslash, *colon, *separator;
    string temp;
    dword j;
    while ((key = map_next(&search->vpk->file_entries, &iter)))
    {
        Q_strncpy(temp, key, sizeof(temp));

        while (temp[0])
        {
            if (matchpattern(temp, pattern, true))
            {
                for (j = 0; j < list->numstrings; j++)
                {
                    if (!Q_strcmp(list->strings[j], temp))
                        break;
                }

                if (j == list->numstrings)
                    stringlistappend(list, temp);
            }

            // strip off one path element at a time until empty
            // this way directories are added to the listing if they match the pattern
            slash = Q_strrchr(temp, '/');
            backslash = Q_strrchr(temp, '\\');
            colon = Q_strrchr(temp, ':');
            separator = temp;
            if (separator < slash)
                separator = slash;
            if (separator < backslash)
                separator = backslash;
            if (separator < colon)
                separator = colon;
            *((char *) separator) = 0;
        }
    }
}


static byte *VPK_LoadFile(searchpath_t *search, const char *path, int pack_ind, fs_offset_t *lumpsizeptr)
{
    Con_DPrintf("VPK_LoadFile(%s, %s, %i)\n", search->filename, path, pack_ind);
    byte *buf;
    vpk_t *vpk = search->vpk;
    vpk_fileEntry_t *entry = map_get(&vpk->file_entries, path);
    if (!entry)
    {
        Con_DPrintf("VPK_LoadFile(%s, %s, %i) file not found!\n", search->filename, path, pack_ind);
        return NULL;
    }
    buf = (byte *) Mem_Malloc(vpk->mempool, entry->data_size);
#ifdef XASH_REDUCE_FD
    string name_buf;
    if (entry->part_id == 0)
    {
        Q_snprintf(name_buf, sizeof(name_buf), "%s_dir.vpk", search->vpk->prefix);
    } else
    {
        Q_snprintf(name_buf, sizeof(name_buf), "%s_%03d.vpk", search->vpk->prefix, entry->part_id - 1);
    }
    file_t *part_handle = FS_Open(name_buf, "rb", false);
#else
    file_t *part_handle = vpk->parts[entry->part_id];
#endif
    FS_Seek(part_handle, entry->data_offset, SEEK_SET);
    FS_Read(part_handle, buf, entry->data_size);
    *lumpsizeptr = entry->data_size;

#ifdef XASH_REDUCE_FD
    FS_Close(part_handle);
#endif
    return buf;
}

static file_t *VPK_OpenFile(searchpath_t *search, const char *filename, const char *mode, int pack_ind)
{
    (void) search;
    (void) filename;
    (void) mode;
    (void) pack_ind;
    return NULL;
}

static int VPK_FindFile(searchpath_t *search, const char *path, char *fixedname, size_t len)
{
//    Con_DPrintf("VPK_FindFile(%s, %s)\n", search->filename, path);
    vpk_t *vpk = search->vpk;
    vpk_fileEntry_t *filepos = map_get(&vpk->file_entries, path);
    if (!filepos)
    {
        return -1;
    }
    Q_strncpy(fixedname, path, len);
    return 0;
}

static void VPK_Close(searchpath_t *search)
{
    Con_DPrintf("VPK_Close()\n");
    Mem_FreePool(&search->vpk->mempool);
#ifndef XASH_REDUCE_FD
    for (int i = 0; i < search->vpk->parts_count; ++i)
    {
        FS_Close(search->vpk->parts[i]);
    }
    Mem_Free(search->vpk->parts);
#endif
    Mem_Free(search->vpk);
}

/*
====================
FS_AddVPK_Fullpath
====================
*/
searchpath_t *FS_AddVPK_Fullpath(const char *vpkfile, int flags)
{
    searchpath_t *search;
    vpk_t *vpk;
    vpk = VPK_Open(vpkfile);

    if (!vpk)
    {
        Con_Reportf(S_ERROR "FS_AddVPK_Fullpath: unable to load vpk \"%s\"\n", vpkfile);
        return NULL;
    }

    vpk->mempool = Mem_AllocPool(vpkfile);
    search = (searchpath_t *) Mem_Calloc(fs_mempool, sizeof(searchpath_t));
    Q_strncpy(search->filename, vpkfile, sizeof(search->filename));
    search->vpk = vpk;
    search->type = SEARCHPATH_VPK;
    search->flags = flags;

    search->pfnPrintInfo = VPK_PrintInfo;
    search->pfnClose = VPK_Close;
    search->pfnOpenFile = VPK_OpenFile;
    search->pfnFileTime = VPK_FileTime;
    search->pfnFindFile = VPK_FindFile;
    search->pfnSearch = VPK_Search;
    search->pfnLoadFile = VPK_LoadFile;

    Con_Reportf("Adding vpk: %s\n", vpkfile);
    return search;
}
