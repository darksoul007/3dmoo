/*
* Copyright (C) 2014 - plutoo
* Copyright (C) 2014 - ichfly
* Copyright (C) 2014 - Normmatt
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "util.h"
#include "mem.h"
#include "handles.h"
#include "fs.h"
#include "loader.h"

/* ____ Extended Save Data implementation ____ */

static u32 extsavedatafile_Read(file_type* self, u32 ptr, u32 sz, u64 off, u32* read_out)
{
    FILE* fd = self->type_specific.sysdata.fd;
    *read_out = 0;

    if(off >> 32) {
        ERROR("64-bit offset not supported.\n");
        return -1;
    }

    if(fseek64(fd, off, SEEK_SET) == -1) {
        ERROR("fseek failed.\n");
        return -1;
    }

    u8* b = malloc(sz);
    if(b == NULL) {
        ERROR("Not enough mem.\n");
        return -1;
    }

    u32 read = fread(b, 1, sz, fd);
    if(read != sz) {
        ERROR("fread failed\n");
        free(b);
        return -1;
    }

    if(mem_Write(b, ptr, read) != 0) {
        ERROR("mem_Write failed.\n");
        free(b);
        return -1;
    }

    *read_out = read;
    free(b);

    return 0; // Result
}

static u32 extsavedatafile_Write(file_type* self, u32 ptr, u32 sz, u64 off, u32 flush_flags, u32* written_out)
{
    FILE* fd = self->type_specific.sysdata.fd;
    *written_out = 0;

    if (off >> 32) {
        ERROR("64-bit offset not supported.\n");
        return -1;
    }

    if (fseek64(fd, off, SEEK_SET) == -1) {
        ERROR("fseek failed.\n");
        return -1;
    }

    u8* b = malloc(sz);
    if (b == NULL) {
        ERROR("Not enough mem.\n");
        return -1;
    }

    if (mem_Read(b, ptr, sz) != 0) {
        ERROR("mem_Read failed.\n");
        free(b);
        return -1;
    }

    u32 written = fwrite(b, 1, sz, fd);
    if(written != sz) {
        ERROR("fwrite failed\n");
        free(b);
        return -1;
    }

    *written_out = written;
    free(b);

    return 0; // Result
}

static u64 extsavedatafile_GetSize(file_type* self)
{
    return self->type_specific.sysdata.sz;
}

static u32 extsavedatafile_SetSize(file_type* self, u64 sz)
{
    FILE* fd = self->type_specific.sysdata.fd;

    self->type_specific.sysdata.sz = sz;
    if(ftruncate(fileno(fd), sz) == -1) {
        ERROR("ftruncate failed.\n");
        return -1;
    }

    return 0;
}

static u32 extsavedatafile_Close(file_type* self)
{
    // Close file and free yourself
    fclose(self->type_specific.sysdata.fd);
    free(self);

    return 0;
}



/* ____ FS implementation ____ */

static bool extsavedata_FileExists(archive* self, file_path path)
{
    char p[256], tmp[256];
    struct stat st;

    // Generate path on host file system
    snprintf(p, 256, "extsavedata/%s/%s",
             loader_h.productcode,
             fs_PathToString(path.type, path.ptr, path.size, tmp, 256));

    if(!fs_IsSafePath(p)) {
        ERROR("Got unsafe path.\n");
        return false;
    }

    return stat(p, &st) == 0;
}

static u32 extsavedata_CreateFile(archive* self, file_path path, u32 size)
{
    char *p = malloc(256);

    char tmp[256];

    // Generate path on host file system
    snprintf(p, 256, "extsavedata/%s/%s",
             loader_h.productcode, fs_PathToString(path.type, path.ptr, path.size, tmp, 256));

    if(!fs_IsSafePath(p)) {
        ERROR("Got unsafe path.\n");
        free(p);
        return 0;
    }

    int result;

    int fd = open(p, O_CREAT | O_EXCL | O_WRONLY, S_IRUSR | S_IWUSR);

    if(fd == -1)
    {
        result = errno;
        if(result == EEXIST) result = 0x82044BE;
    }
    else {
        result = ftruncate(fd, size);
        if(result != 0) result = errno;
        close(fd);
    }

    if(result == ENOSPC) result = 0x86044D2;
    free(p);
    return result;
}

