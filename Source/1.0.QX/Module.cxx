/*
Copyright (c) 2024 - 2025 Americus Maximus

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

#include "Bink.hxx"
#include "Buffer.hxx"
#include "Cursor.hxx"
#include "Frame.hxx"
#include "IO.hxx"
#include "Mem.hxx"
#include "Module.hxx"
#include "Sound.hxx"
#include "Thread.hxx"
#include "Timer.hxx"

// 0x10001000
void RADEXPLINK RADSetMemory(RADMEMALLOC a, RADMEMFREE f)
{
    MemAllocate = a;
    MemFree = f;
}

// 0x10001020
void PTR4* RADEXPLINK radmalloc(u32 numbytes)
{
    if (numbytes == 0 || numbytes == INVALID_MEMORY_ALLOCATION_SIZE) { return NULL; }

    void* ptr = NULL;

    u8 type = INTERNAL_MEMORY_ALLOCATION;

    if (MemAllocate == NULL) {
        ptr = malloc(numbytes + MEMORY_ALLOCATION_EXTRA_SPACE);

        if (ptr == NULL) { return NULL; }

        type = INTERNAL_MEMORY_ALLOCATION;
    }
    else {
        ptr = MemAllocate(numbytes + MEMORY_ALLOCATION_EXTRA_SPACE);

        if (ptr == NULL) {
            ptr = malloc(numbytes + MEMORY_ALLOCATION_EXTRA_SPACE);

            if (ptr == NULL) { return NULL; }

            type = INTERNAL_MEMORY_ALLOCATION;
        }
        else if (ptr == INVALID_MEMORY_ALLOCATION_VALUE) { return NULL; }
        else { type = EXTERNAL_MEMORY_ALLOCATION; }
    }

    const u8 offset = CALCULATE_MEMORY_OFFSET_VALUE(ptr);

    void* result = (void*)((size_t)ptr + offset);

    SET_MEMORY_OFFSET_VALUE(result, offset);
    SET_MEMORY_TYPE_VALUE(result, type);

    if (type == EXTERNAL_MEMORY_ALLOCATION) {
        SET_MEMORY_FREE_FUNCTION_VALUE(result, MemFree);
    }

    return result;
}

// 0x100010a0
void RADEXPLINK radfree(void PTR4* ptr)
{
    if (ptr != NULL) {
        if (GET_MEMORY_TYPE_VALUE(ptr) == EXTERNAL_MEMORY_ALLOCATION) {
            GET_MEMORY_FREE_FUNCTION_VALUE(ptr)(CALCULATE_MEMORY_OFFSET_POINTER(ptr));
            return;
        }

        free(CALCULATE_MEMORY_OFFSET_POINTER(ptr));
    }
}

// 0x100025e0
void RADEXPLINK ExpandBundleSizes(struct BUNDLEPOINTERS* pointers, u32 width)
{
    pointers->typeptr = (void*)BinkCalculateBundleSize(512, width, 4, 1);
    pointers->type16ptr = (void*)BinkCalculateBundleSize(512, width / 2, 4, 1);
    pointers->colorptr = (void*)BinkCalculateBundleSize(512, width, 8, 64);
    pointers->bits2ptr = (void*)BinkCalculateBundleSize(512, width, 8, 8);
    pointers->motionXptr = (void*)BinkCalculateBundleSize(512, width, 5, 1);
    pointers->motionYptr = (void*)BinkCalculateBundleSize(512, width, 5, 1);
    pointers->dctptr = (void*)BinkCalculateBundleSize(512, width, 11, 1);
    pointers->mdctptr = (void*)BinkCalculateBundleSize(512, width, 11, 1);
    pointers->patptr = (void*)BinkCalculateBundleSize(512, width, 4, 48);
}

// 0x100026e0
void RADEXPLINK ExpandBink(void* yp1, void* yp2, void* ap1, void* ap2, void* mp, u32 decompwidth, u32 decompheight, u32 width, u32 height, u32 param_10, u32 key, u32 param_12, u32 flags, u32 type)
{
    // TODO NOT IMPLEMENTED
}

// 0x10005690
s32 RADEXPLINK BinkBufferSetDirectDraw(void PTR4* lpDirectDraw, void PTR4* lpPrimary)
{
    if (lpDirectDraw != NULL && lpPrimary != NULL) {
        BufferDirectDraw = (LPDIRECTDRAW)lpDirectDraw;
        BufferDirectDrawSurface = (LPDIRECTDRAWSURFACE)lpPrimary;
        BufferCount = 0x8000000; // TODO

        BinkBufferDirectDrawCapabilitiesInitialize();

        return TRUE;
    }

    BufferDirectDraw = NULL;
    BufferDirectDrawSurface = NULL;
    BufferCount = 0;

    return TRUE;
}

// 0x10005980
s32 RADEXPLINK BinkIsSoftwareCursor(void PTR4* lpDDSP, HCURSOR cur)
{
    if (GetSystemMetrics(SM_MOUSEPRESENT)) {
        LPDIRECTDRAWSURFACE surface = (LPDIRECTDRAWSURFACE)lpDDSP;

        DDSURFACEDESC desc;
        radmemset(&desc, 0x00, sizeof(DDSURFACEDESC));

        desc.dwSize = sizeof(DDSURFACEDESC);

        desc.dwFlags = DDSD_CAPS;
        desc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

        HRESULT result = DD_OK;

        while ((result = surface->Lock(NULL, &desc, DDLOCK_WAIT, NULL)) == DDERR_SURFACELOST) {
            if (FAILED(surface->Restore())) { return FALSE; }
        }

        if (SUCCEEDED(result)) {
            surface->Unlock(desc.lpSurface);

            if (CursorPointer == cur && CursorSurface == desc.lpSurface
                && CursorBits == desc.ddpfPixelFormat.dwRGBBitCount) {
                return CursorIsSoftware;
            }

            WNDCLASSA wndc;

            HINSTANCE inst = GetModuleHandleA(NULL);

            wndc.style = CS_NOCLOSE;
            wndc.lpfnWndProc = BinkCursorWindowProc;
            wndc.cbClsExtra = 0;
            wndc.cbWndExtra = 0;
            wndc.hInstance = inst;
            wndc.hIcon = NULL;
            wndc.hCursor = cur;
            wndc.hbrBackground = NULL;
            wndc.lpszMenuName = NULL;
            wndc.lpszClassName = CURSOR_TYPE_WINDOW_NAME;

            RegisterClassA(&wndc);

            POINT point;
            GetCursorPos(&point);

            HWND wnd = CreateWindowExA(WS_EX_TOPMOST,
                CURSOR_TYPE_WINDOW_NAME, "", WS_POPUP | WS_VISIBLE,
                point.x - (CURSOR_TYPE_WINDOW_SIZE / 2), point.y - (CURSOR_TYPE_WINDOW_SIZE / 2),
                CURSOR_TYPE_WINDOW_SIZE, CURSOR_TYPE_WINDOW_SIZE, NULL, NULL, inst, NULL);

            MSG msg;
            PeekMessageA(&msg, NULL, 0, 0, FALSE);

            POINT target;
            target.x = point.x;
            target.y = point.y;

            ScreenToClient(wnd, &target);

            s32 count = 0;

            do { count = count + 1; } while (ShowCursor(FALSE) >= 0);

            COLORREF color = NULL;

            {
                HDC dc = GetDC(wnd);
                color = GetPixel(dc, target.x, target.y);
                ReleaseDC(wnd, dc);
            }

            HCURSOR cursor = SetCursor(cur);

            do { count = count - 1; } while (ShowCursor(TRUE) < 0);

            SetCursor(cur);

            surface->Lock(NULL, &desc, DDLOCK_WAIT, NULL);

            {
                u8* ptr = (u8*)((size_t)desc.lpSurface
                    + desc.lPitch * point.y + (point.x * desc.ddpfPixelFormat.dwRGBBitCount >> 3));

                ptr[0] = color == NULL ? 0xFF : 0x00;
            }

            surface->Unlock(desc.lpSurface);

            do { count = count + 1; } while (ShowCursor(FALSE) >= 0);

            HDC dc = GetDC(wnd);
            const u32 match = GetPixel(dc, target.x, target.y) == color;

            if (!match) { SetPixel(dc, target.x, target.y, color); }

            ReleaseDC(wnd, dc);
            DestroyWindow(wnd);
            UnregisterClassA(CURSOR_TYPE_WINDOW_NAME, inst);

            if (count <= 0) {
                for (s32 i = 0; i <= -count; i++) { ShowCursor(FALSE); }
            }
            else {
                for (s32 i = 0; i <= count; i++) { ShowCursor(TRUE); }
            }

            SetCursor(cursor);

            CursorBits = desc.ddpfPixelFormat.dwRGBBitCount;
            CursorPointer = cur;
            CursorSurface = desc.lpSurface;
            CursorIsSoftware = match;

            return match;
        }
    }

    return FALSE;
}

// 0x10005cb0
s32 RADEXPLINK BinkCheckCursor(HWND wnd, s32 x, s32 y, s32 w, s32 h)
{
    s32 count = 0;
    if (CursorWidth == 0) {
        CursorWidth = GetSystemMetrics(SM_CXCURSOR);
        CursorHeight = GetSystemMetrics(SM_CYCURSOR);
    }

    POINT window;
    window.x = x;
    window.y = y;

    if (wnd != NULL) { ClientToScreen(wnd, &window); }

    POINT cursor;
    GetCursorPos(&cursor);

    if (window.x < (CursorWidth + cursor.x) && cursor.x < (window.x + w)
        && window.y < (CursorHeight + cursor.y) && cursor.y < (window.y + h)) {
        do {
            count = count + 1;
        } while (ShowCursor(FALSE) >= 0);
    }

    return count;
}

// 0x10005d60
void RADEXPLINK BinkRestoreCursor(s32 checkcount)
{
    if (checkcount != 0) {
        do {
            ShowCursor(TRUE);
            checkcount = checkcount - 1;
        } while (checkcount != 0);
    }
}

// 0x10005d90
void RADEXPLINK BinkBufferSetResolution(s32 w, s32 h, s32 bits)
{
    BufferWidth = w;
    BufferHeight = h;
    BufferBits = bits;
}

// 0x10005db0
void RADEXPLINK BinkBufferCheckWinPos(HBINKBUFFER buf, s32 PTR4* NewWindowX, s32 PTR4* NewWindowY)
{
    if (buf != NULL) {
        if (NewWindowX != NULL) {
            s32 value = *NewWindowX + buf->ClientOffsetX;

            if (buf->noclipping) {
                if (BufferScreenWidth < (s32)(buf->Width + value)) {
                    value = BufferScreenWidth - buf->Width;
                }

                if (value < 0) { value = 0; }
            }

            s32 result = 0;

            if (BufferBytes == 4) { result = value + 1U & 0xFFFFFFFE; }
            else if (BufferBytes == 3) { result = value + 7U & 0xFFFFFFF8; }
            else { result = value + 3U & 0xFFFFFFFC; }

            *NewWindowX = result - buf->ClientOffsetX;
        }

        if (NewWindowY != NULL) {
            s32 value = buf->ClientOffsetY + *NewWindowY;

            if (buf->noclipping) {
                if (BufferScreenHeight < (s32)(buf->Height + value)) {
                    value = BufferScreenHeight - buf->Height;
                }

                if (value < 0) { value = 0; }
            }

            *NewWindowY = value - buf->ClientOffsetY;
        }
    }
}

// 0x10005e60
s32 RADEXPLINK BinkBufferSetOffset(HBINKBUFFER buf, s32 destx, s32 desty)
{
    if (buf == NULL) { return FALSE; }

    POINT point;
    point.y = 0;
    point.x = 0;

    ClientToScreen(buf->wnd, &point);

    buf->destx = point.x + destx;
    buf->wndx = destx;
    buf->desty = point.y + desty;
    buf->wndy = desty;
    buf->SurfaceType = buf->SurfaceType | BINKCOPYALL;

    BinkBufferMove(buf, BINKBUFFERMOVEMODE_NONE);

    return TRUE;
}

// 0x10006050
HBINKBUFFER RADEXPLINK BinkBufferOpen(HWND wnd, u32 width, u32 height, u32 bufferflags)
{
    if (width == 0) { return NULL; }
    if (height == 0) { return NULL; }

    u32 buffertype = bufferflags & BINKBUFFERTYPEMASK;

    const u32 options = bufferflags & BINKBUFFERSCALES;
    const u32 fullscreen = bufferflags & BINKBUFFERRESOLUTION;

    s32 dc = FALSE, dd = FALSE;
    s32 issoftcur = FALSE;
    s32 overlay = FALSE, offscreen = FALSE;
    s32 noclipping = FALSE;
    u32 scales = 0, surfacetype = 0;

    LPDIRECTDRAWCLIPPER clipper = NULL;
    LPDIRECTDRAWSURFACE surface = NULL;

    // Notes:
    // 1. If buffer type is BINKBUFFERDIBSECTION then create HDC surface.
    // 2. If buffer type is BINKBUFFERAUTO or BINKBUFFERPRIMARY then create DirectDraw primary surface.
    // 3. Otherwise - create either overlay or offscreen surface, only when DirectDraw is availble.

    if (buffertype == BINKBUFFERDIBSECTION) {
        if (BinkBufferDeviceContextInitialize(wnd, fullscreen)) { dc = TRUE; }
        else {
            dd = TRUE;
            BinkBufferDirectDrawInitialize(wnd, fullscreen);
        }
    }
    else {
        dd = TRUE;
        BinkBufferDirectDrawInitialize(wnd, fullscreen);

        if (BufferDirectDraw == NULL) {
            if (buffertype != BINKBUFFERAUTO) {
                radstrcpy(BufferError, "DirectDraw is not available");
                BinkBufferDirectDrawRelease();

                return NULL;
            }
        }

        if (buffertype == BINKBUFFERAUTO) { buffertype = BINKBUFFERPRIMARY; }
    }

    if (!dc && (BufferDirectDraw == NULL || BufferBytes == 0)
        && BinkBufferDeviceContextInitialize(wnd, fullscreen)) {
        dc = TRUE;
    }

    // Notes:
    // If the surface is not a HDC surface, then:
    // Set configuration and create objects needed for DirectDraw surfaces.

    if (!dc) {
        switch (buffertype) {
        case BINKBUFFERPRIMARY: {
            if (options == 0) {
                surfacetype = BinkDDSurfaceType(BufferDirectDrawSurface);

                if (surfacetype != BINKSURFACE8P) {
                    if (surfacetype == BINKSURFACENONE) { radstrcpy(BufferError, "Video mode not supported."); }
                    else {
                        surfacetype = surfacetype | BINKSURFACEDIRECT;
                        noclipping = TRUE;
                        issoftcur = BinkIsSoftwareCursor(BufferDirectDrawSurface, (HCURSOR)GetClassLongA(wnd, GCL_HCURSOR));
                        surface = BufferDirectDrawSurface;
                    }
                }
            }

            break;
        }
        case BINKBUFFERYV12OVERLAY: {
            if ((BufferOverlayCaps & options) == options) {
                surface = BinkBufferDirectDrawOverlaySurfaceCreate(MAKEFOURCC('Y', 'V', '1', '2'), width, height, BINKSURFACEYV12);

                if (surface != NULL) {
                    overlay = TRUE;
                    surfacetype = BinkDDSurfaceType(surface) | BINKSURFACEDIRECT;
                    scales = BufferOverlayCaps;
                }
            }

            break;
        }
        case BINKBUFFERYUY2OVERLAY: {
            if ((BufferOverlayCaps & options) == options) {
                surface = BinkBufferDirectDrawOverlaySurfaceCreate(MAKEFOURCC('Y', 'U', 'Y', '2'), width, height, BINKSURFACEYUY2);

                if (surface != NULL) {
                    overlay = TRUE;
                    surfacetype = BinkDDSurfaceType(surface) | BINKSURFACEDIRECT;
                    scales = BufferOverlayCaps;
                }
            }

            break;
        }
        case BINKBUFFERUYVYOVERLAY: {
            if ((BufferOverlayCaps & options) == options) {
                surface = BinkBufferDirectDrawOverlaySurfaceCreate(MAKEFOURCC('U', 'Y', 'V', 'Y'), width, height, BINKSURFACEUYVY);

                if (surface != NULL) {
                    overlay = TRUE;
                    surfacetype = BinkDDSurfaceType(surface) | BINKSURFACEDIRECT;
                    scales = BufferOverlayCaps;
                }
            }

            break;
        }
        case BINKBUFFERYV12OFFSCREEN: {
            if ((BufferBlitCaps & options) == options) {
                surface = BinkBufferDirectDrawOffscreenSurfaceCreate(MAKEFOURCC('Y', 'V', '1', '2'), width, height, BINKSURFACEYV12);

                if (surface != NULL) {
                    surfacetype = BinkDDSurfaceType(surface) | (options != 0 ? BINKSURFACESLOW : BINKSURFACEFAST);
                    scales = BufferBlitCaps;
                    offscreen = TRUE;
                }
            }

            break;
        }
        case BINKBUFFERYUY2OFFSCREEN:
        {
            if ((BufferBlitCaps & options) == options) {
                surface = BinkBufferDirectDrawOffscreenSurfaceCreate(MAKEFOURCC('Y', 'U', 'Y', '2'), width, height, BINKSURFACEYUY2);

                if (surface != NULL) {
                    surfacetype = BinkDDSurfaceType(surface) | (options != 0 ? BINKSURFACESLOW : BINKSURFACEFAST);
                    scales = BufferBlitCaps;
                    offscreen = TRUE;
                }
            }

            break;
        }
        case BINKBUFFERUYVYOFFSCREEN: {
            if ((BufferBlitCaps & options) == options) {
                surface = BinkBufferDirectDrawOffscreenSurfaceCreate(MAKEFOURCC('U', 'Y', 'V', 'Y'), width, height, BINKSURFACEUYVY);

                if (surface != NULL) {
                    scales = BufferBlitCaps;
                    surfacetype = BinkDDSurfaceType(surface) | (options != 0 ? BINKSURFACESLOW : BINKSURFACEFAST);
                    offscreen = TRUE;
                }
            }

            break;
        }
        case BINKBUFFERRGBOFFSCREENVIDEO:
        case BINKBUFFERRGBOFFSCREENSYSTEM: {
            if ((BufferBlitCaps & options) == options) {
                surface = BinkBufferDirectDrawOffscreenSurfaceCreate(
                    buffertype == BINKBUFFERRGBOFFSCREENVIDEO ? 0 : 1 /* TODO */, width, height, BINKSURFACE8P);

                if (surface != NULL) {
                    surfacetype = BinkDDSurfaceType(surface);

                    if (surfacetype != BINKSURFACE8P) {
                        if (surfacetype == BINKSURFACENONE) {
                            surface->Release();
                            surface = NULL;

                            radstrcpy(BufferError, "Video mode not supported.");

                            break;
                        }

                        surfacetype = surfacetype | (options != 0 ? BINKSURFACESLOW : BINKSURFACEFAST);
                        scales = BufferBlitCaps;
                        offscreen = TRUE;

                        break;
                    }

                    surface->Release();
                    surface = NULL;
                }
            }

            break;
        }
        }

        if (buffertype > BINKBUFFERLAST) {
            if (BufferScreenBits == 0) { radstrcpy(BufferError, "256 color mode not supported."); }
            else if (BufferScreenBits > 8) { radstrcpy(BufferError, "No capable blitting style available."); }

            if (!dd) { BinkBufferDirectDrawRelease(); }
            if (dc) { BinkBufferDeviceContextRelease(); }

            return NULL;
        }

        if (buffertype != BINKBUFFERAUTO) {
            if (surface != NULL) {
                BufferDirectDraw->CreateClipper(0, &clipper, NULL);

                HBINKBUFFER buf = (HBINKBUFFER)radmalloc(sizeof(BINKBUFFER));

                radmemset(buf, 0x00, sizeof(BINKBUFFER));

                buf->Height = height;
                buf->Width = width;
                buf->ScaleFlags = scales;
                buf->SurfaceType = surfacetype;
                buf->ddsurface = surface;
                buf->ddclipper = clipper;
                buf->issoftcur = issoftcur;
                buf->ddoverlay = overlay;
                buf->ddoffscreen = offscreen;
                buf->type = buffertype;
                buf->noclipping = noclipping;
                buf->ScreenWidth = BufferScreenWidth;
                buf->ScreenHeight = BufferScreenHeight;
                buf->ScreenDepth = BufferScreenBits;
                buf->dibh = NULL;
                buf->dibpitch = bufferflags;
                buf->wnd = wnd;
                buf->dibbuffer = NULL;
                buf->diboldbitmap = NULL;
                buf->dibinfo = NULL;
                buf->dibdc = NULL;
                buf->loadeddd = dd;
                buf->loadedwin = dc;

                RECT rect;
                GetWindowRect(wnd, &rect);

                buf->WindowWidth = rect.right - rect.left;
                buf->WindowHeight = rect.bottom - rect.top;

                buf->ClientOffsetX = rect.left;
                buf->ClientOffsetY = rect.top;

                rect.left = 0;
                rect.top = 0;

                ClientToScreen(wnd, (LPPOINT)&rect);

                buf->ClientOffsetX = rect.left - buf->ClientOffsetX;
                buf->ClientOffsetY = rect.top - buf->ClientOffsetY;

                GetClientRect(wnd, &rect);

                buf->ExtraWindowWidth = buf->WindowWidth - rect.right;
                buf->ExtraWindowHeight = buf->WindowHeight - rect.bottom;

                buf->WindowWidth = buf->ExtraWindowWidth + buf->Width;
                buf->WindowHeight = buf->WindowHeight - rect.bottom + buf->Height;

                BinkBufferSetOffset(buf, 0, 0);
                BinkBufferSetScale(buf, buf->Width, buf->Height);

                if (buf->ddclipper != NULL) { ((LPDIRECTDRAWCLIPPER)buf->ddclipper)->SetHWnd(0, wnd); }

                return buf;
            }

            if (BufferError[0] == NULL) {
                if (BufferScreenBits == 0) { radstrcpy(BufferError, "256 color mode not supported."); }
                else if (BufferScreenBits > 8) { radstrcpy(BufferError, "No capable blitting style available."); }

                if (!dd) { BinkBufferDirectDrawRelease(); }
                if (dc) { BinkBufferDeviceContextRelease(); }

                return NULL;
            }
        }

        BufferError[0] = NULL;

        if (buffertype != BINKBUFFERPRIMARY) {
            if (BufferScreenBits == 0) { radstrcpy(BufferError, "256 color mode not supported."); }
            else if (BufferScreenBits > 8) { radstrcpy(BufferError, "No capable blitting style available."); }

            if (!dd) { BinkBufferDirectDrawRelease(); }
            if (dc) { BinkBufferDeviceContextRelease(); }

            return NULL;
        }
    }

    if (BufferBytes != 0) {
        HDC hdc = GetDC(wnd);
        HDC dibdc = CreateCompatibleDC(hdc);
        ReleaseDC(wnd, hdc);

        if (dibdc != NULL) {
            LPBITMAPINFO bmp = (LPBITMAPINFO)radmalloc(BINK_BUFFER_BITMAP_MEMORY_SIZE);

            if (bmp != NULL) {
                radmemset(bmp, 0x00, BINK_BUFFER_BITMAP_MEMORY_SIZE);

                bmp->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
                bmp->bmiHeader.biWidth = (BufferBytes * width + 0x1F & 0xFFFFFFE0) / BufferBytes;
                bmp->bmiHeader.biHeight = -1 * (s32)height;
                bmp->bmiHeader.biPlanes = 1;
                bmp->bmiHeader.biBitCount = (WORD)(BufferBytes * 8);
                bmp->bmiHeader.biCompression = BufferBytes == 3 ? BI_BITFIELDS : BI_RGB;

                surfacetype = BinkBufferDeviceContextSurfaceType(hdc, bmp->bmiColors);

                void* buffer = NULL;
                HBITMAP dib = CreateDIBSection(dibdc, bmp, 0, &buffer, NULL, 0);

                if (dib != NULL) {
                    HGDIOBJ gdi = SelectObject(dibdc, dib);
                    buffertype = BINKBUFFERDIBSECTION;
                    surfacetype |= (options != 0 ? BINKSURFACESLOW : BINKSURFACEFAST);
                    scales = BINKBUFFERSCALES;

                    HBINKBUFFER buf = (HBINKBUFFER)radmalloc(sizeof(BINKBUFFER));

                    radmemset(buf, 0x00, sizeof(BINKBUFFER));

                    buf->Height = height;
                    buf->Width = width;
                    buf->ScaleFlags = scales;
                    buf->SurfaceType = surfacetype;
                    buf->ddsurface = surface;
                    buf->ddclipper = clipper;
                    buf->issoftcur = issoftcur;
                    buf->ddoverlay = overlay;
                    buf->ddoffscreen = offscreen;
                    buf->type = buffertype;
                    buf->noclipping = noclipping;
                    buf->ScreenWidth = BufferScreenWidth;
                    buf->ScreenHeight = BufferScreenHeight;
                    buf->ScreenDepth = BufferScreenBits;
                    buf->dibh = dib;
                    buf->dibpitch = (bmp->bmiHeader.biWidth * BufferBytes + 3) & 0xFFFFFFFC;
                    buf->wnd = wnd;
                    buf->dibbuffer = buffer;
                    buf->diboldbitmap = gdi;
                    buf->dibinfo = bmp;
                    buf->dibdc = dibdc;
                    buf->loadeddd = dd;
                    buf->loadedwin = dc;

                    RECT rect;
                    GetWindowRect(wnd, &rect);

                    buf->WindowWidth = rect.right - rect.left;
                    buf->WindowHeight = rect.bottom - rect.top;

                    buf->ClientOffsetX = rect.left;
                    buf->ClientOffsetY = rect.top;

                    rect.left = 0;
                    rect.top = 0;

                    ClientToScreen(wnd, (LPPOINT)&rect);

                    buf->ClientOffsetX = rect.left - buf->ClientOffsetX;
                    buf->ClientOffsetY = rect.top - buf->ClientOffsetY;

                    GetClientRect(wnd, &rect);

                    buf->ExtraWindowWidth = buf->WindowWidth - rect.right;
                    buf->ExtraWindowHeight = buf->WindowHeight - rect.bottom;

                    buf->WindowWidth = buf->ExtraWindowWidth + buf->Width;
                    buf->WindowHeight = buf->WindowHeight - rect.bottom + buf->Height;

                    BinkBufferSetOffset(buf, 0, 0);
                    BinkBufferSetScale(buf, buf->Width, buf->Height);

                    if (buf->ddclipper != NULL) { ((LPDIRECTDRAWCLIPPER)buf->ddclipper)->SetHWnd(0, wnd); }

                    return buf;
                }

                DeleteDC(dibdc);
                radfree(bmp);
            }
        }
    }

    if (!dd) { BinkBufferDirectDrawRelease(); }
    if (dc) { BinkBufferDeviceContextRelease(); }

    return NULL;
}

