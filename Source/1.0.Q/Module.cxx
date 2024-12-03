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

#include "Cursor.hxx"
#include "Module.hxx"

#define MAX_MODULE_NAME_PATH_LENGTH 128

// 0x100010d0
BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved)
{
    if (reason != DLL_PROCESS_ATTACH) { return TRUE; }

    State.Instance = instance;

    char name[MAX_MODULE_NAME_PATH_LENGTH];
    GetModuleFileNameA(instance, name, MAX_MODULE_NAME_PATH_LENGTH);

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

// 0x10007500
void RADEXPLINK BinkBufferBlit(HBINKBUFFER buf, BINKRECT PTR4* rects, u32 numrects)
{
    // TODO NOT IMPLEMENTED
}

// 0x10005db0
void RADEXPLINK BinkBufferCheckWinPos(HBINKBUFFER buf, s32 PTR4* NewWindowX, s32 PTR4* NewWindowY)
{
    // TODO NOT IMPLEMENTED
}

// 0x10007b30
s32 RADEXPLINK BinkBufferClear(HBINKBUFFER buf, u32 RGB)
{
    // TODO NOT IMPLEMENTED

    return TRUE;
}

// 0x100072a0
void RADEXPLINK BinkBufferClose(HBINKBUFFER buf)
{
    // TODO NOT IMPLEMENTED
}

// 0x100079a0
char PTR4* RADEXPLINK BinkBufferGetDescription(HBINKBUFFER buf)
{
    // TODO NOT IMPLEMENTED

    return NULL;
}

// 0x10007b20
char PTR4* RADEXPLINK BinkBufferGetError()
{
    // TODO NOT IMPLEMENTED

    return NULL;
}

// 0x10007360
s32 RADEXPLINK BinkBufferLock(HBINKBUFFER buf)
{
    // TODO NOT IMPLEMENTED

    return TRUE;
}

// 0x10006050
HBINKBUFFER RADEXPLINK BinkBufferOpen(HWND wnd, u32 width, u32 height, u32 bufferflags)
{
    // TODO NOT IMPLEMENTED

    return NULL;
}

// 0x10005690
s32 RADEXPLINK BinkBufferSetDirectDraw(void PTR4* lpDirectDraw, void PTR4* lpPrimary)
{
    // TODO NOT IMPLEMENTED

    return TRUE;
}

// 0x10007990
s32 RADEXPLINK BinkBufferSetHWND(HBINKBUFFER buf, HWND newwnd)
{
    return FALSE;
}

// 0x10005e60
s32 RADEXPLINK BinkBufferSetOffset(HBINKBUFFER buf, s32 destx, s32 desty)
{
    // TODO NOT IMPLEMENTED

    return TRUE;
}

// 0x10005d90
void RADEXPLINK BinkBufferSetResolution(s32 w, s32 h, s32 bits)
{
    // TODO NOT IMPLEMENTED
}

// 0x10007880
s32 RADEXPLINK BinkBufferSetScale(HBINKBUFFER buf, u32 w, u32 h)
{
    // TODO NOT IMPLEMENTED

    return TRUE;
}

// 0x10007490
s32 RADEXPLINK BinkBufferUnlock(HBINKBUFFER buf)
{
    // TODO NOT IMPLEMENTED

    return TRUE;
}

// 0x10005cb0
s32 RADEXPLINK BinkCheckCursor(HWND wnd, s32 x, s32 y, s32 w, s32 h)
{
    if (CursorState.Width == 0) {
        CursorState.Width = GetSystemMetrics(SM_CXCURSOR);
        CursorState.Height = GetSystemMetrics(SM_CYCURSOR);
    }

    POINT window = { x, y };
    if (wnd != NULL) { ClientToScreen(wnd, &window); }

    POINT cursor;
    GetCursorPos(&cursor);

    s32 result = 0;
    if (window.x < (CursorState.Width + cursor.x) && cursor.x < (window.x + w)
        && window.y < (CursorState.Height + cursor.y) && cursor.y < (window.y + h)) {
        do {
            result = result + 1;
        } while (-1 < ShowCursor(FALSE));
    }

    return result;
}

// 0x1000a2e0
void RADEXPLINK BinkClose(HBINK bnk)
{
    // TODO NOT IMPLEMENTED
}

// 0x1000b170
void RADEXPLINK BinkCloseTrack(HBINKTRACK bnkt)
{
    // TODO NOT IMPLEMENTED
}

// 0x10009040
s32 RADEXPLINK BinkCopyToBuffer(HBINK bnk, void* dest, s32 destpitch, u32 destheight, u32 destx, u32 desty, u32 flags)
{
    // TODO NOT IMPLEMENTED

    return TRUE;
}

// 0x10007b80
s32 RADEXPLINK BinkDDSurfaceType(void PTR4* lpDDS)
{
    // TODO NOT IMPLEMENTED

    return -1;
}

// 0x10009bc0
s32 RADEXPLINK BinkDoFrame(HBINK bnk)
{
    // TODO NOT IMPLEMENTED

    return TRUE;
}

// 0x10007e40
char PTR4* RADEXPLINK BinkGetError(void)
{
    // TODO NOT IMPLEMENTED

    return NULL;
}

// 0x1000a140
u32 RADEXPLINK BinkGetKeyFrame(HBINK bnk, u32 frame, s32 flags)
{
    // TODO NOT IMPLEMENTED

    return 0;
}

// 0x1000a7e0
void RADEXPLINK BinkGetRealtime(HBINK bink, BINKREALTIME PTR4* run, u32 frames)
{
    // TODO NOT IMPLEMENTED
}

// 0x1000a910
s32 RADEXPLINK BinkGetRects(HBINK bnk, u32 flags)
{
    // TODO NOT IMPLEMENTED

    return 0;
}

// 0x1000a630
void RADEXPLINK BinkGetSummary(HBINK bnk, BINKSUMMARY PTR4* sum)
{
    // TODO NOT IMPLEMENTED
}

// 0x1000b1a0
u32 RADEXPLINK BinkGetTrackData(HBINKTRACK bnkt, void PTR4* dest)
{
    // TODO NOT IMPLEMENTED

    return 0;
}

// 0x1000b070
u32 RADEXPLINK BinkGetTrackID(HBINK bnk, u32 trackindex)
{
    if (bnk != NULL) { return bnk->trackIDs[trackindex]; }

    return 0;
}

// 0x1000b050
u32 RADEXPLINK BinkGetTrackMaxSize(HBINK bnk, u32 trackindex)
{
    if (bnk != NULL) { return bnk->tracksizes[trackindex]; }

    return 0;
}

// 0x1000b030
u32 RADEXPLINK BinkGetTrackType(HBINK bnk, u32 trackindex)
{
    if (bnk != NULL) { return bnk->tracktypes[trackindex]; }

    return 0;
}

// 0x1000a220
void RADEXPLINK BinkGoto(HBINK bnk, u32 frame, s32 flags)
{
    // TODO NOT IMPLEMENTED
}

// 0x10005980
s32 RADEXPLINK BinkIsSoftwareCursor(void PTR4* lpDDSP, HCURSOR cur)
{
    // TODO NOT IMPLEMENTED

    return FALSE;
}

// 0x1000b020
void PTR4* RADEXPLINK BinkLogoAddress(void)
{
    // TODO NOT IMPLEMENTED

    return NULL;
}

// 0x1000a0c0
void RADEXPLINK BinkNextFrame(HBINK bnk)
{
    // TODO NOT IMPLEMENTED
}

// 0x10007f00
HBINK RADEXPLINK BinkOpen(const char PTR4* name, u32 flags)
{
    // TODO NOT IMPLEMENTED

    return NULL;
}

// 0x1002a940
BINKSNDOPEN RADEXPLINK BinkOpenDirectSound(u32 param)
{
    // TODO NOT IMPLEMENTED

    return NULL;
}

// 0x10028f90
BINKSNDOPEN RADEXPLINK BinkOpenMiles(u32 param)
{
    // TODO NOT IMPLEMENTED

    return NULL;
}

// 0x1000b090
HBINKTRACK RADEXPLINK BinkOpenTrack(HBINK bnk, u32 trackindex)
{
    // TODO NOT IMPLEMENTED

    return NULL;
}

// 0x1002b3e0
BINKSNDOPEN RADEXPLINK BinkOpenWaveOut(u32 param)
{
    // TODO NOT IMPLEMENTED

    return NULL;
}

// 0x1000a5a0
s32 RADEXPLINK BinkPause(HBINK bnk, s32 pause)
{
    // TODO NOT IMPLEMENTED

    return FALSE;
}

// 0x10005d60
void RADEXPLINK BinkRestoreCursor(s32 checkcount)
{
    if (checkcount != 0) {
        do
        {
            ShowCursor(TRUE);

            checkcount = checkcount - 1;
        } while (checkcount != 0);
    }
}

// 0x1000afa0
void RADEXPLINK BinkService(HBINK bink)
{
    // TODO NOT IMPLEMENTED
}

// 0x10007e10
void RADEXPLINK BinkSetError(const char PTR4* err)
{
    // TODO NOT IMPLEMENTED
}

// 0x10007ea0
void RADEXPLINK BinkSetFrameRate(u32 forcerate, u32 forceratediv)
{
    // TODO NOT IMPLEMENTED
}

// 0x10007ed0
void RADEXPLINK BinkSetIO(BINKIOOPEN io)
{
    // TODO NOT IMPLEMENTED
}

// 0x10007ec0
void RADEXPLINK BinkSetIOSize(u32 iosize)
{
    // TODO NOT IMPLEMENTED
}

// 0x1000aff0
void RADEXPLINK BinkSetPan(HBINK bnk, s32 pan)
{
    if (bnk != NULL && bnk->trackindex != -1 && bnk->bsnd.Pan != NULL) { bnk->bsnd.Pan(&bnk->bsnd, pan); }
}

// 0x10007ee0
void RADEXPLINK BinkSetSimulate(u32 sim)
{
    // TODO NOT IMPLEMENTED
}

// 0x1000b2e0
s32 RADEXPLINK BinkSetSoundOnOff(HBINK bnk, s32 onoff)
{
    // TODO NOT IMPLEMENTED

    return 0;
}

// 0x10007e50
s32 RADEXPLINK BinkSetSoundSystem(BINKSNDSYSOPEN open, u32 param)
{
    // TODO NOT IMPLEMENTED

    return FALSE;
}

// 0x10007ef0
void RADEXPLINK BinkSetSoundTrack(u32 track)
{
    // TODO NOT IMPLEMENTED
}

// 0x1000b2c0
s32 RADEXPLINK BinkSetVideoOnOff(HBINK bnk, s32 onoff)
{
    if (bnk != NULL) { bnk->videoon = onoff; }

    return onoff;
}

// 0x1000afc0
void RADEXPLINK BinkSetVolume(HBINK bnk, s32 volume)
{
    if (bnk != NULL && bnk->trackindex != -1 && bnk->bsnd.Volume != NULL) { bnk->bsnd.Volume(&bnk->bsnd, volume); }
}

// 0x1000a3c0
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

// 0x1002ba30
u32 RADEXPLINK RADTimerRead(void)
{
    // TODO NOT IMPLEMENTED

    return 0;
}

// 0x10014350
void RADEXPLINK YUV_blit_16a1bpp(u32 param_1, u32 param_2, u32 param_3, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12, u32 param_13)
{
    // TODO NOT IMPLEMENTED
}

// 0x10016890
void RADEXPLINK YUV_blit_16a1bpp_mask(u32 param_1, u32 param_2, u32 param_3, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12, u32 param_13)
{
    // TODO NOT IMPLEMENTED
}

// 0x100137b0
void RADEXPLINK YUV_blit_16a4bpp(u32 param_1, u32 param_2, u32 param_3, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12, u32 param_13)
{
    // TODO NOT IMPLEMENTED
}

// 0x100168f0
void RADEXPLINK YUV_blit_16a4bpp_mask(u32 param_1, u32 param_2, u32 param_3, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12, u32 param_13)
{
    // TODO NOT IMPLEMENTED
}

// 0x10012790
void RADEXPLINK YUV_blit_16bpp(u32 param_1, u32 param_2, u32 param_3, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12)
{
    // TODO NOT IMPLEMENTED
}

// 0x10016840
void RADEXPLINK YUV_blit_16bpp_mask(u32 param_1, u32 param_2, u32 param_3, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12)
{
    // TODO NOT IMPLEMENTED
}

// 0x100119b0
void RADEXPLINK YUV_blit_24bpp(u32 param_1, u32 param_2, u32 param_3, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12)
{
    // TODO NOT IMPLEMENTED
}

// 0x100167a0
void RADEXPLINK YUV_blit_24bpp_mask(u32 param_1, u32 param_2, u32 param_3, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12)
{
    // TODO NOT IMPLEMENTED
}

// 0x10011a00
void RADEXPLINK YUV_blit_24rbpp(u32 param_1, u32 param_2, u32 param_3, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12)
{
    // TODO NOT IMPLEMENTED
}

// 0x100167f0
void RADEXPLINK YUV_blit_24rbpp_mask(u32 param_1, u32 param_2, u32 param_3, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12)
{
    // TODO NOT IMPLEMENTED
}

// 0x1000fbc0
void RADEXPLINK YUV_blit_32abpp(u32 param_1, u32 param_2, u32 param_3, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12, u32 param_13)
{
    // TODO NOT IMPLEMENTED
}

// 0x100166e0
void RADEXPLINK YUV_blit_32abpp_mask(u32 param_1, u32 param_2, u32 param_3, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12, u32 param_13)
{
    // TODO NOT IMPLEMENTED
}

// 0x1000d180
void RADEXPLINK YUV_blit_32bpp(u32 param_1, u32 param_2, u32 param_3, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12)
{
    // TODO NOT IMPLEMENTED
}

// 0x10015e60
void RADEXPLINK YUV_blit_32bpp_mask(u32 param_1, u32 param_2, u32 param_3, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12)
{
    // TODO NOT IMPLEMENTED
}

// 0x1000fc20
void RADEXPLINK YUV_blit_32rabpp(u32 param_1, u32 param_2, u32 param_3, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12, u32 param_13)
{
    // TODO NOT IMPLEMENTED
}

// 0x10016740
void RADEXPLINK YUV_blit_32rabpp_mask(u32 param_1, u32 param_2, u32 param_3, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12, u32 param_13)
{
    // TODO NOT IMPLEMENTED
}

// 0x1000e410
void RADEXPLINK YUV_blit_32rbpp(u32 param_1, u32 param_2, u32 param_3, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12)
{
    // TODO NOT IMPLEMENTED
}

// 0x10016690
void RADEXPLINK YUV_blit_32rbpp_mask(u32 param_1, u32 param_2, u32 param_3, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12)
{
    // TODO NOT IMPLEMENTED
}

// 0x10015b20
void RADEXPLINK YUV_blit_UYVY(u32 param_1, u32 param_2, u32 param_3, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12)
{
    // TODO NOT IMPLEMENTED
}

// 0x100169a0
void RADEXPLINK YUV_blit_UYVY_mask(u32 param_1, u32 param_2, u32 param_3, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12)
{
    // TODO NOT IMPLEMENTED
}

// 0x10014ee0
void RADEXPLINK YUV_blit_YUY2(u32 param_1, u32 param_2, u32 param_3, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12)
{
    // TODO NOT IMPLEMENTED
}

// 0x10016950
void RADEXPLINK YUV_blit_YUY2_mask(u32 param_1, u32 param_2, u32 param_3, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12)
{
    // TODO NOT IMPLEMENTED
}

// 0x10015bb0
void RADEXPLINK YUV_blit_YV12(u32 param_1, u32 param_2, u32 param_3, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12, u32 param_13)
{
    // TODO NOT IMPLEMENTED
}

// 0x1000b4e0
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