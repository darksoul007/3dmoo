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

#include "util.h"
#include "handles.h"
#include "mem.h"
#include "arm11.h"

#include "service_macros.h"

u32 lock_handle;
u32 event_handles[2];

u8* APTs_sharedfont = NULL;
size_t APTs_sharedfontsize = 0;

SERVICE_START(apt_s);


SERVICE_CMD(0x20080)
{
    u32 app_id = CMD(1);
    DEBUG("RegisterApp, app_id=%08x\n", app_id);

    // Init some event handles.
    event_handles[0] = handle_New(HANDLE_TYPE_EVENT, HANDLE_SUBEVENT_APTMENUEVENT);
    handleinfo* h = handle_Get(event_handles[0]);
    h->locked = true;
    h->locktype = LOCK_TYPE_ONESHOT;

    event_handles[1] = handle_New(HANDLE_TYPE_EVENT, HANDLE_SUBEVENT_APTPAUSEEVENT);
    h = handle_Get(event_handles[1]);
    h->locked = false; // Fire start event
    h->locktype = LOCK_TYPE_ONESHOT;

    RESP(1, 0); // Result
    RESP(3, event_handles[0]);
    RESP(4, event_handles[1]);
    return 0;
}

SERVICE_CMD(0x30040)
{
    u32 unk = CMD(1);
    DEBUG("Enable, unk=%08x\n", unk);

    RESP(1, 0); // Result
    return 0;
}

SERVICE_CMD(0x3E0080)
{
    u32 unk  = CMD(1);
    u32 unk1 = CMD(2);
    DEBUG("ReplySleepQuery, unk=%08x, unk1=%08x\n", unk, unk1);

    RESP(1, 0); // Result
    return 0;
}

SERVICE_CMD(0x430040)
{
    u32 app_id = CMD(1);
    DEBUG("NotifyToWait, app_id=%08x\n", app_id);

    RESP(1, 0); // Result
    return 0;
}

SERVICE_CMD(0x440000)
{
    DEBUG("GetSharedFont\n");

    // Load shared binary from sys/shared_font.bin
    if(APTs_sharedfont == NULL) {
        FILE* fd = fopen("sys/shared_font.bin", "rb");

        if(fd == NULL) {
            ERROR("No shared font available. Please put one in: sys/shared_font.bin\n");
            RESP(1, -1);
            return 0;
        }

        // Get file size
        fseek(fd, 0, SEEK_END);
        APTs_sharedfontsize = ftell(fd);
        fseek(fd, 0, SEEK_SET);

        // Allocate buffer for font
        APTs_sharedfont = malloc(APTs_sharedfontsize + 4);

        if (APTs_sharedfont == NULL) {
            ERROR("malloc() failed trying to read shared font.\n");
            fclose(fd);
            RESP(1, -1);
            return 0;
        }

        // Read it
        if (fread(APTs_sharedfont + 4, APTs_sharedfontsize, 1, fd) != 1) {
            ERROR("fread() failed trying to read shared font.\n");
            fclose(fd);
            free(APTs_sharedfont);
            APTs_sharedfont = NULL;
            RESP(1, -1);
            return 0;
        }

        APTs_sharedfont[3] = 0x2;
        APTs_sharedfont[2] = 0x0;
        APTs_sharedfont[1] = 0x0;
        APTs_sharedfont[0] = 0x0;

        fclose(fd);
    }

    RESP(1, 0); // Result
    RESP(2, 0xDEAD0000); // mem addr

    // Handle for shared memory
    RESP(4, handle_New(HANDLE_TYPE_SHAREDMEM, MEM_TYPE_APT_S_SHARED_FONT));
    return 0;
}

SERVICE_CMD(0x4b00c2)   //AppletUtility
{
    u32 unk = CMD(1);
    u32 pointeresize = CMD(2);
    u32 pointerzsize = CMD(3);
    u32 pointere = CMD(5);
    u32 pointerz = EXTENDED_CMD(1);
    u8* data = (u8*)malloc(pointeresize + 1);
    u32 i;

    DEBUG("AppletUtility %08x (%08x %08x,%08x %08x)\n", unk, pointere, pointeresize, pointerz, pointerzsize);

    // Dump data.
    mem_Read(data, pointere, pointeresize);

    for (i=0; i<pointeresize; i++) {
        if (i % 16 == 0)
            printf("\n");

        printf("%02x ", data[i]);
    }
    printf("\n");

    free(data);

    RESP(1, 0); // Worked
    return 0;
}

SERVICE_CMD(0xb0040)
{
    u32 appID = CMD(1);
    DEBUG("InquireNotification, appID=%08x\n", appID);

    RESP(1, 0); // Result
    RESP(2, 0); // Signal type (1=home button press, 7=shutdown, 8=pwr button, ..)
    return 0;
}

SERVICE_CMD(0xd0080)
{
    u32 appID = CMD(1);
    u32 bufSize = CMD(2);
    DEBUG("ReceiveParameter, appID=%08x, bufSize=%08x\n", appID, bufSize);

    RESP(1, 0); // Result
    RESP(2, 0); // AppId of triggering process
    RESP(3, 1); // Signal type (1=app just started, 0xb=returning to app, 0xc=exiting app)
    RESP(4, 0x10);
    RESP(5, 0); // Some handle
    RESP(6, 0); // (bufSize<<14) | 2
    RESP(7, 0); // bufPtr

    return 0;
}

SERVICE_CMD(0xe0080)
{
    u32 appID = CMD(1);
    u32 bufSize = CMD(2);
    DEBUG("GlanceParameter, appID=%08x, bufSize=%08x\n", appID, bufSize);

    RESP(1, 0); // Result
    RESP(2, 0); // AppId of triggering process
    RESP(3, 1); // Signal type (1=app just started, 0xb=returning to app, 0xc=exiting app)
    RESP(4, 0x10);
    RESP(5, 0); // Some handle
    RESP(6, 0); // (bufSize<<14) | 2
    RESP(7, 0); // bufPtr

    return 0;
}

SERVICE_END();