// 0x100072a0
void RADEXPLINK BinkBufferClose(HBINKBUFFER buf)
{
    if (buf != NULL && buf->Width != 0) {
        if (!buf->ddoverlay) {
            if (buf->ddoffscreen) { ((LPDIRECTDRAWSURFACE)buf->ddsurface)->Release(); }
        }
        else {
            LPDIRECTDRAWSURFACE surface = (LPDIRECTDRAWSURFACE)buf->ddsurface;

            surface->UpdateOverlay(NULL, BufferDirectDrawSurface, NULL, DDOVER_HIDE, NULL);
            surface->Release();
        }

        HDC hdc = buf->dibdc;

        if (hdc != NULL) {
            SelectObject(hdc, buf->diboldbitmap);
            DeleteObject(buf->dibh);
            DeleteDC(hdc);
            radfree(buf->dibinfo);
        }

        if (buf->loadeddd) { BinkBufferDirectDrawRelease(); }
        if (buf->loadedwin) { BinkBufferDeviceContextRelease(); }

        radmemset(buf, 0x00, sizeof(BINKBUFFER));

        radfree(buf);
    }
}

// 0x10007360
s32 RADEXPLINK BinkBufferLock(HBINKBUFFER buf)
{
    if (buf == NULL) { return FALSE; }

    if (buf->ddsurface == NULL) {
        if (buf->dibh != NULL) {
            buf->Buffer = buf->dibbuffer;
            buf->BufferPitch = buf->dibpitch;
        }

        return TRUE;
    }

    DDSURFACEDESC desc;
    radmemset(&desc, 0x00, sizeof(DDSURFACEDESC));

    desc.dwSize = sizeof(DDSURFACEDESC);

    if (buf->issoftcur) { buf->cursorcount = BinkCheckCursor(NULL, buf->destx, buf->desty, buf->Width, buf->Height); }

    LPDIRECTDRAWSURFACE surface = (LPDIRECTDRAWSURFACE)buf->ddsurface;

    while (FAILED(surface->Lock(NULL, &desc, DDLOCK_WAIT, NULL))) {
        buf->SurfaceType = buf->SurfaceType | BINKCOPYALL;

        if (FAILED(surface->Restore())) {
            if (buf->issoftcur) { BinkRestoreCursor(buf->cursorcount); }

            return FALSE;
        }
    }

    buf->buffertop = desc.lpSurface;

    if (!buf->ddoverlay && !buf->ddoffscreen) {
        buf->Buffer = (void*)((size_t)desc.lpSurface + buf->desty * desc.lPitch + buf->destx * BufferBytes);
        buf->BufferPitch = desc.lPitch;

        return TRUE;
    }

    buf->Buffer = desc.lpSurface;
    buf->BufferPitch = desc.lPitch;

    return TRUE;
}