static u32 extsavedata_OpenFile(archive* self, file_path path, u32 flags, u32 attr)
{
    char p[256], tmp[256];

    // Generate path on host file system
    snprintf(p, 256, "extsavedata/%s/%s",
             loader_h.productcode,
             fs_PathToString(path.type, path.ptr, path.size, tmp, 256));

    if(!fs_IsSafePath(p)) {
        ERROR("Got unsafe path.\n");
        return 0;
    }

    char mode[10];
    memset(mode, 0, 10);

    switch (flags) {
    case 1: //R
        strcpy(mode, "rb");
        break;
    case 2: //W
    case 3: //RW
        strcpy(mode, "rb+");
        break;
    case 4: //C
    case 6: //W+C
        strcpy(mode, "wb");
        break;
    }

    FILE* fd = fopen(p, mode);

    if(fd == NULL) {
        ERROR("Failed to open extsavedata, path=%s\n", p);
        return 0;
    }

    fseek(fd, 0, SEEK_END);
    u32 sz;

    if((sz=ftell(fd)) == -1) {
        ERROR("ftell() failed.\n");
        fclose(fd);
        return 0;
    }

    // Create file object
    file_type* file = calloc(sizeof(file_type), 1);

    if(file == NULL) {
        ERROR("calloc() failed.\n");
        fclose(fd);
        return 0;
    }

    file->type_specific.sysdata.fd = fd;
    file->type_specific.sysdata.sz = (u64) sz;

    // Setup function pointers.
    file->fnRead = &extsavedatafile_Read;
    file->fnWrite = &extsavedatafile_Write;
    file->fnGetSize = &extsavedatafile_GetSize;
    file->fnSetSize = &extsavedatafile_SetSize;
    file->fnClose = &extsavedatafile_Close;

    file->handle = handle_New(HANDLE_TYPE_FILE, (uintptr_t)file);
    return file->handle;
}

u32 extsavedata_ReadDir(dir_type* self, u32 ptr, u32 entrycount, u32* read_out)
{
    u32 current = 0;
    struct dirent* ent;

    rewinddir(self->dir);

    while(current < entrycount) {
        errno = 0;
        if((ent = readdir(self->dir)) == NULL) {
            if(errno == 0) {
                // Dir empty. Memset region?
                return 0;
            }
            else {
                ERROR("readdir() failed.\n");
                return -1;
            }
        }
    }

    // Convert file-name to UTF16 and copy.
    u16 utf16[256];
    for(u32 i = 0; i < 256; i++)
        utf16[i] = ent->d_name[i];

    mem_Write((uint8_t*)utf16, ptr, sizeof(utf16));

    // XXX: 8.3 name @ 0x20C
    mem_Write8(ptr + 0x215, 0xA); //unknown
    // XXX: 8.3 ext @ 0x216

    mem_Write8(ptr + 0x21A, 0x1); // Unknown
    mem_Write8(ptr + 0x21B, 0x0); // Unknown

    struct stat st;

    char path[256];
    snprintf(path, sizeof(path), "%s/%s", self->path, ent->d_name);
    if (stat(path,&st) != 0 ) {
        ERROR("Failed to stat: %s\n", path);
        return -1;
    }
    // Is directory flag
    if (S_ISDIR(st.st_mode)) {
        mem_Write8(ptr + 0x21C, 0x1);

        mem_Write8(ptr + 0x216, ' '); // 8.3 file extension
        mem_Write8(ptr + 0x217, ' ');
        mem_Write8(ptr + 0x218, ' ');
    }
    else { // Is directory flag

    // XXX: hidden flag @ 0x21D
    // XXX: archive flag @ 0x21E
    // XXX: readonly flag @ 0x21F

        mem_Write8(ptr + 0x21C, 0x0);
        mem_Write32(ptr + 0x220, st.st_size);

#if defined(ENV64BIT)
        mem_Write32(ptr + 0x224, (st.st_size >> 32));
#else
        mem_Write32(ptr + 0x224, 0);
#endif
    }


    *read_out = current;
    return 0;
}

