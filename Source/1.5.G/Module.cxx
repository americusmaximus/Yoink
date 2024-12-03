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

// 0x300010c0
BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID)
{
    if (reason != DLL_PROCESS_ATTACH) { return TRUE; }

    State.Instance = instance;

    char name[MAX_MODULE_NAME_PATH_LENGTH];
    GetModuleFileNameA(instance, name, MAX_MODULE_NAME_PATH_LENGTH - 1);

    const U32 length = radstrlen(name);

    for (U32 x = length; x != 0; x--)
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

// 0x30003260
void RADEXPLINK BinkBufferBlit(HBINKBUFFER buf, BINKRECT PTR4* rects, U32 numrects)
{
    // TODO NOT IMPLEMENTED
}

// 0x300018a0
void RADEXPLINK BinkBufferCheckWinPos(HBINKBUFFER buf, S32 PTR4* NewWindowX, S32 PTR4* NewWindowY)
{
    // TODO NOT IMPLEMENTED
}

// 0x30003920
S32 RADEXPLINK BinkBufferClear(HBINKBUFFER buf, U32 RGB)
{
    // TODO NOT IMPLEMENTED

    return TRUE;
}

// 0x30002f20
void RADEXPLINK BinkBufferClose(HBINKBUFFER buf)
{
    // TODO NOT IMPLEMENTED
}

// 0x30003790
char PTR4* RADEXPLINK BinkBufferGetDescription(HBINKBUFFER buf)
{
    // TODO NOT IMPLEMENTED

    return NULL;
}

// 0x30003910
char PTR4* RADEXPLINK BinkBufferGetError()
{
    // TODO NOT IMPLEMENTED

    return NULL;
}

// 0x30003020
S32 RADEXPLINK BinkBufferLock(HBINKBUFFER buf)
{
    // TODO NOT IMPLEMENTED

    return TRUE;
}

// 0x30001b40
HBINKBUFFER RADEXPLINK BinkBufferOpen(void* /*HWND*/ wnd, U32 width, U32 height, U32 bufferflags)
{
    // TODO NOT IMPLEMENTED

    return NULL;
}

// 0x300011a0
S32 RADEXPLINK BinkBufferSetDirectDraw(void PTR4* lpDirectDraw, void PTR4* lpPrimary)
{
    // TODO NOT IMPLEMENTED

    return TRUE;
}

// 0x30003780
S32 RADEXPLINK BinkBufferSetHWND(HBINKBUFFER buf, void* /*HWND*/ newwnd)
{
    return FALSE;
}

// 0x30001950
S32 RADEXPLINK BinkBufferSetOffset(HBINKBUFFER buf, S32 destx, S32 desty)
{
    // TODO NOT IMPLEMENTED

    return TRUE;
}

// 0x30001880
void RADEXPLINK BinkBufferSetResolution(S32 w, S32 h, S32 bits)
{
    // TODO NOT IMPLEMENTED
}

// 0x30003670
S32 RADEXPLINK BinkBufferSetScale(HBINKBUFFER buf, U32 w, U32 h)
{
    // TODO NOT IMPLEMENTED

    return TRUE;
}

// 0x300031e0
S32 RADEXPLINK BinkBufferUnlock(HBINKBUFFER buf)
{
    // TODO NOT IMPLEMENTED

    return TRUE;
}

// 0x300017a0
s32 RADEXPLINK BinkCheckCursor(void* /*HWND*/ wnd, S32 x, S32 y, S32 w, S32 h)
{
    // TODO NOT IMPLEMENTED

    return 0;
}

// 0x30006950
void RADEXPLINK BinkClose(HBINK bnk)
{
    // TODO NOT IMPLEMENTED
}

// 0x30007c60
void RADEXPLINK BinkCloseTrack(HBINKTRACK bnkt)
{
    // TODO NOT IMPLEMENTED
}

// 0x300051c0
S32 RADEXPLINK BinkCopyToBuffer(HBINK bnk, void* dest, S32 destpitch, U32 destheight, U32 destx, U32 desty, U32 flags)
{
    // TODO NOT IMPLEMENTED

    return TRUE;
}

// 0x30005200
S32 RADEXPLINK BinkCopyToBufferRect(HBINK bnk, void* dest, S32 destpitch, U32 destheight, U32 destx, U32 desty, U32 srcx, U32 srcy, U32 srcw, U32 srch, U32 flags)
{
    // TODO NOT IMPLEMENTED

    return TRUE;
}

// 0x300039d0
S32 RADEXPLINK BinkDDSurfaceType(void PTR4* lpDDS)
{
    // TODO NOT IMPLEMENTED

    return -1;
}

// 0x30005f40
S32 RADEXPLINK BinkDoFrame(HBINK bnk)
{
    // TODO NOT IMPLEMENTED

    return TRUE;
}

// 0x30003ba0
S32 RADEXPLINK BinkDX8SurfaceType(void* lpD3Ds)
{
    // TODO NOT IMPLEMENTED

    return -1;
}

// 0x30003ca0
char PTR4* RADEXPLINK BinkGetError(void)
{
    // TODO NOT IMPLEMENTED

    return NULL;
}

// 0x300066b0
U32 RADEXPLINK BinkGetKeyFrame(HBINK bnk, U32 frame, S32 flags)
{
    // TODO NOT IMPLEMENTED

    return 0;
}

// 0x300070c0
void RADEXPLINK BinkGetRealtime(HBINK bink, BINKREALTIME PTR4* run, U32 frames)
{
    // TODO NOT IMPLEMENTED
}

// 0x30007250
S32 RADEXPLINK BinkGetRects(HBINK bnk, U32 flags)
{
    // TODO NOT IMPLEMENTED

    return 0;
}

// 0x30006eb0
void RADEXPLINK BinkGetSummary(HBINK bnk, BINKSUMMARY PTR4* sum)
{
    // TODO NOT IMPLEMENTED
}

// 0x30007c90
U32 RADEXPLINK BinkGetTrackData(HBINKTRACK bnkt, void PTR4* dest)
{
    // TODO NOT IMPLEMENTED

    return 0;
}

// 0x30007b30
U32 RADEXPLINK BinkGetTrackID(HBINK bnk, U32 trackindex)
{
    if (bnk != NULL) { return bnk->trackIDs[trackindex]; }

    return 0;
}

// 0x30007b10
U32 RADEXPLINK BinkGetTrackMaxSize(HBINK bnk, U32 trackindex)
{
    if (bnk != NULL) { return bnk->tracksizes[trackindex]; }

    return 0;
}

// 0x30007af0
U32 RADEXPLINK BinkGetTrackType(HBINK bnk, U32 trackindex)
{
    if (bnk != NULL) { return bnk->tracktypes[trackindex]; }

    return 0;
}

// 0x30006790
void RADEXPLINK BinkGoto(HBINK bnk, U32 frame, S32 flags)
{
    // TODO NOT IMPLEMENTED
}

// 0x30001480
S32 RADEXPLINK BinkIsSoftwareCursor(void PTR4* lpDDSP, void* /*HCURSOR*/ cur)
{
    // TODO NOT IMPLEMENTED

    return FALSE;
}

// 0x30007ae0
void PTR4* RADEXPLINK BinkLogoAddress(void)
{
    // TODO NOT IMPLEMENTED

    return NULL;
}

// 0x30006530
void RADEXPLINK BinkNextFrame(HBINK bnk)
{
    // TODO NOT IMPLEMENTED
}

// 0x30003d90
HBINK RADEXPLINK BinkOpen(const char PTR4* name, U32 flags)
{
    // TODO NOT IMPLEMENTED

    return NULL;
}

// 0x30008760
BINKSNDOPEN RADEXPLINK BinkOpenDirectSound(U32 param)
{
    // TODO NOT IMPLEMENTED

    return NULL;
}

// 0x30009690
BINKSNDOPEN RADEXPLINK BinkOpenMiles(U32 param)
{
    // TODO NOT IMPLEMENTED

    return NULL;
}

// 0x30007b50
HBINKTRACK RADEXPLINK BinkOpenTrack(HBINK bnk, U32 trackindex)
{
    // TODO NOT IMPLEMENTED

    return NULL;
}

// 0x30007f80
BINKSNDOPEN RADEXPLINK BinkOpenWaveOut(U32 param)
{
    // TODO NOT IMPLEMENTED

    return NULL;
}

// 0x30006db0
S32 RADEXPLINK BinkPause(HBINK bnk, S32 pause)
{
    // TODO NOT IMPLEMENTED

    return FALSE;
}

// 0x30001860
void RADEXPLINK BinkRestoreCursor(S32 checkcount)
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

// 0x300078e0
void RADEXPLINK BinkService(HBINK bink)
{
    // TODO NOT IMPLEMENTED
}

// 0x30003c80
void RADEXPLINK BinkSetError(const char PTR4* err)
{
    // TODO NOT IMPLEMENTED
}

// 0x30003d00
void RADEXPLINK BinkSetFrameRate(U32 forcerate, U32 forceratediv)
{
    // TODO NOT IMPLEMENTED
}

// 0x30003d30
void RADEXPLINK BinkSetIO(BINKIOOPEN io)
{
    // TODO NOT IMPLEMENTED
}

// 0x30003d20
void RADEXPLINK BinkSetIOSize(U32 iosize)
{
    // TODO NOT IMPLEMENTED
}

// 0x30007970
void RADEXPLINK BinkSetMixBins(HBINK bnk, U32 trackid, U32 PTR4* mix_bins, U32 total)
{
    // TODO NOT IMPLEMENTED
}

// 0x300079f0
void RADEXPLINK BinkSetMixBinVolumes(HBINK bnk, U32 trackid, U32 PTR4* vol_mix_bins, S32 PTR4* volumes, U32 total)
{
    // TODO NOT IMPLEMENTED
}

// 0x30007a70
void RADEXPLINK BinkSetPan(HBINK bnk, U32 trackid, S32 pan)
{
    // TODO NOT IMPLEMENTED
}

// 0x30003d40
void RADEXPLINK BinkSetSimulate(U32 sim)
{
    // TODO NOT IMPLEMENTED
}

// 0x30007dd0
S32 RADEXPLINK BinkSetSoundOnOff(HBINK bnk, S32 onoff)
{
    // TODO NOT IMPLEMENTED

    return 0;
}

// 0x30003cb0
S32 RADEXPLINK BinkSetSoundSystem(BINKSNDSYSOPEN open, U32 param)
{
    // TODO NOT IMPLEMENTED

    return FALSE;
}

// 0x30003d50
void RADEXPLINK BinkSetSoundTrack(U32 total_tracks, U32 PTR4* tracks)
{
    // TODO NOT IMPLEMENTED
}

// 0x30007db0
S32 RADEXPLINK BinkSetVideoOnOff(HBINK bnk, S32 onoff)
{
    if (bnk != NULL) { bnk->videoon = onoff; }

    return onoff;
}

// 0x30007900
void RADEXPLINK BinkSetVolume(HBINK bnk, U32 trackid, S32 volume)
{
    // TODO NOT IMPLEMENTED
}

// 0x30006b30
S32 RADEXPLINK BinkWait(HBINK bnk)
{
    // TODO NOT IMPLEMENTED

    return FALSE;
}

// 0x30001000
void RADEXPLINK RADSetMemory(RADMEMALLOC a, RADMEMFREE f)
{
    // TODO NOT IMPLEMENTED
}

// 0x30009de0
U32 RADEXPLINK RADTimerRead(void)
{
    // TODO NOT IMPLEMENTED

    return 0;
}

// 0x30023b20
void RADEXPLINK YUV_blit_16a1bpp(U32 param_1, U32 param_2, U32 param_3, U32 param_4, U32 param_5, U32 param_6, U32 param_7, U32 param_8, U32 param_9, U32 param_10, U32 param_11, U32 param_12, U32 param_13)
{
    // TODO NOT IMPLEMENTED
}

// 0x30023b70
void RADEXPLINK YUV_blit_16a1bpp_mask(U32 param_1, U32 param_2, U32 param_3, U32 param_4, U32 param_5, U32 param_6, U32 param_7, U32 param_8, U32 param_9, U32 param_10, U32 param_11, U32 param_12, U32 param_13, U32 param_14, U32 param_15)
{
    // TODO NOT IMPLEMENTED
}

// 0x30021fb0
void RADEXPLINK YUV_blit_16a4bpp(U32 param_1, U32 param_2, U32 param_3, U32 param_4, U32 param_5, U32 param_6, U32 param_7, U32 param_8, U32 param_9, U32 param_10, U32 param_11, U32 param_12, U32 param_13)
{
    // TODO NOT IMPLEMENTED
}

// 0x30022000
void RADEXPLINK YUV_blit_16a4bpp_mask(U32 param_1, U32 param_2, U32 param_3, U32 param_4, U32 param_5, U32 param_6, U32 param_7, U32 param_8, U32 param_9, U32 param_10, U32 param_11, U32 param_12, U32 param_13, U32 param_14, U32 param_15)
{
    // TODO NOT IMPLEMENTED
}

// 0x3001fd20
void RADEXPLINK YUV_blit_16bpp(U32 param_1, U32 param_2, U32 param_3, U32 param_4, U32 param_5, U32 param_6, U32 param_7, U32 param_8, U32 param_9, U32 param_10, U32 param_11, U32 param_12)
{
    // TODO NOT IMPLEMENTED
}

// 0x3001fd70
void RADEXPLINK YUV_blit_16bpp_mask(U32 param_1, U32 param_2, U32 param_3, U32 param_4, U32 param_5, U32 param_6, U32 param_7, U32 param_8, U32 param_9, U32 param_10, U32 param_11, U32 param_12, U32 param_13, U32 param_14)
{
    // TODO NOT IMPLEMENTED
}

// 0x3001c0b0
void RADEXPLINK YUV_blit_24bpp(U32 param_1, U32 param_2, U32 param_3, U32 param_4, U32 param_5, U32 param_6, U32 param_7, U32 param_8, U32 param_9, U32 param_10, U32 param_11, U32 param_12)
{
    // TODO NOT IMPLEMENTED
}

// 0x3001c100
void RADEXPLINK YUV_blit_24bpp_mask(U32 param_1, U32 param_2, U32 param_3, U32 param_4, U32 param_5, U32 param_6, U32 param_7, U32 param_8, U32 param_9, U32 param_10, U32 param_11, U32 param_12, U32 param_13, U32 param_14)
{
    // TODO NOT IMPLEMENTED
}

// 0x3001dc40
void RADEXPLINK YUV_blit_24rbpp(U32 param_1, U32 param_2, U32 param_3, U32 param_4, U32 param_5, U32 param_6, U32 param_7, U32 param_8, U32 param_9, U32 param_10, U32 param_11, U32 param_12)
{
    // TODO NOT IMPLEMENTED
}

// 0x3001dc90
void RADEXPLINK YUV_blit_24rbpp_mask(U32 param_1, U32 param_2, U32 param_3, U32 param_4, U32 param_5, U32 param_6, U32 param_7, U32 param_8, U32 param_9, U32 param_10, U32 param_11, U32 param_12, U32 param_13, U32 param_14)
{
    // TODO NOT IMPLEMENTED
}

// 0x30017ef0
void RADEXPLINK YUV_blit_32abpp(U32 param_1, U32 param_2, U32 param_3, U32 param_4, U32 param_5, U32 param_6, U32 param_7, U32 param_8, U32 param_9, U32 param_10, U32 param_11, U32 param_12, U32 param_13)
{
    // TODO NOT IMPLEMENTED
}

// 0x30017f40
void RADEXPLINK YUV_blit_32abpp_mask(U32 param_1, U32 param_2, U32 param_3, U32 param_4, U32 param_5, U32 param_6, U32 param_7, U32 param_8, U32 param_9, U32 param_10, U32 param_11, U32 param_12, U32 param_13, U32 param_14, U32 param_15)
{
    // TODO NOT IMPLEMENTED
}

// 0x30013c60
void RADEXPLINK YUV_blit_32bpp(U32 param_1, U32 param_2, U32 param_3, U32 param_4, U32 param_5, U32 param_6, U32 param_7, U32 param_8, U32 param_9, U32 param_10, U32 param_11, U32 param_12)
{
    // TODO NOT IMPLEMENTED
}

// 0x30013cb0
void RADEXPLINK YUV_blit_32bpp_mask(U32 param_1, U32 param_2, U32 param_3, U32 param_4, U32 param_5, U32 param_6, U32 param_7, U32 param_8, U32 param_9, U32 param_10, U32 param_11, U32 param_12, U32 param_13, U32 param_14)
{
    // TODO NOT IMPLEMENTED
}

// 0x30019870
void RADEXPLINK YUV_blit_32rabpp(U32 param_1, U32 param_2, U32 param_3, U32 param_4, U32 param_5, U32 param_6, U32 param_7, U32 param_8, U32 param_9, U32 param_10, U32 param_11, U32 param_12, U32 param_13)
{
    // TODO NOT IMPLEMENTED
}

// 0x300198c0
void RADEXPLINK YUV_blit_32rabpp_mask(U32 param_1, U32 param_2, U32 param_3, U32 param_4, U32 param_5, U32 param_6, U32 param_7, U32 param_8, U32 param_9, U32 param_10, U32 param_11, U32 param_12, U32 param_13, U32 param_14, U32 param_15)
{
    // TODO NOT IMPLEMENTED
}

// 0x30015880
void RADEXPLINK YUV_blit_32rbpp(U32 param_1, U32 param_2, U32 param_3, U32 param_4, U32 param_5, U32 param_6, U32 param_7, U32 param_8, U32 param_9, U32 param_10, U32 param_11, U32 param_12)
{
    // TODO NOT IMPLEMENTED
}

// 0x300158d0
void RADEXPLINK YUV_blit_32rbpp_mask(U32 param_1, U32 param_2, U32 param_3, U32 param_4, U32 param_5, U32 param_6, U32 param_7, U32 param_8, U32 param_9, U32 param_10, U32 param_11, U32 param_12, U32 param_13, U32 param_14)
{
    // TODO NOT IMPLEMENTED
}

// 0x30026b60
void RADEXPLINK YUV_blit_UYVY(U32 param_1, U32 param_2, U32 param_3, U32 param_4, U32 param_5, U32 param_6, U32 param_7, U32 param_8, U32 param_9, U32 param_10, U32 param_11, U32 param_12)
{
    // TODO NOT IMPLEMENTED
}

// 0x30026be0
void RADEXPLINK YUV_blit_UYVY_mask(U32 param_1, U32 param_2, U32 param_3, U32 param_4, U32 param_5, U32 param_6, U32 param_7, U32 param_8, U32 param_9, U32 param_10, U32 param_11, U32 param_12, U32 param_13, U32 param_14)
{
    // TODO NOT IMPLEMENTED
}

// 0x30025af0
void RADEXPLINK YUV_blit_YUY2(U32 param_1, U32 param_2, U32 param_3, U32 param_4, U32 param_5, U32 param_6, U32 param_7, U32 param_8, U32 param_9, U32 param_10, U32 param_11, U32 param_12)
{
    // TODO NOT IMPLEMENTED
}

// 0x30025b70
void RADEXPLINK YUV_blit_YUY2_mask(U32 param_1, U32 param_2, U32 param_3, U32 param_4, U32 param_5, U32 param_6, U32 param_7, U32 param_8, U32 param_9, U32 param_10, U32 param_11, U32 param_12, U32 param_13, U32 param_14)
{
    // TODO NOT IMPLEMENTED
}

// 0x30027000
void RADEXPLINK YUV_blit_YV12(U32 param_1, U32 param_2, U32 param_3, U32 param_4, U32 param_5, U32 param_6, U32 param_7, U32 param_8, U32 param_9, U32 param_10, U32 param_11, U32 param_12, U32 param_13)
{
    // TODO NOT IMPLEMENTED
}

// 0x3000a680
void RADEXPLINK YUV_init(U32 flags)
{
    // TODO NOT IMPLEMENTED
}

// 0x30001020
void PTR4* RADEXPLINK radmalloc(U32 numbytes)
{
    // TODO NOT IMPLEMENTED

    return NULL;
}

// 0x30001090
void RADEXPLINK radfree(void PTR4* ptr)
{
    // TODO NOT IMPLEMENTED
}