// 0x10007490
s32 RADEXPLINK BinkBufferUnlock(HBINKBUFFER buf)
{
    if (buf == NULL) { return FALSE; }

    if (buf->ddsurface == NULL) {
        if (buf->dibh != NULL) {
            buf->Buffer = NULL;
            buf->BufferPitch = 0;
        }
    }
    else {
        LPDIRECTDRAWSURFACE surface = (LPDIRECTDRAWSURFACE)buf->ddsurface;
        surface->Unlock(buf->buffertop);

        if (buf->issoftcur) { BinkRestoreCursor(buf->cursorcount); }

        buf->Buffer = NULL;
        buf->BufferPitch = 0;

        BinkBufferMove(buf, BINKBUFFERMOVEMODE_SHOW);
    }

    buf->SurfaceType = buf->SurfaceType & (~BINKCOPYALL);

    return TRUE;
}

// 0x10007500
void RADEXPLINK BinkBufferBlit(HBINKBUFFER buf, BINKRECT PTR4* rects, u32 numrects)
{
    if (buf != NULL && numrects != 0) {
        if (buf->ddoverlay) {
            BufferDirectDrawSurface->SetClipper((LPDIRECTDRAWCLIPPER)buf->ddclipper);

            DDBLTFX fx;
            fx.dwSize = sizeof(DDBLTFX);
            fx.dwFillColor = BufferFillColor;

            RECT rect;

            if (rects == NULL) {
                rect.left = buf->destx;
                rect.top = buf->desty;
                rect.right = buf->StretchWidth + rect.left;
                rect.bottom = buf->StretchHeight + rect.top;

                BufferDirectDrawSurface->Blt(&rect, NULL, &rect, DDBLT_WAIT | DDBLT_COLORFILL, &fx);
            }
            else {
                for (u32 i = 0; i < numrects; i++) {
                    rect.left = buf->destx + rects[i].Left;
                    rect.top = rects[i].Top + buf->desty;
                    rect.right = rects[i].Width + rect.left;
                    rect.bottom = rects[i].Height + rect.top;

                    BufferDirectDrawSurface->Blt(&rect, NULL, &rect, DDBLT_WAIT | DDBLT_COLORFILL, &fx);
                }
            }

            BufferDirectDrawSurface->SetClipper(NULL);

            return;
        }

        if (buf->ddoffscreen) {
            BufferDirectDrawSurface->SetClipper((LPDIRECTDRAWCLIPPER)buf->ddclipper);

            RECT src, dst;

            if (rects == NULL) {
                src.left = buf->destx;
                src.top = buf->desty;
                src.right = buf->StretchWidth + src.left;
                src.bottom = buf->StretchHeight + src.top;

                dst.right = buf->Width;
                dst.bottom = buf->Height;
                dst.top = 0;
                dst.left = 0;

                BufferDirectDrawSurface->Blt(&src, (LPDIRECTDRAWSURFACE)buf->ddsurface, &dst, DDBLT_WAIT, NULL);
            }
            else {
                for (u32 i = 0; i < numrects; i++) {
                    src.left = buf->destx + ((rects[i].Left * buf->StretchWidth) / buf->Width);
                    src.top = buf->desty + ((rects[i].Top * buf->StretchHeight) / buf->Height);
                    src.right = ((rects[i].Width * buf->StretchWidth) / buf->Width) + src.left;
                    src.bottom = ((rects[i].Height * buf->StretchHeight) / buf->Height) + src.top;

                    dst.top = rects[i].Top;
                    dst.left = rects[i].Left;
                    dst.right = rects[i].Width + dst.left;
                    dst.bottom = rects[i].Height + dst.top;

                    BufferDirectDrawSurface->Blt(&src, (LPDIRECTDRAWSURFACE)buf->ddsurface, &dst, DDBLT_WAIT, NULL);
                }
            }

            BufferDirectDrawSurface->SetClipper(NULL);

            return;
        }

        if (buf->dibh != NULL) {
            HDC hdc = GetDC((HWND)buf->wnd);
            SetStretchBltMode(hdc, COLORONCOLOR);

            if (rects == NULL) {
                StretchBlt(hdc, buf->wndx, buf->StretchWidth, buf->StretchWidth, buf->StretchHeight,
                    (HDC)buf->dibdc, 0, 0, buf->Width, buf->Height, SRCCOPY);
            }
            else {
                for (u32 i = 0; i < numrects; i++) {
                    const s32 x = buf->wndx + (rects[i].Left * buf->StretchWidth / buf->Width);
                    const s32 y = buf->wndy + (rects[i].Top * buf->StretchHeight / buf->Height);
                    const s32 w = rects[i].Width * buf->StretchWidth / buf->Width;
                    const s32 h = rects[i].Height * buf->StretchHeight / buf->Height;

                    StretchBlt(hdc, x, y, w, h,
                        (HDC)buf->dibdc, rects[i].Left, rects[i].Top, rects[i].Width, rects[i].Height, SRCCOPY);
                }
            }

            ReleaseDC((HWND)buf->wnd, hdc);
        }
    }
}

// 0x10007880
s32 RADEXPLINK BinkBufferSetScale(HBINKBUFFER buf, u32 w, u32 h)
{
    if (buf != NULL) {
        if (w == 0) { w = Width; }
        if (h == 0) { h = Height; }

        u32 options = 0, result = 0;

        if (w != buf->Width) {
            if (w % buf->Width == 0) { options = BINKBUFFERSTRETCHXINT; }
            else if (w != buf->Width) {
                if (buf->Width % w == 0) { options = BINKBUFFERSHRINKXINT; }
                else if (buf->Width < w) { options = BINKBUFFERSTRETCHX; }
                else if (w < buf->Width) { options = BINKBUFFERSHRINKX; }
            }
        }

        result = (buf->ScaleFlags & options) == options;

        if (result) { buf->StretchWidth = w; }

        options = 0;

        if (h != buf->Height) {
            if (w % buf->Height == 0) { options = BINKBUFFERSTRETCHYINT; }
            else if (h != buf->Height) {
                if (buf->Height % h == 0) { options = BINKBUFFERSHRINKYINT; }
                else if (buf->Height < h) { options = BINKBUFFERSTRETCHY; }
                else if (h < buf->Height) { options = BINKBUFFERSHRINKY; }
            }
        }

        if ((buf->ScaleFlags & options) == options) { buf->StretchHeight = h; }
        else { result = 0; }

        buf->WindowWidth = buf->StretchWidth + buf->ExtraWindowWidth;
        buf->WindowHeight = buf->StretchHeight + buf->ExtraWindowHeight;

        return result;
    }

    return 0;
}

// 0x10007990
s32 RADEXPLINK BinkBufferSetHWND(HBINKBUFFER buf, HWND newwnd)
{
    return FALSE;
}

// 0x100079a0
char PTR4* RADEXPLINK BinkBufferGetDescription(HBINKBUFFER buf)
{
    if (buf == NULL) { return NULL; }

    switch (buf->type) {
    case BINKBUFFERPRIMARY: {
        radstrcpy(BufferDescription, "DirectDraw primary surface");
        return BufferDescription;
    }
    case BINKBUFFERDIBSECTION: {
        radstrcpy(BufferDescription, "DIB Section");
        return BufferDescription;
    }
    case BINKBUFFERYV12OVERLAY: {
        radstrcpy(BufferDescription, "DirectDraw YV12 overlay");
        return BufferDescription;
    }
    case BINKBUFFERYUY2OVERLAY: {
        radstrcpy(BufferDescription, "DirectDraw YUY2 overlay");
        return BufferDescription;
    }
    case BINKBUFFERUYVYOVERLAY: {
        radstrcpy(BufferDescription, "DirectDraw UYVY overlay");
        return BufferDescription;
    }
    case BINKBUFFERYV12OFFSCREEN: {
        radstrcpy(BufferDescription, "DirectDraw YV12 off-screen surface");
        return BufferDescription;
    }
    case BINKBUFFERYUY2OFFSCREEN: {
        radstrcpy(BufferDescription, "DirectDraw YUY2 off-screen surface");
        return BufferDescription;
    }
    case BINKBUFFERUYVYOFFSCREEN: {
        radstrcpy(BufferDescription, "DirectDraw UYVY off-screen surface");
        return BufferDescription;
    }
    case BINKBUFFERRGBOFFSCREENVIDEO: {
        radstrcpy(BufferDescription, "DirectDraw RGB off-screen video surface");
        return BufferDescription;
    }
    case BINKBUFFERRGBOFFSCREENSYSTEM: {
        radstrcpy(BufferDescription, "DirectDraw RGB off-screen system surface");
        return BufferDescription;
    }
    }

    return BufferDescription;
}

// 0x10007b20
char PTR4* RADEXPLINK BinkBufferGetError()
{
    return BufferError;
}

// 0x10007b30
s32 RADEXPLINK BinkBufferClear(HBINKBUFFER buf, u32 RGB)
{
    if (BinkBufferLock(buf)) {
        BinkBufferClear(buf->Buffer, buf->SurfaceType, buf->BufferPitch, buf->Width, buf->Height);
        BinkBufferUnlock(buf);
        return TRUE;
    }

    return FALSE;
}

