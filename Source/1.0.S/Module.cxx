/*
Copyright (c) 2024 Americus Maximus

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "Module.hxx"

#define MAX_MODULE_NAME_PATH_LENGTH 128

// 0x100010d0
BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID)
{
    if (reason != DLL_PROCESS_ATTACH) { return TRUE; }

    State.Instance = instance;

    char name[MAX_MODULE_NAME_PATH_LENGTH];
    GetModuleFileNameA(instance, name, MAX_MODULE_NAME_PATH_LENGTH - 1);

    const u32 length = radstrlen(name);

    for (u32 x = length; x != 0; x--)
    {
        if (name[x] == ':') { break; } // Stop when disk identifier is found, i.e. C:

        if (name[x] == '\\') { name[x] = NULL; break; } // Truncate path at the last backslash.
    }

    char directory[MAX_MODULE_NAME_PATH_LENGTH];
    GetWindowsDirectoryA(directory, MAX_MODULE_NAME_PATH_LENGTH - 1);

    if (radstricmp(name, directory) != 0)
    {
        GetSystemDirectoryA(directory, MAX_MODULE_NAME_PATH_LENGTH - 1);

        if (radstricmp(name, directory) != 0) { return TRUE; }
    }

    MessageBoxA(NULL, "The BinkW32.DLL file is incorrectly installed in the Windows or Windows system directory.", "Bink Error", MB_ICONHAND | MB_OK);

    return FALSE;
}

// 0x100072a0
void RADEXPLINK BinkBufferBlit(HBINKBUFFER buf, BINKRECT PTR4* rects, u32 numrects)
{
    // TODO NOT IMPLEMENTED
}

// 0x10005b50
void RADEXPLINK BinkBufferCheckWinPos(HBINKBUFFER buf, s32 PTR4* NewWindowX, s32 PTR4* NewWindowY)
{
    // TODO NOT IMPLEMENTED
}

// 0x100078d0
s32 RADEXPLINK BinkBufferClear(HBINKBUFFER buf, u32 RGB)
{
    // TODO NOT IMPLEMENTED

    return TRUE;
}

// 0x10007040
void RADEXPLINK BinkBufferClose(HBINKBUFFER buf)
{
    // TODO NOT IMPLEMENTED
}

// 0x10007740
char PTR4* RADEXPLINK BinkBufferGetDescription(HBINKBUFFER buf)
{
    // TODO NOT IMPLEMENTED

    return NULL;
}

// 0x100078c0
char PTR4* RADEXPLINK BinkBufferGetError()
{
    // TODO NOT IMPLEMENTED

    return NULL;
}

// 0x10007100
s32 RADEXPLINK BinkBufferLock(HBINKBUFFER buf)
{
    // TODO NOT IMPLEMENTED

    return TRUE;
}

// 0x10005df0
HBINKBUFFER RADEXPLINK BinkBufferOpen(HWND wnd, u32 width, u32 height, u32 bufferflags)
{
    // TODO NOT IMPLEMENTED

    return NULL;
}

// 0x10005430
s32 RADEXPLINK BinkBufferSetDirectDraw(void PTR4* lpDirectDraw, void PTR4* lpPrimary)
{
    // TODO NOT IMPLEMENTED

    return TRUE;
}

// 0x10007730
s32 RADEXPLINK BinkBufferSetHWND(HBINKBUFFER buf, HWND newwnd)
{
    return FALSE;
}

// 0x10005c00
s32 RADEXPLINK BinkBufferSetOffset(HBINKBUFFER buf, s32 destx, s32 desty)
{
    // TODO NOT IMPLEMENTED

    return TRUE;
}

// 0x10005b30
void RADEXPLINK BinkBufferSetResolution(s32 w, s32 h, s32 bits)
{
    // TODO NOT IMPLEMENTED
}

// 0x10007620
s32 RADEXPLINK BinkBufferSetScale(HBINKBUFFER buf, u32 w, u32 h)
{
    // TODO NOT IMPLEMENTED

    return TRUE;
}

// 0x10007230
s32 RADEXPLINK BinkBufferUnlock(HBINKBUFFER buf)
{
    // TODO NOT IMPLEMENTED

    return TRUE;
}

// 0x10005a50
s32 RADEXPLINK BinkCheckCursor(HWND wnd, s32 x, s32 y, s32 w, s32 h)
{
    // TODO NOT IMPLEMENTED

    return 0;
}

// 0x1000a0b0
void RADEXPLINK BinkClose(HBINK bnk)
{
    // TODO NOT IMPLEMENTED
}

// 0x1000af60
void RADEXPLINK BinkCloseTrack(HBINKTRACK bnkt)
{
    // TODO NOT IMPLEMENTED
}

// 0x10008df0
s32 RADEXPLINK BinkCopyToBuffer(HBINK bnk, void* dest, s32 destpitch, u32 destheight, u32 destx, u32 desty, u32 flags)
{
    // TODO NOT IMPLEMENTED

    return TRUE;
}

// 0x10007920
s32 RADEXPLINK BinkDDSurfaceType(void PTR4* lpDDS)
{
    // TODO NOT IMPLEMENTED

    return -1;
}

// 0x10009990
s32 RADEXPLINK BinkDoFrame(HBINK bnk)
{
    // TODO NOT IMPLEMENTED

    return TRUE;
}

// 0x10007be0
char PTR4* RADEXPLINK BinkGetError(void)
{
    // TODO NOT IMPLEMENTED

    return NULL;
}

// 0x10009f10
u32 RADEXPLINK BinkGetKeyFrame(HBINK bnk, u32 frame, s32 flags)
{
    // TODO NOT IMPLEMENTED

    return 0;
}

// 0x1000a5d0
void RADEXPLINK BinkGetRealtime(HBINK bink, BINKREALTIME PTR4* run, u32 frames)
{
    // TODO NOT IMPLEMENTED
}

// 0x1000a700
s32 RADEXPLINK BinkGetRects(HBINK bnk, u32 flags)
{
    // TODO NOT IMPLEMENTED

    return 0;
}

// 0x1000a420
void RADEXPLINK BinkGetSummary(HBINK bnk, BINKSUMMARY PTR4* sum)
{
    // TODO NOT IMPLEMENTED
}

// 0x1000af90
u32 RADEXPLINK BinkGetTrackData(HBINKTRACK bnkt, void PTR4* dest)
{
    // TODO NOT IMPLEMENTED

    return 0;
}

// 0x1000ae60
u32 RADEXPLINK BinkGetTrackID(HBINK bnk, u32 trackindex)
{
    if (bnk != NULL) { return bnk->trackIDs[trackindex]; }

    return 0;
}

// 0x1000ae40
u32 RADEXPLINK BinkGetTrackMaxSize(HBINK bnk, u32 trackindex)
{
    if (bnk != NULL) { return bnk->tracksizes[trackindex]; }

    return 0;
}

// 0x1000ae20
u32 RADEXPLINK BinkGetTrackType(HBINK bnk, u32 trackindex)
{
    if (bnk != NULL) { return bnk->tracktypes[trackindex]; }

    return 0;
}

// 0x10009ff0
void RADEXPLINK BinkGoto(HBINK bnk, u32 frame, s32 flags)
{
    // TODO NOT IMPLEMENTED
}

// 0x10005720
s32 RADEXPLINK BinkIsSoftwareCursor(void PTR4* lpDDSP, HCURSOR cur)
{
    // TODO NOT IMPLEMENTED

    return FALSE;
}

// 0x1000ae10
void PTR4* RADEXPLINK BinkLogoAddress(void)
{
    // TODO NOT IMPLEMENTED

    return NULL;
}

// 0x10009e90
void RADEXPLINK BinkNextFrame(HBINK bnk)
{
    // TODO NOT IMPLEMENTED
}

// 0x10007ca0
HBINK RADEXPLINK BinkOpen(const char PTR4* name, u32 flags)
{
    // TODO NOT IMPLEMENTED

    return NULL;
}

// 0x1002a710
BINKSNDOPEN RADEXPLINK BinkOpenDirectSound(u32 param)
{
    // TODO NOT IMPLEMENTED

    return NULL;
}

// 0x10028d60
BINKSNDOPEN RADEXPLINK BinkOpenMiles(u32 param)
{
    // TODO NOT IMPLEMENTED

    return NULL;
}

// 0x1000ae80
HBINKTRACK RADEXPLINK BinkOpenTrack(HBINK bnk, u32 trackindex)
{
    // TODO NOT IMPLEMENTED

    return NULL;
}

// 0x1002b1a0
BINKSNDOPEN RADEXPLINK BinkOpenWaveOut(u32 param)
{
    // TODO NOT IMPLEMENTED

    return NULL;
}

// 0x1000a390
s32 RADEXPLINK BinkPause(HBINK bnk, s32 pause)
{
    // TODO NOT IMPLEMENTED

    return FALSE;
}

// 0x10005b00
void RADEXPLINK BinkRestoreCursor(s32 checkcount)
{
    if (checkcount != 0)
    {
        do
        {
            ShowCursor(TRUE);

            checkcount = checkcount - 1;
        } while (checkcount != 0);
    }
}

// 0x1000ad90
void RADEXPLINK BinkService(HBINK bink)
{
    // TODO NOT IMPLEMENTED
}

// 0x10007bb0
void RADEXPLINK BinkSetError(const char PTR4* err)
{
    // TODO NOT IMPLEMENTED
}

// 0x10007c40
void RADEXPLINK BinkSetFrameRate(u32 forcerate, u32 forceratediv)
{
    // TODO NOT IMPLEMENTED
}

// 0x10007c70
void RADEXPLINK BinkSetIO(BINKIOOPEN io)
{
    // TODO NOT IMPLEMENTED
}

// 0x10007c60
void RADEXPLINK BinkSetIOSize(u32 iosize)
{
    // TODO NOT IMPLEMENTED
}

// 0x1000ade0
void RADEXPLINK BinkSetPan(HBINK bnk, s32 pan)
{
    if (bnk != NULL && bnk->trackindex != -1 && bnk->bsnd.Pan != NULL) { bnk->bsnd.Pan(&bnk->bsnd, pan); }
}

// 0x10007c80
void RADEXPLINK BinkSetSimulate(u32 sim)
{
    // TODO NOT IMPLEMENTED
}

// 0x1000b0d0
s32 RADEXPLINK BinkSetSoundOnOff(HBINK bnk, s32 onoff)
{
    // TODO NOT IMPLEMENTED

    return 0;
}

// 0x10007bf0
s32 RADEXPLINK BinkSetSoundSystem(BINKSNDSYSOPEN open, u32 param)
{
    // TODO NOT IMPLEMENTED

    return FALSE;
}

// 0x10007c90
void RADEXPLINK BinkSetSoundTrack(u32 track)
{
    // TODO NOT IMPLEMENTED
}

// 0x1000b0b0
s32 RADEXPLINK BinkSetVideoOnOff(HBINK bnk, s32 onoff)
{
    if (bnk != NULL) { bnk->videoon = onoff; }

    return onoff;
}

// 0x1000adb0
void RADEXPLINK BinkSetVolume(HBINK bnk, s32 volume)
{
    if (bnk != NULL && bnk->trackindex != -1 && bnk->bsnd.Volume != NULL) { bnk->bsnd.Volume(&bnk->bsnd, volume); }
}

// 0x1000a190
s32 RADEXPLINK BinkWait(HBINK bnk)
{
    // TODO NOT IMPLEMENTED

    return FALSE;
}

// 0x100026e0
void RADEXPLINK ExpandBink(u32 param_1, u32 param_2, u32 param_3, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12, u32 param_13, u32 param_14)
{
    // TODO NOT IMPLEMENTED
}

// 0x100025e0
void RADEXPLINK ExpandBundleSizes(u32 param_1, u32 param_2)
{
    // TODO NOT IMPLEMENTED
}

// 0x10001000
void RADEXPLINK RADSetMemory(RADMEMALLOC a, RADMEMFREE f)
{
    // TODO NOT IMPLEMENTED
}

// 0x1002b7f0
u32 RADEXPLINK RADTimerRead(void)
{
    // TODO NOT IMPLEMENTED

    return 0;
}

// 0x10014130
void RADEXPLINK YUV_blit_16a1bpp(u32 param_1, u32 param_2, u32 param_3, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12, u32 param_13)
{
    // TODO NOT IMPLEMENTED
}

// 0x10016670
void RADEXPLINK YUV_blit_16a1bpp_mask(u32 param_1, u32 param_2, u32 param_3, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12, u32 param_13)
{
    // TODO NOT IMPLEMENTED
}

// 0x10013590
void RADEXPLINK YUV_blit_16a4bpp(u32 param_1, u32 param_2, u32 param_3, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12, u32 param_13)
{
    // TODO NOT IMPLEMENTED
}

// 0x100166d0
void RADEXPLINK YUV_blit_16a4bpp_mask(u32 param_1, u32 param_2, u32 param_3, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12, u32 param_13)
{
    // TODO NOT IMPLEMENTED
}

// 0x10012570
void RADEXPLINK YUV_blit_16bpp(u32 param_1, u32 param_2, u32 param_3, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12)
{
    // TODO NOT IMPLEMENTED
}

// 0x10016620
void RADEXPLINK YUV_blit_16bpp_mask(u32 param_1, u32 param_2, u32 param_3, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12)
{
    // TODO NOT IMPLEMENTED
}

// 0x10011790
void RADEXPLINK YUV_blit_24bpp(u32 param_1, u32 param_2, u32 param_3, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12)
{
    // TODO NOT IMPLEMENTED
}

// 0x10016580
void RADEXPLINK YUV_blit_24bpp_mask(u32 param_1, u32 param_2, u32 param_3, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12)
{
    // TODO NOT IMPLEMENTED
}

// 0x100117e0
void RADEXPLINK YUV_blit_24rbpp(u32 param_1, u32 param_2, u32 param_3, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12)
{
    // TODO NOT IMPLEMENTED
}

// 0x100165d0
void RADEXPLINK YUV_blit_24rbpp_mask(u32 param_1, u32 param_2, u32 param_3, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12)
{
    // TODO NOT IMPLEMENTED
}

// 0x1000f9a0
void RADEXPLINK YUV_blit_32abpp(u32 param_1, u32 param_2, u32 param_3, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12, u32 param_13)
{
    // TODO NOT IMPLEMENTED
}

// 0x100164c0
void RADEXPLINK YUV_blit_32abpp_mask(u32 param_1, u32 param_2, u32 param_3, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12, u32 param_13)
{
    // TODO NOT IMPLEMENTED
}

// 0x1000cf60
void RADEXPLINK YUV_blit_32bpp(u32 param_1, u32 param_2, u32 param_3, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12)
{
    // TODO NOT IMPLEMENTED
}

// 0x10015c40
void RADEXPLINK YUV_blit_32bpp_mask(u32 param_1, u32 param_2, u32 param_3, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12)
{
    // TODO NOT IMPLEMENTED
}

// 0x1000fa00
void RADEXPLINK YUV_blit_32rabpp(u32 param_1, u32 param_2, u32 param_3, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12, u32 param_13)
{
    // TODO NOT IMPLEMENTED
}

// 0x10016520
void RADEXPLINK YUV_blit_32rabpp_mask(u32 param_1, u32 param_2, u32 param_3, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12, u32 param_13)
{
    // TODO NOT IMPLEMENTED
}

// 0x1000e1f0
void RADEXPLINK YUV_blit_32rbpp(u32 param_1, u32 param_2, u32 param_3, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12)
{
    // TODO NOT IMPLEMENTED
}

// 0x10016470
void RADEXPLINK YUV_blit_32rbpp_mask(u32 param_1, u32 param_2, u32 param_3, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12)
{
    // TODO NOT IMPLEMENTED
}

// 0x10015900
void RADEXPLINK YUV_blit_UYVY(u32 param_1, u32 param_2, u32 param_3, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12)
{
    // TODO NOT IMPLEMENTED
}

// 0x10016780
void RADEXPLINK YUV_blit_UYVY_mask(u32 param_1, u32 param_2, u32 param_3, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12)
{
    // TODO NOT IMPLEMENTED
}

// 0x10014cc0
void RADEXPLINK YUV_blit_YUY2(u32 param_1, u32 param_2, u32 param_3, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12)
{
    // TODO NOT IMPLEMENTED
}

// 0x10016730
void RADEXPLINK YUV_blit_YUY2_mask(u32 param_1, u32 param_2, u32 param_3, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12)
{
    // TODO NOT IMPLEMENTED
}

// 0x10015990
void RADEXPLINK YUV_blit_YV12(u32 param_1, u32 param_2, u32 param_3, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12, u32 param_13)
{
    // TODO NOT IMPLEMENTED
}

// 0x1000b2c0
void RADEXPLINK YUV_init(u32 flags)
{
    // TODO NOT IMPLEMENTED
}

// 0x10001020
void PTR4* RADEXPLINK radmalloc(u32 numbytes)
{
    // TODO NOT IMPLEMENTED

    return NULL;
}

// 0x100010a0
void RADEXPLINK radfree(void PTR4* ptr)
{
    // TODO NOT IMPLEMENTED
}