static u32 extsavedata_OpenDir(archive* self, file_path path)
{
    // Create file object
    dir_type* dir = (dir_type*)malloc(sizeof(dir_type));

    dir->f_path = path;
    dir->self = self;

    // Setup function pointers.
    dir->fnRead = &extsavedata_ReadDir;

    char tmp[256];
    snprintf(dir->path, 256, "extsavedata/%s/%s",
             loader_h.productcode, fs_PathToString(path.type, path.ptr, path.size, tmp, 256));

    dir->dir = opendir(dir->path);

    if(dir->dir == NULL) {
        ERROR("Dir not found: %s.\n", dir->path);
        free(dir);
        return 0;
    }

    return handle_New(HANDLE_TYPE_DIR, (uintptr_t)dir);
}

int extsavedata_CreateDir(archive* self, file_path path)
{
    char p[256], tmp[256];

    // Generate path on host file system
    snprintf(p, 256, "extsavedata/%s/%s",
             loader_h.productcode, fs_PathToString(path.type, path.ptr, path.size, tmp, 256));

    if(!fs_IsSafePath(p)) {
        ERROR("Got unsafe path.\n");
        return 0;
    }
#ifdef _WIN32
    return _mkdir(p);
#else
    return mkdir(p, 0777);
#endif
}

int extsavedata_DeleteDir(archive* self, file_path path)
{
    char p[256], tmp[256];

    // Generate path on host file system
    snprintf(p, 256, "extsavedata/%s/%s",
             loader_h.productcode, fs_PathToString(path.type, path.ptr, path.size, tmp, 256));

    if(!fs_IsSafePath(p)) {
        ERROR("Got unsafe path.\n");
        return 0;
    }
#ifdef _WIN32
    return _rmdir(p);
#else
    return rmdir(p);
#endif
}

static void extsavedata_Deinitialize(archive* self)
{
    // Free yourself
    free(self);
}

static u32 extsavedata_GetResult()
{
    char p[256];
    struct stat st;

    // Generate path on host file system
    snprintf(p, 256, "extsavedata/%s", loader_h.productcode);

    if(!fs_IsSafePath(p)) {
        ERROR("Got unsafe path.\n");
        return false;
    }

    if(stat(p, &st) == 0)
        return 0;
    else
        return 0xc8a04554; //Not formatted?
}

archive* extsavedata_OpenArchive(file_path path)
{
    archive* arch = calloc(sizeof(archive), 1);

    if(arch == NULL) {
        ERROR("malloc failed.\n");
        return NULL;
    }

    // Setup function pointers
    arch->fnCreateDir = &extsavedata_CreateDir;
    arch->fnOpenDir = &extsavedata_OpenDir;
    arch->fnDeleteDir = NULL;
    arch->fnRenameDir = NULL;
    arch->fnFileExists = &extsavedata_FileExists;
    arch->fnCreateFile = &extsavedata_CreateFile;
    arch->fnOpenFile = &extsavedata_OpenFile;
    arch->fnRenameFile = NULL;
    arch->fnDeleteFile = &extsavedata_DeleteDir;
    arch->fnDeinitialize = &extsavedata_Deinitialize;

    snprintf(arch->type_specific.sysdata.path,
             sizeof(arch->type_specific.sysdata.path),
             "");

    arch->result = extsavedata_GetResult();
    return arch;
}