// 0x10007b80
s32 RADEXPLINK BinkDDSurfaceType(void PTR4* lpDDS)
{
    if (lpDDS == NULL) { return BINKSURFACENONE; }

    LPDIRECTDRAWSURFACE surface = (LPDIRECTDRAWSURFACE)lpDDS;

    DDPIXELFORMAT format;
    radmemset(&format, 0x00, sizeof(DDPIXELFORMAT));

    format.dwSize = sizeof(DDPIXELFORMAT);

    surface->GetPixelFormat(&format);

    if (format.dwFourCC == MAKEFOURCC('U', 'Y', 'V', 'Y')) { return BINKSURFACEUYVY; }
    if (format.dwFourCC == MAKEFOURCC('Y', 'V', '1', '2')) { return BINKSURFACEYV12; }
    if (format.dwFourCC == MAKEFOURCC('Y', 'U', 'Y', '2')) { return BINKSURFACEYUY2; }

    if (format.dwRGBBitCount == 8) { return BINKSURFACE8P; }
    if (format.dwRGBBitCount == 24) { return (format.dwRBitMask != 0x00FF0000) + 1; }
    if (format.dwRGBBitCount == 32) {
        if (format.dwRGBAlphaBitMask != 0) { return (format.dwRBitMask != 0x00FF0000) + 5; }

        return (format.dwRBitMask != 0x00FF0000) + 3;
    }

    if (format.dwRBitMask == 0xF800 && format.dwGBitMask == 0x7E0 && format.dwBBitMask == 0x1F) { return BINKSURFACE565; }


    if (format.dwRGBAlphaBitMask == 0x8000) {
        if (format.dwRBitMask == 0x7C00) {
            if (format.dwGBitMask != 0x3E0) { return BINKSURFACENONE; }
            if (format.dwBBitMask != 0x1F) { return BINKSURFACENONE; }

            return BINKSURFACE5551;
        }
    }
    else if (format.dwRBitMask == 0x7C00) {
        if (format.dwGBitMask == 0x3E0 && format.dwBBitMask == 0x1F) { return BINKSURFACE555; }
        if (format.dwRGBAlphaBitMask == 0xF000 && format.dwRBitMask == 0xF00
            && format.dwGBitMask == 0xF0 && format.dwBBitMask == 0xF) {
            return BINKSURFACE4444;
        }

        return BINKSURFACENONE;
    }
    if (format.dwRBitMask == 0xFC00) {
        if (format.dwGBitMask == 0x3E0) {
            if (format.dwBBitMask == 0x1F) { return BINKSURFACE655; }
        }
        else if (format.dwGBitMask == 0x3F0 && format.dwBBitMask == 0xF) { return BINKSURFACE664; }
    }

    if (format.dwRGBAlphaBitMask == 0xF000 && format.dwRBitMask == 0xF00
        && format.dwGBitMask == 0xF0 && format.dwBBitMask == 0xF) {
        return BINKSURFACE4444;
    }

    return BINKSURFACENONE;
}

// 0x10007e10
void RADEXPLINK BinkSetError(const char PTR4* err)
{
    radstrcpy(BinkError, err);
}

// 0x10007e40
char PTR4* RADEXPLINK BinkGetError(void)
{
    return BinkError;
}

// 0x10007e50
s32 RADEXPLINK BinkSetSoundSystem(BINKSNDSYSOPEN open, void* param)
{
    if (open == NULL) { return FALSE; }

    if (SoundOpen != NULL) {
        if (SoundOpen == open) {
            const BINKSNDOPEN result = SoundOpen(param);

            if (result) { SoundSystem = result; }

            return SoundSystem != NULL;
        }

        if (SoundCounter != 0) { return FALSE; }
    }

    SoundOpen = open;

    const BINKSNDOPEN result = SoundOpen(param);

    if (result) { SoundSystem = result; }

    return SoundSystem != NULL;
}

// 0x10007ea0
void RADEXPLINK BinkSetFrameRate(u32 forcerate, u32 forceratediv)
{
    FrameRate = forcerate;
    FrameRateDiv = forceratediv;
}

// 0x10007ec0
void RADEXPLINK BinkSetIOSize(u32 iosize)
{
    IOSize = iosize;
}

// 0x10007ed0
void RADEXPLINK BinkSetIO(BINKIOOPEN io)
{
    IO = io;
}

// 0x10007ee0
void RADEXPLINK BinkSetSimulate(u32 sim)
{
    Simulate = sim;
}

// 0x10007ef0
void RADEXPLINK BinkSetSoundTrack(u32 track)
{
    SoundTrack = track;
}

// 0x10007f00
HBINK RADEXPLINK BinkOpen(const char PTR4* name, u32 flags) // TODO refactor this!
{
    BINK bink;
    BINKIOHEADER header;

    radmemset(&bink, 0x00, sizeof(BINK));

    bink.timeopen = RADTimerRead();

    BinkError[0] = NULL;

    if (flags & BINKFROMMEMORY) { radmemcpy(&header, name, sizeof(BINKIOHEADER)); }
    else {
        BINKIOOPEN action = BinkOpenFile;

        if ((flags & BINKIOPROCESSOR) && IO != NULL) { action = IO; }

        IO = NULL;

        if (action(&bink.bio, name, flags) == 0) {
            if (BinkError[0] == NULL) { BinkSetError("Error opening file."); }

            return NULL;
        }

        bink.bio.ReadHeader(&bink.bio, 0, &header, sizeof(BINKIOHEADER));
    }

    if (header.Magic != BINKMARKER1 && header.Magic != BINKMARKER2
        && header.Magic != BINKMARKER3 && header.Magic != BINKMARKER4) {
        BinkSetError("Not a Bink file.");

        if (!(flags & BINKFROMMEMORY)) { bink.bio.Close(&bink.bio); }

        return NULL;
    }

    if (header.Frames == 0) {
        BinkSetError("The file doesn't contain any compressed frames yet.");

        if (!(flags & BINKFROMMEMORY)) { bink.bio.Close(&bink.bio); }

        return NULL;
    }

    bink.UVWidth = (((header.Width + 1) >> 1) + 7) & 0xFFFFFFF8;
    bink.UVHeight = (((header.Height + 1) >> 1) + 7) & 0xFFFFFFF8;

    bink.YWidth = bink.UVWidth * 2;
    bink.YHeight = bink.UVHeight * 2;

    bink.MaskPitch = bink.YWidth >> 4;

    bink.Width = header.Width;
    bink.Height = header.Height;
    bink.decompwidth = header.Width;
    bink.decompheight = header.Height;
    bink.BinkType = header.Options;

    {
        u32 options = flags & ~BINKCOPYNOSCALING;

        if (!(header.Options & BINKALPHA)) { options = flags & ~(BINKCOPYNOSCALING | BINKALPHA); }

        bink.OpenFlags = header.Options & BINKGRAYSCALE | options;
    }

    {
        u32 scaling = flags & BINKCOPYNOSCALING;

        if (scaling != BINKCOPYNOSCALING) {
            if (scaling == 0) { scaling = header.Options & BINKCOPYNOSCALING; }

            bink.OpenFlags = bink.OpenFlags | scaling;

            if (scaling <= BINKCOPY2XW) {
                if (scaling == BINKCOPY2XW) { bink.Width = header.Width * 2; }
                else if (scaling == BINKCOPY2XH || scaling == BINKCOPY2XHI) { bink.Height = header.Height * 2; }
            }
            else if (scaling == BINKCOPY2XWH || scaling == BINKCOPY2XWHI) {
                bink.Width = header.Width * 2;
                bink.Height = header.Height * 2;
            }
        }

        if (header.Magic == BINKMARKER1 || header.Magic == BINKMARKER2) {
            bink.OpenFlags = bink.OpenFlags | (BINKRBINVERT | BINKOLDFRAMEFORMAT);
        }
        else if (header.Magic == BINKMARKER3) { bink.OpenFlags = bink.OpenFlags | BINKOLDFRAMEFORMAT; }
    }

    bink.Frames = header.Frames;
    bink.InternalFrames = header.InternalFrames;

    if (!(flags & BINKFRAMERATE) || FrameRate == DEFAULT_FRAME_RATE) {
        bink.FrameRate = header.Dividend;
        bink.FrameRateDiv = header.Divider;
    }
    else {
        bink.FrameRate = FrameRate;
        bink.FrameRateDiv = FrameRateDiv;
        FrameRate = DEFAULT_FRAME_RATE;
    }

    bink.Size = header.Size;
    bink.NumTracks = header.Tracks;
    bink.LargestFrameSize = header.MaxFrameSize;
    bink.fileframerate = header.Dividend;
    bink.runtimeframes = ((header.Divider >> 1) + header.Dividend) / header.Divider;
    bink.MaskLength = bink.MaskPitch * bink.YHeight >> 4;
    bink.fileframeratediv = header.Divider;

    if (bink.runtimeframes == 0) { bink.runtimeframes = 1; }

    bink.runtimemoveamt = (bink.runtimeframes - 1) * sizeof(size_t);

    BinkQueueMem(&bink.MaskPlane, (size_t)(bink.MaskLength + 16));
    BinkQueueMem(&bink.rtframetimes, (size_t)(bink.runtimeframes * sizeof(size_t)));
    BinkQueueMem(&bink.rtadecomptimes, (size_t)(bink.runtimeframes * sizeof(size_t)));
    BinkQueueMem(&bink.rtvdecomptimes, (size_t)(bink.runtimeframes * sizeof(size_t)));
    BinkQueueMem(&bink.rtblittimes, (size_t)(bink.runtimeframes * sizeof(size_t)));
    BinkQueueMem(&bink.rtreadtimes, (size_t)(bink.runtimeframes * sizeof(size_t)));
    BinkQueueMem(&bink.rtidlereadtimes, (size_t)(bink.runtimeframes * sizeof(size_t)));
    BinkQueueMem(&bink.rtthreadreadtimes, (size_t)(bink.runtimeframes * sizeof(size_t)));

    ExpandBundleSizes(&bink.bunp, bink.YWidth);

    BinkQueueMem(&bink.bunp, (size_t)bink.bunp.typeptr);
    BinkQueueMem(&bink.bunp.type16ptr, (size_t)bink.bunp.type16ptr);
    BinkQueueMem(&bink.bunp.colorptr, (size_t)bink.bunp.colorptr);
    BinkQueueMem(&bink.bunp.bits2ptr, (size_t)bink.bunp.bits2ptr);
    BinkQueueMem(&bink.bunp.motionXptr, (size_t)bink.bunp.motionXptr);
    BinkQueueMem(&bink.bunp.motionYptr, (size_t)bink.bunp.motionYptr);
    BinkQueueMem(&bink.bunp.dctptr, (size_t)bink.bunp.dctptr);
    BinkQueueMem(&bink.bunp.mdctptr, (size_t)bink.bunp.mdctptr);
    BinkQueueMem(&bink.bunp.patptr, (size_t)bink.bunp.patptr);

    if (!(flags & BINKFROMMEMORY)) {
        BinkQueueMem(&bink.frameoffsets, (bink.InternalFrames + 1) * sizeof(size_t));
        BinkQueueMem(&bink.tracksizes, bink.NumTracks * sizeof(size_t));
        BinkQueueMem(&bink.tracktypes, bink.NumTracks * sizeof(size_t));
        BinkQueueMem(&bink.trackIDs, bink.NumTracks * sizeof(size_t));
    }

    HBINK result = (HBINK)BinkAllocateMem(&bink, sizeof(BINK));

    if (result != NULL)
    {
        radmemcpy(result, &bink, sizeof(BINK));

        result->bio.bink = result;

        result->rtadecomptimes[0] = 0;
        result->rtvdecomptimes[0] = 0;
        result->rtblittimes[0] = 0;
        result->rtreadtimes[0] = 0;
        result->rtidlereadtimes[0] = 0;
        result->rtthreadreadtimes[0] = 0;

        if (flags & BINKFROMMEMORY) {
            result->tracksizes = (u32*)((size_t)name + sizeof(BINKIOHEADER));
            result->tracktypes = (u32*)((size_t)name + result->NumTracks * 1 * sizeof(u32) + sizeof(BINKIOHEADER));
            result->trackIDs = (s32*)((size_t)name + result->NumTracks * 2 * sizeof(u32) + sizeof(BINKIOHEADER));
            result->frameoffsets = (u32*)((size_t)name + result->NumTracks * 3 * sizeof(u32) + sizeof(BINKIOHEADER));
        }
        else {
            result->bio.ReadHeader(&result->bio, -1, result->tracksizes, result->NumTracks * sizeof(s32));
            result->bio.ReadHeader(&result->bio, -1, result->tracktypes, result->NumTracks * sizeof(s32));
            result->bio.ReadHeader(&result->bio, -1, result->trackIDs, result->NumTracks * sizeof(s32));
            result->bio.ReadHeader(&result->bio, -1, result->frameoffsets, (result->InternalFrames + 1) * sizeof(u32));
        }

        u32 local_8 = 0; // TODO Name
        result->Highest1SecRate = BinkCalculateHighest1SecFrameRate(result->Frames,
            result->frameoffsets, result->runtimeframes, &result->Highest1SecFrame, &local_8);

        if (!(result->OpenFlags & BINKALPHA)) {
            if (local_8 == 0) {
                BinkQueueMem(&result->YPlane[1],
                    result->YHeight * result->YWidth + result->UVHeight * result->UVWidth * 2);
            }
        }
        else {
            BinkQueueMem(&result->APlane[0], result->YWidth * result->YHeight);

            if (local_8 == 0) {
                BinkQueueMem(&result->APlane[1], result->YWidth * result->YHeight);

                if (local_8 == 0) {
                    BinkQueueMem(&result->YPlane[1],
                        result->YHeight * result->YWidth + result->UVHeight * result->UVWidth * 2);
                }
            }
        }

        result->YPlane[0] =
            BinkAllocateMem(result, result->YHeight * result->YWidth + result->UVHeight * result->UVWidth * 2);

        if (result->YPlane[0] != NULL) {
            if (local_8 != 0) {
                result->YPlane[1] = result->YPlane[0];
                result->APlane[1] = result->APlane[0];
            }

            if (!(flags & BINKIOSIZE) || IOSize == DEFAULT_IO_SIZE) {
                result->iosize = result->Highest1SecRate;
            }
            else {
                result->iosize = IOSize;
                IOSize = DEFAULT_IO_SIZE;
            }

            u32 simulate = 0;

            if (!(flags & BINKSIMULATE) || Simulate == DEFAULT_SIMULATE) {
                simulate = 0;
            }
            else {
                simulate = Simulate;
                Simulate = DEFAULT_SIMULATE;
            }

            if (flags & BINKFROMMEMORY) {
                result->preloadptr = (void*)((size_t)name + ALIGN_FRAME_OFFSETS(result->frameoffsets[0]));

            LAB_1000876e: // TODO

                ((u8*)result->MaskPlane)[result->MaskLength] = 0;
                result->FrameNum = DEFAULT_FRAME_NUM;

                if (result->FrameRate == 0) { result->twoframestime = 2000; }
                else { result->twoframestime = (result->FrameRateDiv * 2000) / result->FrameRate; }

                result->videoon = TRUE;

                BinkFrameRead(result, 1);

                result->timeopen = RADTimerRead() - result->timeopen;

                if (result->preloadptr == NULL && !(flags & BINKNOFILLIOBUF)) {
                    while (result->bio.Idle(&result->bio)) {}
                }

                if (result->NumTracks == 0) { result->trackindex = BINKNOSOUND; }
                else if (!(result->OpenFlags & BINKSNDTRACK)) { result->trackindex = 0; }
                else if (SoundTrack == BINKNOSOUND) { result->trackindex = BINKNOSOUND; }
                else {
                    s32 track = BINKNOSOUND;

                    for (s32 x = 0; x < result->NumTracks; x++) {
                        if (result->trackIDs[x] == SoundTrack) {
                            track = x;
                            break;
                        }
                    }

                    result->trackindex = track;
                }

                SoundTrack = BINKNOSOUND;

                if (result->trackindex != BINKNOSOUND) {
                    if (SOUND_TRACK_TYPE_IS_ACTIVE(result->tracktypes[result->trackindex]) && SoundSystem != NULL) {
                        u32 freq = SOUND_TRACK_TYPE_FREQUENCY(result->tracktypes[result->trackindex]);

                        if (bink.FrameRate != 0 && bink.FrameRateDiv != 0) {
                            freq = freq * (bink.fileframeratediv * bink.FrameRate) / (bink.fileframerate * bink.FrameRateDiv);
                        }

                        if (SoundSystem(&result->bsnd, freq, SOUND_TRACK_TYPE_BITS(result->tracktypes[result->trackindex]),
                            SOUND_TRACK_TYPE_CHANNELS(result->tracktypes[result->trackindex]), result->OpenFlags, result)) {
                            SoundCounter = SoundCounter + 1;
                            result->sndconvert8 = SOUND_TRACK_TYPE_CONVERT_TO_8BITS(result->tracktypes[result->trackindex]);

                            result->soundon = TRUE;
                            result->sndbufsize = SOUND_BUFFER_SIZE(result->tracksizes[result->trackindex]);

                            result->sndbuf = (u8*)radmalloc(result->sndbufsize);
                            result->sndwritepos = result->sndbuf;
                            result->sndreadpos = result->sndbuf;

                            result->sndend = (u8*)((size_t)result->sndbuf + result->sndbufsize);
                            result->sndprime = ((SOUND_TRACK_TYPE_CHANNELS(result->tracktypes[result->trackindex])
                                * freq * 2) * (0x2EE - result->bsnd.Latency) / 1000) & 0xFFFFFFFC;

                            if (result->sndbufsize < result->sndprime) { result->sndprime = result->sndbufsize; }

                            result->sndcomp = BinkSoundCompressionInitialize(
                                SOUND_TRACK_TYPE_FREQUENCY(result->tracktypes[result->trackindex]),
                                SOUND_TRACK_TYPE_CHANNELS(result->tracktypes[result->trackindex]), 1); // TODO

                            result->sndamt = 0;
                            result->sndendframe = result->Frames - (result->fileframerate * 3) / (result->fileframeratediv << 2);
                        }
                    }

                    if (!result->soundon) { result->trackindex = BINKNOSOUND; }
                }

                if (result->trackindex != BINKNOSOUND
                    || (!(result->OpenFlags & BINKNOTHREADEDIO) && result->preloadptr == NULL)) {
                    result->threadcontrol = THREAD_CONTROL_ACTIVE;
                    result->BackgroundThread = BinkCreateThread(BinkThreadAction, result, NULL);

                    if (result->BackgroundThread == NULL) {
                        result->threadcontrol = THREAD_CONTROL_NONE;

                        BinkClose(result);

                        return NULL;
                    }
                }

                return result;
            }

            result->iosize = result->bio.GetBufferSize(&result->bio, result->iosize);

            if ((result->Size * 9) / 10 <= result->iosize) {
                flags = flags | BINKPRELOADALL;
                result->OpenFlags = result->OpenFlags | BINKPRELOADALL;
            }

            if (!(flags & BINKPRELOADALL)) {
                BinkQueueMem(&result->compframe, result->LargestFrameSize);

                result->ioptr = (u8*)BinkAllocateMem(result, result->iosize);

                if (result->ioptr == NULL) { result->iosize = 0; }

                result->bio.SetInfo(&result->bio, result->ioptr, result->iosize, result->Size + 8, simulate);

                goto LAB_1000876e;
            }

            const u32 size = result->Size - ALIGN_FRAME_OFFSETS(result->frameoffsets[0]) + 8;

            result->preloadptr = BinkAllocateMem(result, size);

            if (result->preloadptr != NULL) {
                result->bio.SetInfo(&result->bio, NULL, 0, result->Size + 8, simulate);
                result->bio.ReadFrame(&result->bio, 0, ALIGN_FRAME_OFFSETS(result->frameoffsets[0]), result->preloadptr, size);
                result->bio.Close(&result->bio);

                result->bio.ForegroundTime = 0;

                goto LAB_1000876e;
            }

            radfree(bink.YPlane);
        }

        radfree(result);
    }

    BinkSetError("Out of memory.");

    if (!(flags & BINKFROMMEMORY)) { bink.bio.Close(&bink.bio); }

    return NULL;
}

