/*
 * Copyright (C) 2014 - plutoo
 * Copyright (C) 2014 - ichfly
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


int mem_AddSegment(uint32_t base, uint32_t size, uint8_t* data);
int mem_Write8(uint32_t addr, uint8_t w);
u8  mem_Read8(uint32_t addr);
int mem_Write16(uint32_t addr, uint16_t w);
u16 mem_Read16(uint32_t addr);
int mem_Write32(uint32_t addr, uint32_t w);
u32 mem_Read32(uint32_t addr);
int mem_Write(const uint8_t* in_buff, uint32_t addr, uint32_t size);
int mem_Read(uint8_t* buf_out, uint32_t addr, uint32_t size);
int mem_AddMappingShared(uint32_t base, uint32_t size, u8* data, u32 perm, u32 status);
bool mem_test(uint32_t addr);
u8* mem_rawaddr(uint32_t addr, uint32_t size);
void mem_Dbugdump();

#ifdef MEM_REORDER
void mem_Reorder();
#endif

typedef struct {
    uint32_t base;
    uint32_t size;
    uint8_t* phys;
    u32      permition;
    u32      State;
#ifdef MEM_TRACE_EXTERNAL
    bool enable_log;
#endif
#ifdef MEM_REORDER
    u64 accesses;
#endif

} memmap_t;

#define MAX_MAPPINGS 32

#ifdef MODULE_SUPPORT
void ModuleSupport_MemInit(u32 modulenum);
void ModuleSupport_Memadd(u32 modulenum, u32 codeset_handle);
int ModuleSupport_mem_AddMappingShared(uint32_t base, uint32_t size, u8* data, u32 process, u32 perm, u32 status);
#define PERM_NONE 0
#define PERM_R 1
#define PERM_W 2
#define PERM_RW 3
#define PERM_RX 5

#define STAT_IO       2
#define STAT_CODE     4
#define STAT_PRIVAT   5
#define STAT_SHARED   6

#endif