// 0x10009040
s32 RADEXPLINK BinkCopyToBuffer(HBINK bnk, void* dest, s32 destpitch, u32 destheight, u32 destx, u32 desty, u32 flags)
{
    if (bnk == NULL) { return FALSE; }
    if (dest == NULL) { return FALSE; }

    bnk->bio.Working = TRUE;
    const u32 lastblitflags = flags | bnk->OpenFlags & (BINKCOPYNOSCALING | BINKGRAYSCALE | BINKRBINVERT);

    YUV_init(flags & BINKSURFACEMASK);

    if (destpitch < 0) {
        dest = (void*)((size_t)dest - (destheight - desty - 1) * destpitch);
        desty = 0;
    }

    s32 result = FALSE;

    if (bnk->trackindex != BINKNOSOUND) {
        BinkSoundFrameFill(bnk);

        if (bnk->trackindex != BINKNOSOUND && bnk->FrameRate != 0 && !bnk->Paused) {
            BinkSetStartSyncTime(bnk);

            const u32 start = RADTimerRead() - bnk->startsynctime;
            const u32 end = (1000 * bnk->playedframes - 1000 * bnk->startsyncframe) * bnk->FrameRateDiv / bnk->FrameRate;

            if (start < end || (start - end <= bnk->twoframestime)) { bnk->skippedlastblit = FALSE; }
            else {
                if (!(flags & BINKNOSKIP) && !(bnk->OpenFlags & BINKNOSKIP)) {
                    bnk->skippedlastblit = TRUE;
                    bnk->skippedblits = bnk->skippedblits + 1;

                    if (bnk->preloadptr == NULL
                        && ((bnk->bio.CurBufUsed + 1) * 100) / (bnk->bio.CurBufSize + 1) < 10 // TODO
                        && !bnk->Paused) {
                        bnk->bio.Working = FALSE;

                        if (((bnk->bio.CurBufUsed + 1) * 100) / (bnk->bio.CurBufSize + 1) < 0x28) { // TODO
                            do {
                                if (bnk->trackindex != BINKNOSOUND) { BinkSoundFrameFill(bnk); }
                            } while (bnk->bio.Idle(&bnk->bio)
                                && (((bnk->bio.CurBufUsed + 1) * 100) / (bnk->bio.CurBufSize + 1) < 0x28)); // TODO
                        }

                        if (bnk->bsnd.SoundDroppedOut) {
                            bnk->bsnd.SoundDroppedOut = FALSE;

                            if (1 < bnk->FrameNum && bnk->FrameNum <= bnk->sndendframe) {
                                BinkSetSoundOnOff(bnk, FALSE);
                                bnk->sndreenter = bnk->sndreenter + 1;

                                if (((bnk->bio.CurBufUsed + 1) * 100) / (bnk->bio.CurBufSize + 1) < 0x5a) { // TODO
                                    do {
                                        if (!bnk->bio.Idle(&bnk->bio)) { break; }
                                    } while (((bnk->bio.CurBufUsed + 1) * 100) / (bnk->bio.CurBufSize + 1) < 0x5a); // TODO
                                }

                                while (bnk->sndreenter != 1) { Sleep(1); }

                                bnk->soundskips = bnk->soundskips + 1;

                                BinkSoundFrameClear(bnk);

                                bnk->startsynctime = 0;
                                bnk->sndreenter = bnk->sndreenter - 1;

                                BinkSetSoundOnOff(bnk, TRUE);
                            }
                        }

                        bnk->bio.Working = TRUE;
                    }

                    bnk->bio.Working = FALSE;

                    return TRUE;
                }

                result = TRUE;
            }
        }
    }

    if (!(bnk->OpenFlags & BINKNOTHREADEDIO)
        && bnk->preloadptr == NULL && !bnk->bio.DoingARead
        && (((bnk->bio.CurBufUsed + 1) * 100) / (bnk->bio.CurBufSize + 1) < 0x4b)) { // TODO
        Sleep(1);
    }

    bnk->lastblitflags = lastblitflags;

    /*
    TODO
    if (!(flags & BINKCOPYALL)) {
        pbVar4 = (byte*)bnk->MaskPlane;
        uVar5 = 0xffffffff;
        pbVar9 = pbVar4;
        do {
            if (uVar5 == 0) break;
            uVar5 = uVar5 - 1;
            bVar2 = *pbVar9;
            pbVar9 = pbVar9 + 1;
        } while (bVar2 != 0);

        if (bnk->MaskLength <= ~uVar5 - 1) goto switchD_100093e4_switchD; // TODO

        switch (flags & BINKSURFACEMASK) {
        case BINKSURFACE24: {
            YUV_blit_24bpp_mask(dest, destx, desty, destpitch, bnk->MaskPlane, bnk->MaskPitch,
                bnk->YPlane[bnk->PlaneNum], bnk->decompwidth, bnk->decompheight,
                bnk->YWidth, bnk->YHeight, lastblitflags);
            break;
        }
        case BINKSURFACE24R: {
            YUV_blit_24rbpp_mask(dest, destx, desty, destpitch, bnk->MaskPlane, bnk->MaskPitch,
                bnk->YPlane[bnk->PlaneNum], bnk->decompwidth, bnk->decompheight,
                bnk->YWidth, bnk->YHeight, lastblitflags);
            break;
        }
        case BINKSURFACE32: {
            YUV_blit_32bpp_mask(dest, destx, desty, destpitch, bnk->MaskPlane, bnk->MaskPitch,
                bnk->YPlane[bnk->PlaneNum], bnk->decompwidth, bnk->decompheight,
                bnk->YWidth, bnk->YHeight, lastblitflags);
            break;
        }
        case BINKSURFACE32R: {
            YUV_blit_32rbpp_mask(dest, destx, desty, destpitch, bnk->MaskPlane, bnk->MaskPitch,
                bnk->YPlane[bnk->PlaneNum], bnk->decompwidth, bnk->decompheight,
                bnk->YWidth, bnk->YHeight, lastblitflags);
            break;
        }
        case BINKSURFACE32A: {
            if (bnk->APlane[0] != NULL) { // TODO
                YUV_blit_32abpp_mask(dest, destx, desty, destpitch, bnk->MaskPlane, bnk->MaskPitch,
                    bnk->YPlane[bnk->PlaneNum], bnk->decompwidth, bnk->decompheight,
                    bnk->YWidth, bnk->YHeight, bnk->APlane[bnk->PlaneNum], lastblitflags);
                break;
            }

            YUV_blit_32bpp_mask(dest, destx, desty, destpitch, bnk->MaskPlane, bnk->MaskPitch,
                bnk->YPlane[bnk->PlaneNum], bnk->decompwidth, bnk->decompheight,
                bnk->YWidth, bnk->YHeight, lastblitflags);
            break;
        }
        case BINKSURFACE32RA: {
            if (bnk->APlane[0] != NULL) { // TODO
                YUV_blit_32rabpp_mask(dest, destx, desty, destpitch, bnk->MaskPlane, bnk->MaskPitch,
                    bnk->YPlane[bnk->PlaneNum], bnk->decompwidth, bnk->decompheight,
                    bnk->YWidth, bnk->YHeight, bnk->APlane[bnk->PlaneNum], lastblitflags);
                break;
            }

            YUV_blit_32rbpp_mask(dest, destx, desty, destpitch, bnk->MaskPlane, bnk->MaskPitch,
                bnk->YPlane[bnk->PlaneNum], bnk->decompwidth, bnk->decompheight,
                bnk->YWidth, bnk->YHeight, lastblitflags);
            break;
        }
        case BINKSURFACE4444: {
            if (bnk->APlane[0] != NULL) { // TODO
                YUV_blit_16a4bpp_mask(dest, destx, desty, destpitch, bnk->MaskPlane, bnk->MaskPitch,
                    bnk->YPlane[bnk->PlaneNum], bnk->decompwidth, bnk->decompheight,
                    bnk->YWidth, bnk->YHeight, bnk->APlane[bnk->PlaneNum], lastblitflags);
                break;
            }

            YUV_blit_16bpp_mask(dest, destx, desty, destpitch, bnk->MaskPlane, bnk->MaskPitch,
                bnk->YPlane[bnk->PlaneNum], bnk->decompwidth, bnk->decompheight,
                bnk->YWidth, bnk->YHeight, lastblitflags);
            break;
        }
        case BINKSURFACE555:
        case BINKSURFACE565:
        case BINKSURFACE655:
        case BINKSURFACE664: {
            YUV_blit_16bpp_mask(dest, destx, desty, destpitch, bnk->MaskPlane, bnk->MaskPitch,
                bnk->YPlane[bnk->PlaneNum], bnk->decompwidth, bnk->decompheight,
                bnk->YWidth, bnk->YHeight, lastblitflags);
            break;
        }
        case BINKSURFACE5551: {
            if (bnk->APlane[0] != NULL) {
                YUV_blit_16a1bpp_mask(dest, destx, desty, destpitch, bnk->MaskPlane, bnk->MaskPitch,
                    bnk->YPlane[bnk->PlaneNum], bnk->decompwidth, bnk->decompheight,
                    bnk->YWidth, bnk->YHeight, bnk->APlane[bnk->PlaneNum], lastblitflags);
                break;
            }

            YUV_blit_16bpp_mask(dest, destx, desty, destpitch, bnk->MaskPlane, bnk->MaskPitch,
                bnk->YPlane[bnk->PlaneNum], bnk->decompwidth, bnk->decompheight,
                bnk->YWidth, bnk->YHeight, lastblitflags);
            break;
        }
        case BINKSURFACEYUY2: {
            YUV_blit_YUY2_mask(dest, destx, desty, destpitch, bnk->MaskPlane, bnk->MaskPitch,
                bnk->YPlane[bnk->PlaneNum], bnk->decompwidth, bnk->decompheight,
                bnk->YWidth, bnk->YHeight, lastblitflags);
            break;
        }
        case BINKSURFACEUYVY: {
            YUV_blit_UYVY_mask(dest, destx, desty, destpitch, bnk->MaskPlane, bnk->MaskPitch,
                bnk->YPlane[bnk->PlaneNum], bnk->decompwidth, bnk->decompheight,
                bnk->YWidth, bnk->YHeight, lastblitflags);
            break;
        }
        case BINKSURFACEYV12: {
            YUV_blit_YV12(dest, destx, desty, destpitch, destheight,
                bnk->YPlane[bnk->PlaneNum], 0, 0, bnk->decompwidth, bnk->decompheight,
                bnk->YWidth, bnk->YHeight, lastblitflags);
        }
        }

        if (bnk->trackindex != BINKNOSOUND) { BinkSoundFrameFill(bnk); }

        bnk->bio.Working = FALSE;

        return result;
    }

switchD_100093e4_switchD:// TODO

    switch (flags & BINKSURFACEMASK) {
    case BINKSURFACE24: {
        YUV_blit_24bpp(dest, destx, desty, destpitch, bnk->YPlane[bnk->PlaneNum], 0, 0,
            bnk->decompwidth, bnk->decompheight, bnk->YWidth, bnk->YHeight, lastblitflags);
        break;
    }
    case BINKSURFACE24R: {
        YUV_blit_24rbpp(dest, destx, desty, destpitch, bnk->YPlane[bnk->PlaneNum], 0, 0,
            bnk->decompwidth, bnk->decompheight, bnk->YWidth, bnk->YHeight, lastblitflags);
        break;
    }
    case BINKSURFACE32: {
        YUV_blit_32bpp(dest, destx, desty, destpitch, bnk->YPlane[bnk->PlaneNum], 0, 0,
            bnk->decompwidth, bnk->decompheight, bnk->YWidth, bnk->YHeight, lastblitflags);
        break;
    }
    case BINKSURFACE32R: {
        YUV_blit_32rbpp(dest, destx, desty, destpitch, bnk->YPlane[bnk->PlaneNum], 0, 0,
            bnk->decompwidth, bnk->decompheight, bnk->YWidth, bnk->YHeight, lastblitflags);
        break;
    }
    case BINKSURFACE32A: {
        if (bnk->APlane[0] != NULL) { // TODO
            YUV_blit_32abpp(dest, destx, desty, destpitch, bnk->YPlane[bnk->PlaneNum], 0, 0,
                bnk->decompwidth, bnk->decompheight, bnk->YWidth, bnk->YHeight,
                bnk->APlane[bnk->PlaneNum], lastblitflags);
            break;
        }

        YUV_blit_32bpp(dest, destx, desty, destpitch, bnk->YPlane[bnk->PlaneNum], 0, 0,
            bnk->decompwidth, bnk->decompheight, bnk->YWidth, bnk->YHeight, lastblitflags);
        break;
    }
    case BINKSURFACE32RA: {
        if (bnk->APlane[0] != NULL) { // TODO
            YUV_blit_32rabpp(dest, destx, desty, destpitch, bnk->YPlane[bnk->PlaneNum], 0, 0,
                bnk->decompwidth, bnk->decompheight, bnk->YWidth, bnk->YHeight,
                (int)bnk->APlane[bnk->PlaneNum], lastblitflags);
            break;
        }

        YUV_blit_32rbpp(dest, destx, desty, destpitch, bnk->YPlane[bnk->PlaneNum], 0, 0,
            bnk->decompwidth, bnk->decompheight, bnk->YWidth, bnk->YHeight, lastblitflags);
        break;
    }
    case BINKSURFACE4444: {
        if (bnk->APlane[0] != NULL) { // TODO
            YUV_blit_16a4bpp(dest, destx, desty, destpitch, bnk->YPlane[bnk->PlaneNum], 0, 0,
                bnk->decompwidth, bnk->decompheight, bnk->YWidth, bnk->YHeight,
                bnk->APlane[bnk->PlaneNum], lastblitflags);
            break;
        }

        YUV_blit_16bpp(dest, destx, desty, destpitch, bnk->YPlane[bnk->PlaneNum], 0, 0,
            bnk->decompwidth, bnk->decompheight, bnk->YWidth, bnk->YHeight, lastblitflags);
        break;
    }
    case BINKSURFACE555:
    case BINKSURFACE565:
    case BINKSURFACE655:
    case BINKSURFACE664: {
        YUV_blit_16bpp(dest, destx, desty, destpitch, bnk->YPlane[bnk->PlaneNum], 0, 0,
            bnk->decompwidth, bnk->decompheight, bnk->YWidth, bnk->YHeight, lastblitflags);
        break;
    }
    case BINKSURFACE5551: {
        if (bnk->APlane[0] != NULL) { // TODO
            YUV_blit_16a1bpp(dest, destx, desty, destpitch, bnk->YPlane[bnk->PlaneNum], 0, 0,
                bnk->decompwidth, bnk->decompheight, bnk->YWidth, bnk->YHeight,
                bnk->APlane[bnk->PlaneNum], lastblitflags);
            break;
        }

        YUV_blit_16bpp(dest, destx, desty, destpitch, bnk->YPlane[bnk->PlaneNum], 0, 0,
            bnk->decompwidth, bnk->decompheight, bnk->YWidth, bnk->YHeight, lastblitflags);
        break;
    }
    case BINKSURFACEYUY2: {
        YUV_blit_YUY2(dest, destx, desty, destpitch, bnk->YPlane[bnk->PlaneNum], 0, 0,
            bnk->decompwidth, bnk->decompheight, bnk->YWidth, bnk->YHeight, lastblitflags);
        break;
    }
    case BINKSURFACEUYVY: {
        YUV_blit_UYVY(dest, destx, desty, destpitch, bnk->YPlane[bnk->PlaneNum], 0, 0,
            bnk->decompwidth, bnk->decompheight, bnk->YWidth, bnk->YHeight, lastblitflags);
        break;
    }
    case BINKSURFACEYV12: {
        YUV_blit_YV12(dest, destx, desty, destpitch, destheight, bnk->YPlane[bnk->PlaneNum], 0, 0,
            bnk->decompwidth, bnk->decompheight, bnk->YWidth, bnk->YHeight, lastblitflags);
        break;
    }
    }
    */
    if (bnk->trackindex != BINKNOSOUND) { BinkSoundFrameFill(bnk); }

    bnk->bio.Working = FALSE;

    return result;
}

// 0x10009bc0
s32 RADEXPLINK BinkDoFrame(HBINK bnk)
{
    if (bnk != NULL && bnk->lastdecompframe != bnk->FrameNum) {
        bnk->bio.Working = TRUE;
        bnk->startframetime = RADTimerRead();

        // TODO
        //FUN_1002bc90(bnk->rtframetimes + 1, bnk->rtframetimes, bnk->runtimemoveamt);
        //FUN_1002bc90(bnk->rtvdecomptimes + 1, bnk->rtvdecomptimes, bnk->runtimemoveamt);
        //FUN_1002bc90(bnk->rtadecomptimes + 1, bnk->rtadecomptimes, bnk->runtimemoveamt);
        //FUN_1002bc90(bnk->rtblittimes + 1, bnk->rtblittimes, bnk->runtimemoveamt);
        //FUN_1002bc90(bnk->rtreadtimes + 1, bnk->rtreadtimes, bnk->runtimemoveamt);
        //FUN_1002bc90(bnk->rtidlereadtimes + 1, bnk->rtidlereadtimes, bnk->runtimemoveamt);
        //FUN_1002bc90(bnk->rtthreadreadtimes + 1, bnk->rtthreadreadtimes, bnk->runtimemoveamt);

        bnk->rtframetimes[0] = bnk->startframetime;
        bnk->rtvdecomptimes[0] = bnk->timevdecomp;
        bnk->rtadecomptimes[0] = bnk->timeadecomp;
        bnk->rtblittimes[0] = bnk->timeblit;
        bnk->rtreadtimes[0] = bnk->bio.ForegroundTime;
        bnk->rtidlereadtimes[0] = bnk->bio.IdleTime;
        bnk->rtthreadreadtimes[0] = bnk->bio.ThreadTime;

        if (bnk->firstframetime == 0) {
            // TODO local_8 = 0x3e8;
            bnk->rtframetimes[1] = bnk->startframetime - (bnk->fileframeratediv * 1000 / bnk->fileframerate);
            bnk->firstframetime = bnk->startframetime;
            bnk->bio.ThreadTime = 0;
            bnk->bio.IdleTime = 0;
        }
        else if (bnk->trackindex != BINKNOSOUND) { BinkSoundFrameFill(bnk); }

        // TODO puVar8 = bnk->compframe;
        bnk->entireframetime = bnk->startframetime;

        if (bnk->bio.ReadError) { bnk->ReadError = TRUE; }
        if (bnk->ReadError) { return TRUE; }

        for (s32 x = 0; x < bnk->NumTracks; x++) {
            // TODO
            /*uVar2 = *puVar8;
            if (bink->trackindex == x && (uVar2 != 0)) {
                pBVar9 = (BINK*)puVar8[1];
                local_8 = puVar8 + 2;

                bink->sndreenter = bink->sndreenter + 1;

                while (bink->sndreenter != 1) { Sleep(1); }

                while ((pBVar9 != (BINK*)0x0 &&
                    (pBVar5 = (BINK*)(bink->sndbufsize - bink->sndamt), pBVar5 != (BINK*)0x0))) {
                    FUN_100289b0((int*)bink->sndcomp, (int*)&local_c, (int*)&bnk, (int)local_8,
                        (int*)&local_8);
                    if (pBVar9 < bnk) {
                        bnk = pBVar9;
                    }
                    pBVar9 = (BINK*)((int)pBVar9 - (int)bnk);
                    if (pBVar5 < bnk) {
                        bnk = pBVar5;
                    }
                    bink->sndamt = (int)bnk->FrameRects + (bink->sndamt - 0x34);
                    puVar7 = bink->sndwritepos;
                    pBVar5 = (BINK*)(bink->sndend + -(int)puVar7);
                    if (pBVar5 < bnk) {
                        if (pBVar5 != (BINK*)0x0) {
                            puVar6 = local_c;
                            for (uVar4 = (uint)pBVar5 >> 2; uVar4 != 0; uVar4 = uVar4 - 1) {
                                *(undefined4*)puVar7 = *(undefined4*)puVar6;
                                puVar6 = puVar6 + 4;
                                puVar7 = puVar7 + 4;
                            }
                            for (uVar4 = (uint)pBVar5 & 3; uVar4 != 0; uVar4 = uVar4 - 1) {
                                *puVar7 = *puVar6;
                                puVar6 = puVar6 + 1;
                                puVar7 = puVar7 + 1;
                            }
                            bnk = (BINK*)((int)bnk - (int)pBVar5);
                            local_c = local_c + (int)pBVar5;
                        }
                        puVar7 = local_c;
                        puVar6 = bink->sndbuf;
                        for (uVar4 = (uint)bnk >> 2; uVar4 != 0; uVar4 = uVar4 - 1) {
                            *(undefined4*)puVar6 = *(undefined4*)puVar7;
                            puVar7 = puVar7 + 4;
                            puVar6 = puVar6 + 4;
                        }
                        for (uVar4 = (uint)bnk & 3; uVar4 != 0; uVar4 = uVar4 - 1) {
                            *puVar6 = *puVar7;
                            puVar7 = puVar7 + 1;
                            puVar6 = puVar6 + 1;
                        }
                        pBVar3 = (BINK*)bink->sndbuf;
                        pBVar5 = bnk;
                    }
                    else {
                        puVar6 = local_c;
                        for (uVar4 = (uint)bnk >> 2; uVar4 != 0; uVar4 = uVar4 - 1) {
                            *(undefined4*)puVar7 = *(undefined4*)puVar6;
                            puVar6 = puVar6 + 4;
                            puVar7 = puVar7 + 4;
                        }
                        for (uVar4 = (uint)bnk & 3; uVar4 != 0; uVar4 = uVar4 - 1) {
                            *puVar7 = *puVar6;
                            puVar6 = puVar6 + 1;
                            puVar7 = puVar7 + 1;
                        }
                        pBVar5 = (BINK*)bink->sndwritepos;
                        pBVar3 = bnk;
                    }
                    bink->sndwritepos =
                        (uchar*)((int)&pBVar3->FrameRects[0].Width + (int)(pBVar5->FrameRects + -7));
                }

                bink->sndreenter = bink->sndreenter - 1;
            }
            puVar8 = (uint*)((int)(puVar8 + 1) + uVar2);
            */
        }

        const u32 audio = RADTimerRead();

        if (bnk->videoon) {
            if (!bnk->skippedlastblit) {
                radmemset(bnk->MaskPlane, 0x00, bnk->YHeight * bnk->YWidth / 16);
            }

            if (!(bnk->OpenFlags & BINKNOTHREADEDIO) && bnk->preloadptr == NULL
                && !bnk->bio.DoingARead
                && (((bnk->bio.CurBufUsed + 1) * 100) / (bnk->bio.CurBufSize + 1) < 0x4b)) { // TODO
                Sleep(1);
            }

            // TODO
            //ExpandBink(bnk->YPlane[bnk->PlaneNum ^ 1], bnk->YPlane[bnk->PlaneNum],
            //    bnk->APlane[bnk->PlaneNum ^ 1], bnk->APlane[bnk->PlaneNum],
            //    bnk->MaskPlane, bnk->decompwidth, bnk->decompheight, bnk->YWidth,
            //    bnk->YHeight, puVar8, IS_KEY_FRAME(bnk->frameoffsets[bnk->FrameNum - 1]),
            //    &bnk->bunp.typeptr, bnk->OpenFlags, bnk->BinkType);

            bnk->PlaneNum = bnk->PlaneNum ^ 1;
        }

        if (bnk->trackindex != BINKNOSOUND) { BinkSoundFrameFill(bnk); }

        bnk->NumRects = -1;
        bnk->playedframes = bnk->playedframes + 1;

        BinkSetStartSyncTime(bnk);

        bnk->timeadecomp = audio + (bnk->timeadecomp - bnk->startframetime);
        bnk->startblittime = RADTimerRead();
        bnk->timevdecomp = bnk->timevdecomp + (bnk->startblittime - audio);
        bnk->lastdecompframe = bnk->FrameNum;
        bnk->LastFrameNum = bnk->FrameNum;
        bnk->bio.Working = FALSE;
    }

    return FALSE;
}

// 0x1000a0c0
void RADEXPLINK BinkNextFrame(HBINK bnk)
{
    // TODO NOT IMPLEMENTED
}

// 0x1000a140
u32 RADEXPLINK BinkGetKeyFrame(HBINK bnk, u32 frame, s32 flags)
{
    if (bnk == NULL) { return 0; }

    if ((flags & BINKGETKEYNOTEQUAL) || !IS_KEY_FRAME(bnk->frameoffsets[frame - 1])) {
        if (flags & (BINKGETKEYCLOSEST | BINKGETKEYNEXT)) {
            if ((flags & BINKGETKEYNEXT)) {

                for (u32 x = frame; x < bnk->Frames; x++) {
                    if (IS_KEY_FRAME(bnk->frameoffsets[x])) { return x; }
                }
            }
            else if (flags & BINKGETKEYCLOSEST) {
                u32 start = frame >= 2;
                u32 end = frame < bnk->Frames;

                while (start != 0 || end != 0) {
                    if (start != 0) {
                        if (IS_KEY_FRAME(bnk->frameoffsets[start])) { return start; }

                        start = start - 1;
                    }

                    if (end != 0) {
                        if (IS_KEY_FRAME(bnk->frameoffsets[end])) { return end; }

                        end = end + 1 < bnk->Frames;
                    }
                }
            }

            return 0;
        }

        if (frame <= 2) { return 0; }

        for (u32 x = frame - 2; x > 0; x--) {
            if (IS_KEY_FRAME(bnk->frameoffsets[x])) { return x + 1; }
        }

        return frame - 1;
    }

    return frame;
}

// 0x1000a220
void RADEXPLINK BinkGoto(HBINK bnk, u32 frame, s32 flags)
{
    if (bnk != NULL) {
        bnk->bio.Working = TRUE;

        if (bnk->Frames < frame) { frame = bnk->Frames; }

        if (!(flags & BINKGOTOQUICK)) {
            if (bnk->FrameNum != frame) {
                const u32 key = BinkGetKeyFrame(bnk, frame, BINKGETKEYPREVIOUS);

                if ((bnk->FrameNum <= frame && key <= bnk->FrameNum) ||
                    (BinkFrameRead(bnk, key), key != frame)) {
                    BinkPause(bnk, TRUE);

                    if (bnk->FrameNum != bnk->lastdecompframe) { BinkDoFrame(bnk); }

                    BinkNextFrame(bnk);

                    while (bnk->FrameNum != frame) {
                        BinkDoFrame(bnk);
                        BinkNextFrame(bnk);
                    }

                    BinkPause(bnk, FALSE);
                }
            }
        }
        else { BinkFrameRead(bnk, frame); }

        bnk->bio.Working = FALSE;
    }
}

// 0x1000a2e0
void RADEXPLINK BinkClose(HBINK bnk)
{
    if (bnk == NULL) { return; }

    BinkPause(bnk, TRUE);

    if (bnk->threadcontrol == THREAD_CONTROL_ACTIVE) {
        bnk->threadcontrol = THREAD_CONTROL_NONE;

        while (bnk->threadcontrol == THREAD_CONTROL_NONE) { Sleep(1); }
    }

    if (bnk->BackgroundThread != NULL) { BinkThreadClose((HBINKTHREAD)bnk->BackgroundThread); }

    if (bnk->trackindex != BINKNOSOUND) {
        bnk->bsnd.Close(&bnk->bsnd);
        BinkSoundCompressorRelease(bnk->sndcomp);
    }

    if (bnk->preloadptr == NULL) {
        bnk->bio.Close(&bnk->bio);
        radfree(bnk->ioptr);
    }
    else if (!(bnk->OpenFlags & BINKFROMMEMORY)) { radfree(bnk->preloadptr); }

    if (bnk->sndbuf != NULL) { radfree(bnk->sndbuf); }
    if (bnk->YPlane[0] != NULL) { radfree(bnk->YPlane[0]); }

    radmemset(bnk, 0x00, sizeof(BINK));

    radfree(bnk);
}

// 0x1000a3c0
s32 RADEXPLINK BinkWait(HBINK bnk)
{
    if (bnk == NULL) { return FALSE; }

    if ((bnk->playedframes == 0 && !bnk->Paused) || bnk->ReadError) { return FALSE; }

    if (bnk->startsynctime == 0) { BinkSetStartSyncTime(bnk); }
    if (bnk->trackindex != BINKNOSOUND) { BinkSoundFrameFill(bnk); }

    const u32 time = RADTimerRead();
    BinkSumTimeBlit(bnk, time);
    BinkSumFrameTime(bnk, time);

    if (!bnk->Paused && (bnk->trackindex == BINKNOSOUND || bnk->soundon)) {
        if (bnk->FrameRate == 0) { return FALSE; }

        const u32 diff = ((bnk->playedframes - bnk->startsyncframe) * 1000 * bnk->FrameRateDiv) / bnk->FrameRate;

        if (diff <= time - bnk->startsynctime) {
            if (bnk->twoframestime < ((time - bnk->startsynctime) - diff) && bnk->trackindex == BINKNOSOUND) {
                bnk->startsynctime = time;
                bnk->startsyncframe = bnk->playedframes - 1;
            }

            return FALSE;
        }
    }

    if (bnk->preloadptr == NULL) {
        if (bnk->bio.Idle(&bnk->bio) && !(bnk->OpenFlags & BINKNOTHREADEDIO)) { Sleep(1); }
    }

    return TRUE;
}

// 0x1000a5a0
s32 RADEXPLINK BinkPause(HBINK bnk, s32 pause)
{
    if (bnk == NULL) { return FALSE; }

    const u32 time = RADTimerRead();
    BinkSumTimeBlit(bnk, time);
    BinkSumFrameTime(bnk, time);

    if (!pause && bnk->Paused) { bnk->startsynctime = 0; }

    bnk->Paused = pause;

    if (bnk->trackindex != BINKNOSOUND) { bnk->bsnd.Pause(&bnk->bsnd, pause); }

    bnk->skippedlastblit = FALSE;

    if (bnk->playedframes != 0 && bnk->trackindex != BINKNOSOUND) { BinkSoundFrameFill(bnk); }

    return bnk->Paused;
}

// 0x1000a630
void RADEXPLINK BinkGetSummary(HBINK bnk, BINKSUMMARY PTR4* sum)
{
    if (bnk == NULL || sum == NULL) { return; }

    const u32 time = RADTimerRead();

    BinkSumTimeBlit(bnk, time);
    BinkSumFrameTime(bnk, time);

    radmemset(sum, 0x00, sizeof(BINKSUMMARY));

    sum->FrameRate = bnk->FrameRate;
    sum->FrameRateDiv = bnk->FrameRateDiv;
    sum->SkippedBlits = bnk->skippedblits;
    sum->SoundSkips = bnk->soundskips;
    sum->FileFrameRate = bnk->fileframerate;
    sum->FileFrameRateDiv = bnk->fileframeratediv;
    sum->TotalFrames = bnk->Frames;
    sum->TotalPlayedFrames = bnk->playedframes;

    sum->TotalTime = RADTimerRead() - bnk->firstframetime;
    sum->TotalOpenTime = bnk->timeopen;
    sum->TotalAudioDecompTime = bnk->timeadecomp;
    sum->TotalVideoDecompTime = bnk->timevdecomp;
    sum->TotalBlitTime = bnk->timeblit;
    sum->HighestMemAmount = sum->HighestMemAmount + bnk->totalmem;
    sum->TotalIOMemory = bnk->iosize;
    sum->TotalReadSpeed = bnk->bio.BytesRead * 1000 / (bnk->bio.TotalTime + 1);
    sum->TotalReadTime = bnk->bio.ForegroundTime;
    sum->TotalIdleReadTime = bnk->bio.IdleTime;
    sum->TotalBackReadTime = bnk->bio.ThreadTime;

    const u32 offset = bnk->Size - ALIGN_FRAME_OFFSETS(bnk->frameoffsets[0]);
    sum->AverageDataRate = offset * bnk->fileframerate / (bnk->fileframeratediv * bnk->Frames);
    sum->AverageFrameSize = offset / bnk->Frames;

    sum->Highest1SecRate = bnk->Highest1SecRate;
    sum->Highest1SecFrame = bnk->Highest1SecFrame + 1;
    sum->Width = bnk->Width;
    sum->Height = bnk->Height;
    sum->SlowestFrameTime = bnk->slowestframetime;
    sum->Slowest2FrameTime = bnk->slowest2frametime;
    sum->SlowestFrameNum = bnk->slowestframe;
    sum->Slowest2FrameNum = bnk->slowest2frame;
    sum->TotalIOMemory = bnk->bio.BufSize;
    sum->HighestIOUsed = bnk->bio.BufHighUsed;
}

// 0x1000a7e0
void RADEXPLINK BinkGetRealtime(HBINK bink, BINKREALTIME PTR4* run, u32 frames)
{
    const u32 time = RADTimerRead();

    BinkSumTimeBlit(bink, time);
    BinkSumFrameTime(bink, time);

    if (frames == 0 || bink->runtimeframes <= frames) { frames = bink->runtimeframes - 1; }

    if (bink->FrameNum < frames) {
        frames = bink->FrameNum - 1;

        if (frames == 0) { frames = 1; }
    }

    run->FrameNum = bink->LastFrameNum;
    run->FrameRate = bink->FrameRate;
    run->FrameRateDiv = bink->FrameRate;
    run->ReadBufferSize = bink->bio.CurBufSize;
    run->ReadBufferUsed = bink->bio.CurBufUsed;
    run->FramesDataRate = ((bink->frameoffsets[bink->FrameNum] - bink->frameoffsets[bink->FrameNum - frames]) * bink->fileframerate) / (bink->fileframeratediv * frames);
    run->Frames = frames;
    run->FramesTime = bink->rtframetimes[0] - bink->rtframetimes[frames];

    if (run->FramesTime == 0) { run->FramesTime = 1; }

    run->FramesVideoDecompTime = bink->rtvdecomptimes[0] - bink->rtvdecomptimes[frames];
    run->FramesAudioDecompTime = bink->rtadecomptimes[0] - bink->rtadecomptimes[frames];
    run->FramesBlitTime = bink->rtblittimes[0] - bink->rtblittimes[frames];
    run->FramesReadTime = bink->rtreadtimes[0] - bink->rtreadtimes[frames];
    run->FramesIdleReadTime = bink->rtidlereadtimes[0] - bink->rtidlereadtimes[frames];
    run->FramesThreadReadTime = bink->rtthreadreadtimes[0] - bink->rtthreadreadtimes[frames];
}

// 0x1000a910
s32 RADEXPLINK BinkGetRects(HBINK bnk, u32 flags)
{
    // TODO NOT IMPLEMENTED

    return 0;
}

// 0x1000afa0
void RADEXPLINK BinkService(HBINK bink)
{
    if (bink->trackindex != BINKNOSOUND) { BinkSoundFrameFill(bink); }
}

// 0x1000afc0
void RADEXPLINK BinkSetVolume(HBINK bnk, s32 volume)
{
    if (bnk != NULL && bnk->trackindex != BINKNOSOUND && bnk->bsnd.Volume != NULL) {
        bnk->bsnd.Volume(&bnk->bsnd, volume);
    }
}

// 0x1000aff0
void RADEXPLINK BinkSetPan(HBINK bnk, s32 pan)
{
    if (bnk != NULL && bnk->trackindex != BINKNOSOUND && bnk->bsnd.Pan != NULL) {
        bnk->bsnd.Pan(&bnk->bsnd, pan);
    }
}

// 0x1000b020
void PTR4* RADEXPLINK BinkLogoAddress(void)
{
    return BinkVideo;
}

// 0x1000b030
u32 RADEXPLINK BinkGetTrackType(HBINK bnk, u32 trackindex)
{
    if (bnk != NULL) { return bnk->tracktypes[trackindex]; }

    return 0;
}

// 0x1000b050
u32 RADEXPLINK BinkGetTrackMaxSize(HBINK bnk, u32 trackindex)
{
    if (bnk != NULL) { return bnk->tracksizes[trackindex]; }

    return 0;
}

// 0x1000b070
u32 RADEXPLINK BinkGetTrackID(HBINK bnk, u32 trackindex)
{
    if (bnk != NULL) { return bnk->trackIDs[trackindex]; }

    return 0;
}

// 0x1000b090
HBINKTRACK RADEXPLINK BinkOpenTrack(HBINK bnk, u32 trackindex)
{
    if (bnk != NULL && (s32)trackindex < bnk->NumTracks
        && SOUND_TRACK_TYPE_IS_ACTIVE(bnk->tracktypes[trackindex])) {
        HBINKSNDCOMP comp = BinkSoundCompressionInitialize(SOUND_TRACK_TYPE_FREQUENCY(bnk->tracktypes[trackindex]),
            SOUND_TRACK_TYPE_CHANNELS(bnk->tracktypes[trackindex]), 1); // TODO

        if (comp != NULL) {
            HBINKTRACK track = (HBINKTRACK)radmalloc(sizeof(BINKTRACK));

            if (track != NULL) {
                radmemset(track, 0x00, sizeof(BINKTRACK));

                track->bink = bnk;
                track->sndcomp = comp;
                track->Frequency = SOUND_TRACK_TYPE_FREQUENCY(bnk->tracktypes[trackindex]);
                track->Bits = SOUND_TRACK_TYPE_BITS(bnk->tracktypes[trackindex]);
                track->Channels = SOUND_TRACK_TYPE_CHANNELS(bnk->tracktypes[trackindex]);

                track->MaxSize = bnk->tracksizes[trackindex] + 3 & 0xfffffffc;

                if (track->Bits == 8) { track->MaxSize = track->MaxSize / 2; }

                track->trackindex = trackindex;

                return track;
            }

            BinkSoundCompressorRelease(comp);
        }
    }

    return NULL;
}

// 0x1000b170
void RADEXPLINK BinkCloseTrack(HBINKTRACK bnkt)
{
    if (bnkt != NULL) {
        if (bnkt->sndcomp != NULL) {
            BinkSoundCompressorRelease(bnkt->sndcomp);
            bnkt->sndcomp = NULL;
        }
        
        radfree(bnkt);
    }
}

// 0x1000b1a0
u32 RADEXPLINK BinkGetTrackData(HBINKTRACK bnkt, void PTR4* dest)
{
    // TODO NOT IMPLEMENTED

    return 0;
}

// 0x1000b2c0
s32 RADEXPLINK BinkSetVideoOnOff(HBINK bnk, s32 onoff)
{
    if (bnk != NULL) { bnk->videoon = onoff; }

    return onoff;
}

// 0x1000b2e0
s32 RADEXPLINK BinkSetSoundOnOff(HBINK bnk, s32 onoff)
{
    s32 result = FALSE;

    if (bnk != NULL && bnk->trackindex != BINKNOSOUND && bnk->bsnd.SetOnOff != NULL) {
        result = bnk->bsnd.SetOnOff(&bnk->bsnd, onoff);

        if (!result && bnk->soundon) {
            bnk->sndreenter = bnk->sndreenter + 1;

            while (bnk->sndreenter != 1) { Sleep(1); }

            bnk->sndwritepos = bnk->sndbuf;
            bnk->sndreadpos = bnk->sndbuf;

            bnk->sndamt = 0;
            bnk->soundon = FALSE;

            bnk->sndreenter = bnk->sndreenter - 1;

            return FALSE;
        }

        if (result && !bnk->soundon) {
            bnk->sndreenter = bnk->sndreenter + 1;

            while (bnk->sndreenter != 1) { Sleep(1); }

            if (bnk->FrameNum != 1 && bnk->FrameNum != bnk->InternalFrames) {
                bnk->sndamt = bnk->sndprime;

                radmemset(bnk->sndbuf, 0x00, bnk->sndamt);

                bnk->sndwritepos = bnk->sndbuf + bnk->sndamt;
                bnk->sndreadpos = bnk->sndbuf;
            }

            bnk->soundon = TRUE;
            bnk->startsynctime = 0;

            bnk->sndreenter = bnk->sndreenter - 1;
        }
    }

    return result;
}

// 0x1000b4e0
void RADEXPLINK YUV_init(u32 flags)
{
    // TODO NOT IMPLEMENTED
}


// 0x1000d180
void RADEXPLINK YUV_blit_32bpp(void* dest, u32 destx, u32 desty, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12)
{
    // TODO NOT IMPLEMENTED
}

// 0x1000e410
void RADEXPLINK YUV_blit_32rbpp(void* dest, u32 destx, u32 desty, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12)
{
    // TODO NOT IMPLEMENTED
}

// 0x1000fbc0
void RADEXPLINK YUV_blit_32abpp(void* dest, u32 destx, u32 desty, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12, u32 param_13)
{
    // TODO NOT IMPLEMENTED
}

// 0x1000fc20
void RADEXPLINK YUV_blit_32rabpp(void* dest, u32 destx, u32 desty, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12, u32 param_13)
{
    // TODO NOT IMPLEMENTED
}

// 0x10014ee0
void RADEXPLINK YUV_blit_YUY2(void* dest, u32 destx, u32 desty, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12)
{
    // TODO NOT IMPLEMENTED
}

// 0x100119b0
void RADEXPLINK YUV_blit_24bpp(void* dest, u32 destx, u32 desty, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12)
{
    // TODO NOT IMPLEMENTED
}

// 0x10011a00
void RADEXPLINK YUV_blit_24rbpp(void* dest, u32 destx, u32 desty, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12)
{
    // TODO NOT IMPLEMENTED
}

// 0x10012790
void RADEXPLINK YUV_blit_16bpp(void* dest, u32 destx, u32 desty, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12)
{
    // TODO NOT IMPLEMENTED
}

// 0x100137b0
void RADEXPLINK YUV_blit_16a4bpp(void* dest, u32 destx, u32 desty, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12, u32 param_13)
{
    // TODO NOT IMPLEMENTED
}

// 0x10014350
void RADEXPLINK YUV_blit_16a1bpp(void* dest, u32 destx, u32 desty, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12, u32 param_13)
{
    // TODO NOT IMPLEMENTED
}

// 0x10015b20
void RADEXPLINK YUV_blit_UYVY(void* dest, u32 destx, u32 desty, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12)
{
    // TODO NOT IMPLEMENTED
}

// 0x10015bb0
void RADEXPLINK YUV_blit_YV12(void* dest, u32 destx, u32 desty, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12, u32 param_13)
{
    // TODO NOT IMPLEMENTED
}

// 0x10015e60
void RADEXPLINK YUV_blit_32bpp_mask(void* dest, u32 destx, u32 desty, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12)
{
    // TODO NOT IMPLEMENTED
}

// 0x10016690
void RADEXPLINK YUV_blit_32rbpp_mask(void* dest, u32 destx, u32 desty, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12)
{
    // TODO NOT IMPLEMENTED
}

// 0x100166e0
void RADEXPLINK YUV_blit_32abpp_mask(void* dest, u32 destx, u32 desty, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12, u32 param_13)
{
    // TODO NOT IMPLEMENTED
}

// 0x10016740
void RADEXPLINK YUV_blit_32rabpp_mask(void* dest, u32 destx, u32 desty, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12, u32 param_13)
{
    // TODO NOT IMPLEMENTED
}

// 0x100167a0
void RADEXPLINK YUV_blit_24bpp_mask(void* dest, u32 destx, u32 desty, s32 destpitch, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12)
{
    // TODO NOT IMPLEMENTED
}

// 0x100167f0
void RADEXPLINK YUV_blit_24rbpp_mask(void* dest, u32 destx, u32 desty, s32 destpitch, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12)
{
    // TODO NOT IMPLEMENTED
}

// 0x10016840
void RADEXPLINK YUV_blit_16bpp_mask(void* dest, u32 destx, u32 desty, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12)
{
    // TODO NOT IMPLEMENTED
}

// 0x10016890
void RADEXPLINK YUV_blit_16a1bpp_mask(void* dest, u32 destx, u32 desty, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12, u32 param_13)
{
    // TODO NOT IMPLEMENTED
}

// 0x100168f0
void RADEXPLINK YUV_blit_16a4bpp_mask(void* dest, u32 destx, u32 desty, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12, u32 param_13)
{
    // TODO NOT IMPLEMENTED
}

// 0x10016950
void RADEXPLINK YUV_blit_YUY2_mask(void* dest, u32 destx, u32 desty, s32 destpitch, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12)
{
    // TODO NOT IMPLEMENTED
}

// 0x100169a0
void RADEXPLINK YUV_blit_UYVY_mask(void* dest, u32 destx, u32 desty, u32 param_4, u32 param_5, u32 param_6, u32 param_7, u32 param_8, u32 param_9, u32 param_10, u32 param_11, u32 param_12)
{
    // TODO NOT IMPLEMENTED
}

// 0x10028f90
BINKSNDOPEN RADEXPLINK BinkOpenMiles(u32 param)
{
    // TODO NOT IMPLEMENTED

    return NULL;
}

// 0x1002a940
BINKSNDOPEN RADEXPLINK BinkOpenDirectSound(void* param)
{
    if (DirectSound != DEFAULT_DIRECT_SOUND_INSTANCE) {
        if (DirectSound != param) { DirectSound = (LPDIRECTSOUND)param; }
    }
    else { DirectSound = (LPDIRECTSOUND)param; }

    if (DirectSound == NULL) {
        DirectSoundIsManaged = TRUE;

        if (DirectSoundCreateAction == NULL) {
            if (DirectSoundModule == NULL) {
                UINT uMode = SetErrorMode(SEM_NOOPENFILEERRORBOX);
                DirectSoundModule = LoadLibraryA("DSOUND.DLL");
                SetErrorMode(uMode);
            }

            if (DirectSoundModule >= MAX_INVALID_MODULE_HANDLE) {
                DirectSoundCreateAction = (DIRECTSOUNDCREATEACTION)GetProcAddress(DirectSoundModule, "DirectSoundCreate");
            }

            if (DirectSoundCreateAction == NULL) { return NULL; }
        }
    }
    else {
        DirectSoundIsManaged = FALSE;
        DirectSoundModule = NULL;
    }

    if (!BinkDirectSoundInitialize()) { return NULL; }

    BinkDirectSoundRelease();

    return BinkDirectSoundOpen;
}

// 0x1002b3e0
BINKSNDOPEN RADEXPLINK BinkOpenWaveOut(void* param)
{
    return BinkWaveOutOpen;
}

// 0x1002ba30
u32 RADEXPLINK RADTimerRead(void)
{
    if (IsTimerInitializationRequired) {
        IsTimerInitializationRequired = FALSE;

        if (!QueryPerformanceFrequency(&TimerFrequency)) {
            radmemset(&TimerFrequency, 0x00, sizeof(LARGE_INTEGER));

            return timeGetTime();
        }

        QueryPerformanceCounter(&TimerCounter);
    }

    if (TimerFrequency.LowPart != 0 || TimerFrequency.HighPart != 0) {
        LARGE_INTEGER time;
        QueryPerformanceCounter(&time);

        return (u32)((time.QuadPart - TimerCounter.QuadPart) * 1000000 / TimerFrequency.QuadPart);
    }

    return timeGetTime();
}