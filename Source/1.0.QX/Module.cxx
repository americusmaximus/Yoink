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

#include "Buffer.hxx"
#include "Cursor.hxx"
#include "IO.hxx"
#include "Mem.hxx"
#include "Module.hxx"
#include "Timer.hxx"
#include "Thread.hxx"

#define ALIGN_FRAME_OFFSETS(X) ((size_t)X & 0xFFFFFFFE) /* x64 */

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

// 0x10005db0
void RADEXPLINK BinkBufferCheckWinPos(HBINKBUFFER buf, s32 PTR4* NewWindowX, s32 PTR4* NewWindowY)
{
    if (buf != NULL) {
        if (NewWindowX != NULL) {
            s32 value = *NewWindowX + buf->ClientOffsetX;

            if (buf->noclipping) {
                if (BufferScreenWidth < (s32)buf->Width + value) {
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
                if (BufferScreenHeight < (s32)buf->Height + value) {
                    value = BufferScreenHeight - buf->Height;
                }

                if (value < 0) { value = 0; }
            }

            *NewWindowY = value - buf->ClientOffsetY;
        }
    }
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

        HDC hdc = (HDC)buf->dibdc;

        if (hdc != NULL) {
            SelectObject(hdc, (HGDIOBJ)buf->diboldbitmap);
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

// 0x10006050
HBINKBUFFER RADEXPLINK BinkBufferOpen(HWND wnd, u32 width, u32 height, u32 bufferflags)
{
    if (width == 0) { return NULL; }
    if (height == 0) { return NULL; }
    
    u32 buffertype = bufferflags & BINKBUFFERTYPEMASK;
    
    const u32 options = bufferflags & BINKBUFFERSCALES;
    const u32 fullscreen = bufferflags >> 0x17 & 1; // TODO
    
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
            // TODO
            //if (BufferScreenBits == 0) { radstrcpy(BinkError, "256 color mode not supported."); }
            //else if (BufferScreenBits > 8) { radstrcpy(BinkError, "No capable blitting style available."); }

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
                buf->wnd = (u32)wnd;
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

            // TODO
            //if (BufferError[0] == NULL) {
            //    if (BufferScreenBits == 0) { radstrcpy(BinkError, "256 color mode not supported."); }
            //    else if (BufferScreenBits > 8) { radstrcpy(BinkError, "No capable blitting style available."); }
            //}

            if (!dd) { BinkBufferDirectDrawRelease(); }
            if (dc) { BinkBufferDeviceContextRelease(); }

            return NULL;
        }
    }

    if (!dc && (BufferDirectDraw == NULL || BufferBytes == 0)
        && BinkBufferDeviceContextInitialize(wnd, fullscreen)) {
        dc = TRUE;
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
                    buf->wnd = (u32)wnd;
                    buf->dibbuffer = buffer;
                    buf->diboldbitmap = (u32)gdi;
                    buf->dibinfo = bmp;
                    buf->dibdc = (u32)dibdc;
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

// 0x10007990
s32 RADEXPLINK BinkBufferSetHWND(HBINKBUFFER buf, HWND newwnd)
{
    return FALSE;
}

// 0x10005e60
s32 RADEXPLINK BinkBufferSetOffset(HBINKBUFFER buf, s32 destx, s32 desty)
{
    if (buf == NULL) { return FALSE; }

    POINT point;
    point.y = 0;
    point.x = 0;

    ClientToScreen((HWND)buf->wnd, &point);

    buf->destx = point.x + destx;
    buf->wndx = destx;
    buf->desty = point.y + desty;
    buf->wndy = desty;
    buf->SurfaceType = buf->SurfaceType | BINKCOPYALL;

    BinkBufferMove(buf, BINKBUFFERMOVEMODE_NONE);

    return TRUE;
}

// 0x10005d90
void RADEXPLINK BinkBufferSetResolution(s32 w, s32 h, s32 bits)
{
    BufferWidth = w;
    BufferHeight = h;
    BufferBits = bits;
}

// 0x10007880
s32 RADEXPLINK BinkBufferSetScale(HBINKBUFFER buf, u32 w, u32 h)
{
    if (buf != NULL) {
        if (w == 0) { w = Width; }
        if (h == 0) {h = Height; }

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

        if ((buf->ScaleFlags & options) == options) { buf->StretchHeight = h; } else { result = 0; }

        buf->WindowWidth = buf->StretchWidth + buf->ExtraWindowWidth;
        buf->WindowHeight = buf->StretchHeight + buf->ExtraWindowHeight;

        return result;
    }

    return 0;
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

// 0x10009bc0
s32 RADEXPLINK BinkDoFrame(HBINK bnk)
{
    // TODO NOT IMPLEMENTED

    return TRUE;
}

// 0x10007e40
char PTR4* RADEXPLINK BinkGetError(void)
{
    return BinkError;
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
    if (GetSystemMetrics(SM_MOUSEPRESENT)) {
        LPDIRECTDRAWSURFACE surface = (LPDIRECTDRAWSURFACE)lpDDSP;

        DDSURFACEDESC desc;
        radmemset(&desc, 0x00, sizeof(DDSURFACEDESC));

        desc.dwSize = sizeof(DDSURFACEDESC);

        desc.dwFlags = DDSD_CAPS;
        desc.ddsCaps.dwCaps = 0x200; // TODO

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

            while (ShowCursor(FALSE) >= 0) { count = count + 1; }

            COLORREF color = NULL;

            {
                HDC dc = GetDC(wnd);
                color = GetPixel(dc, target.x, target.y);
                ReleaseDC(wnd, dc);
            }

            HCURSOR cursor = SetCursor(cur);

            while (ShowCursor(TRUE) < 0) { count = count - 1; }

            SetCursor(cur);

            surface->Lock(NULL, &desc, DDLOCK_WAIT, NULL);

            {
                u8* ptr = (u8*)((size_t)desc.lpSurface
                    + desc.lPitch * point.y + (point.x * desc.ddpfPixelFormat.dwRGBBitCount >> 3));

                ptr[0] = color == NULL ? 0xFF : 0x00;
            }

            surface->Unlock(desc.lpSurface);

            while (ShowCursor(FALSE) >= 0) { count = count + 1; }

            HDC dc = GetDC(wnd);
            const u32 match = GetPixel(dc, target.x, target.y) == color;

            if (!match) { SetPixel(dc, target.x, target.y, color); }

            ReleaseDC(wnd, dc);
            DestroyWindow(wnd);
            UnregisterClassA(CURSOR_TYPE_WINDOW_NAME, inst);

            if (count <= 0) {
                for (s32 i = 0; i <= -count; i++) { ShowCursor(FALSE); }
            }
            else { for (s32 i = 0; i <= count; i++) { ShowCursor(TRUE); }
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

// 0x1000b020
void PTR4* RADEXPLINK BinkLogoAddress(void)
{
    return BinkVideo;
}

// 0x1000a0c0
void RADEXPLINK BinkNextFrame(HBINK bnk)
{
    // TODO NOT IMPLEMENTED
}

// 0x10007f00
HBINK RADEXPLINK BinkOpen(const char PTR4* name, u32 flags)
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

    QueueBinkMem(&bink.MaskPlane, (size_t)(bink.MaskLength + 0x10)); // TODO
    QueueBinkMem(&bink.rtframetimes, (size_t)(bink.runtimeframes * sizeof(size_t)));
    QueueBinkMem(&bink.rtadecomptimes, (size_t)(bink.runtimeframes * sizeof(size_t)));
    QueueBinkMem(&bink.rtvdecomptimes, (size_t)(bink.runtimeframes * sizeof(size_t)));
    QueueBinkMem(&bink.rtblittimes, (size_t)(bink.runtimeframes * sizeof(size_t)));
    QueueBinkMem(&bink.rtreadtimes, (size_t)(bink.runtimeframes * sizeof(size_t)));
    QueueBinkMem(&bink.rtidlereadtimes, (size_t)(bink.runtimeframes * sizeof(size_t)));
    QueueBinkMem(&bink.rtthreadreadtimes, (size_t)(bink.runtimeframes * sizeof(size_t)));

    ExpandBundleSizes(&bink.bunp, bink.YWidth);

    QueueBinkMem(&bink.bunp, (size_t)bink.bunp.typeptr);
    QueueBinkMem(&bink.bunp.type16ptr, (size_t)bink.bunp.type16ptr);
    QueueBinkMem(&bink.bunp.colorptr, (size_t)bink.bunp.colorptr);
    QueueBinkMem(&bink.bunp.bits2ptr, (size_t)bink.bunp.bits2ptr);
    QueueBinkMem(&bink.bunp.motionXptr, (size_t)bink.bunp.motionXptr);
    QueueBinkMem(&bink.bunp.motionYptr, (size_t)bink.bunp.motionYptr);
    QueueBinkMem(&bink.bunp.dctptr, (size_t)bink.bunp.dctptr);
    QueueBinkMem(&bink.bunp.mdctptr, (size_t)bink.bunp.mdctptr);
    QueueBinkMem(&bink.bunp.patptr, (size_t)bink.bunp.patptr);

    if (!(flags & BINKFROMMEMORY)) {
        QueueBinkMem(&bink.frameoffsets, (bink.InternalFrames + 1) * sizeof(size_t));
        QueueBinkMem(&bink.tracksizes, bink.NumTracks * sizeof(size_t));
        QueueBinkMem(&bink.tracktypes, bink.NumTracks * sizeof(size_t));
        QueueBinkMem(&bink.trackIDs, bink.NumTracks * sizeof(size_t));
    }

    BINK* result = (HBINK)AllocateBinkMem(&bink, sizeof(BINK));

    //TODO
    //if (result != NULL)
    //{
    //    radmemcpy(result, &bink, sizeof(BINK));
    //
    //    result->bio.bink = result;
    //
    //    result->rtadecomptimes = NULL;
    //    result->rtvdecomptimes = NULL;
    //    result->rtblittimes = NULL;
    //    result->rtreadtimes = NULL;
    //    result->rtidlereadtimes = NULL;
    //    result->rtthreadreadtimes = NULL;
    //
    //    if (flags & BINKFROMMEMORY) {
    //        result->tracksizes      = (u32*)((size_t)name + sizeof(BINKIOHEADER));
    //        result->tracktypes      = (u32*)((size_t)name + result->NumTracks * 1 * sizeof(u32) + sizeof(BINKIOHEADER));
    //        result->trackIDs        = (s32*)((size_t)name + result->NumTracks * 2 * sizeof(u32) + sizeof(BINKIOHEADER));
    //        result->frameoffsets    = (u32*)((size_t)name + result->NumTracks * 3 * sizeof(u32) + sizeof(BINKIOHEADER));
    //    }
    //    else {
    //        result->bio.ReadHeader(&result->bio, -1, result->tracksizes, result->NumTracks << 2);
    //        result->bio.ReadHeader(&result->bio, -1, result->tracktypes, result->NumTracks << 2);
    //        result->bio.ReadHeader(&result->bio, -1, result->trackIDs, result->NumTracks << 2);
    //        result->bio.ReadHeader(&result->bio, -1, result->frameoffsets, result->InternalFrames * 4 + 4);
    //    }
    //
    //    result->Highest1SecRate =
    //        FUN_10008ef0(result->Frames, result->frameoffsets,
    //            result->runtimeframes, &result->Highest1SecFrame, &local_8);
    //
    //    if (!(result->OpenFlags & BINKALPHA)) {
    //        if (local_8 == 0) {
    //            QueueBinkMem(result->YPlane + 1 /* TODO */,
    //                (size_t)(result->YHeight * result->YWidth + result->UVHeight * result->UVWidth * 2));
    //        }
    //    }
    //    else {
    //        QueueBinkMem(result->APlane /* TODO */, (size_t)(result->YWidth* result->YHeight));
    //
    //        if (local_8 == 0) {
    //            QueueBinkMem(result->APlane + 1 /* TODO */, (size_t)(result->YWidth* result->YHeight));
    //
    //            if (local_8 == 0) {
    //                QueueBinkMem(result->YPlane + 1 /* TODO */,
    //                    (size_t)(result->YHeight * result->YWidth + result->UVHeight * result->UVWidth * 2));
    //            }
    //        }
    //    }
    //
    //    result->YPlane[0]
    //        = AllocateBinkMem(result, result->YHeight * result->YWidth + result->UVHeight * result->UVWidth * 2);
    //
    //    if (result->YPlane[0] != NULL) {
    //        if (local_8 != 0) {
    //            result->YPlane[1] = result->YPlane[0];
    //            result->APlane[1] = result->APlane[0];
    //        }
    //        if (!(flags & BINKIOSIZE) || IOSize == DEFAULT_IO_SIZE) {
    //            result->iosize = result->Highest1SecRate;
    //        }
    //        else {
    //            result->iosize = IOSize;
    //            IOSize = DEFAULT_IO_SIZE;
    //        }
    //
    //        if (!(flags & BINKSIMULATE) || Simulate == DEFAULT_SIMULATE) {
    //            local_c = 0;
    //        }
    //        else {
    //            local_c = Simulate;
    //            Simulate = DEFAULT_SIMULATE;
    //        }
    //
    //        if (flags & BINKFROMMEMORY) {
    //            result->preloadptr = (void*)((size_t)name + ALIGN_FRAME_OFFSETS(result->frameoffsets[0]));
    //        LAB_1000876e:
    //            *(undefined*)(result->MaskLength + (int)result->MaskPlane) = 0;
    //            result->FrameNum = 0xffffffff;
    //
    //            // TODO 2000 constant
    //            if (result->FrameRate == 0) { result->twoframestime = 2000; }
    //            else { result->twoframestime = (result->FrameRateDiv * 2000) / result->FrameRate; }
    //
    //            result->videoon = TRUE;
    //
    //            FUN_10008af0(result, 1); // TODO
    //
    //            uVar2 = RADTimerRead();
    //
    //            rects[i].timeopen = uVar2 - rects[i].timeopen;
    //            if ((rects[i].preloadptr == (void*)0x0) && ((flags & 0x200000) == 0)) {
    //                iVar9 = (*(code*)(rects[i].bio).Idle)(&rects[i].bio);
    //                while (iVar9 != 0) {
    //                    iVar9 = (*(code*)(rects[i].bio).Idle)(&rects[i].bio);
    //                }
    //            }
    //            iVar9 = rects[i].NumTracks;
    //            if (iVar9 == 0) { rects[i].trackindex = DEFAULT_SOUND_TRACK; }
    //            else if ((rects[i].OpenFlags & 0x4000) == 0) {
    //                rects[i].trackindex = 0;
    //            }
    //            else if (DAT_1003a07c == -1) { rects[i].trackindex = DEFAULT_SOUND_TRACK; }
    //            else {
    //                iVar5 = 0;
    //                if (0 < iVar9) {
    //                    piVar10 = rects[i].trackIDs;
    //                    do {
    //                        if (*piVar10 == DAT_1003a07c) break;
    //                        iVar5 = iVar5 + 1;
    //                        piVar10 = piVar10 + 1;
    //                    } while (iVar5 < iVar9);
    //                }
    //                if (iVar9 <= iVar5) {
    //                    iVar5 = -1;
    //                }
    //                rects[i].trackindex = iVar5;
    //            }
    //
    //            DAT_1003a07c = 0xffffffff;
    //
    //            if (rects[i].trackindex != DEFAULT_SOUND_TRACK) {
    //                if (((rects[i].tracktypes[rects[i].trackindex] & 0x80000000) != 0) &&
    //                    (SoundState.Result != (code*)0x0)) {
    //                    uVar2 = rects[i].tracktypes[rects[i].trackindex] & 0xffff;
    //                    if ((bink.FrameRate != 0) && (bink.FrameRateDiv != 0)) {
    //                        uVar2 = (uint)(((ulonglong)uVar2 * (ulonglong)(bink.fileframeratediv * bink.FrameRate)
    //                            ) / (ulonglong)(bink.fileframerate * bink.FrameRateDiv));
    //                    }
    //                    iVar9 = (*SoundState.Result)
    //                        (&rects[i].bsnd, uVar2,
    //                            (rects[i].tracktypes[rects[i].trackindex] >> 0x1e & 1) * 8 + 8,
    //                            (rects[i].tracktypes[rects[i].trackindex] >> 0x1d & 1) + 1,
    //                            rects[i].OpenFlags, pBVar3);
    //                    if (iVar9 != 0) {
    //                        SoundState.Counter = SoundState.Counter + 1;
    //                        rects[i].sndconvert8 =
    //                            (uint)((rects[i].tracktypes[rects[i].trackindex] >> 0x1b & 8) == 0);
    //
    //                        rects[i].soundon = TRUE;
    //                        rects[i].sndbufsize = rects[i].tracksizes[rects[i].trackindex] + 0xff & 0xffffff00;
    //
    //                        puVar6 = radmalloc(rects[i].sndbufsize);
    //                        rects[i].sndbuf = puVar6;
    //                        rects[i].sndwritepos = puVar6;
    //                        rects[i].sndreadpos = puVar6;
    //
    //                        uVar11 = (rects[i].bsnd).Latency;
    //                        rects[i].sndend = puVar6 + rects[i].sndbufsize;
    //                        uVar2 = (uint)(((ulonglong)
    //                            (((rects[i].tracktypes[rects[i].trackindex] >> 0x1d & 1) + 1) * uVar2 *
    //                                2) * (ulonglong)(0x2ee - uVar11)) / 1000) & 0xfffffffc;
    //                        rects[i].sndprime = uVar2;
    //                        if (rects[i].sndbufsize < uVar2) {
    //                            rects[i].sndprime = rects[i].sndbufsize;
    //                        }
    //                        pscales = FUN_10028890(rects[i].tracktypes[rects[i].trackindex] & 0xffff,
    //                            (rects[i].tracktypes[rects[i].trackindex] >> 0x1d & 1) + 1, 1);
    //                        rects[i].sndcomp = (uint)pscales;
    //                        rects[i].sndamt = 0;
    //                        rects[i].sndendframe =
    //                            rects[i].Frames - (rects[i].fileframerate * 3) / (rects[i].fileframeratediv << 2);
    //                    }
    //                }
    //
    //                if (rects[i].soundon == 0) { rects[i].trackindex = DEFAULT_SOUND_TRACK; }
    //            }
    //
    //            if ((((result->OpenFlags & BINKNOTHREADEDIO) == 0) && result->preloadptr == NULL) // TODO
    //                || (result->trackindex != DEFAULT_SOUND_TRACK)) {
    //                result->threadcontrol = THREAD_CONTROL_ACTIVE;
    //                result->BackgroundThread = *(u32*)CreateBinkThread(BinkThreadAction, result, NULL);
    //
    //                if (result->BackgroundThread == NULL) {
    //                    result->threadcontrol = THREAD_CONTROL_NONE;
    //
    //                    BinkClose(result);
    //
    //                    return NULL;
    //                }
    //            }
    //
    //            return result;
    //        }
    //
    //        rects[i].bio->iosize = rects[i].bio.GetBufferSize(rects[i].bio, rects[i].iosize);
    //
    //        if ((result->Size * 9) / 10 <= uVar2) {
    //            flags = flags | BINKPRELOADALL;
    //            rects[i].OpenFlags = result->OpenFlags | BINKPRELOADALL;
    //        }
    //
    //        if (!(flags & BINKPRELOADALL)) {
    //            QueueBinkMem(&result->compframe, result->LargestFrameSize);
    //
    //            result->ioptr = (u8*)AllocateBinkMem(result, result->iosize);
    //
    //            if (result->ioptr == NULL) { rects[i].iosize = 0; }
    //            
    //            result->bio.SetInfo(&result->bio, result->ioptr, result->iosize, result->Size + 8, local_c);
    //
    //            goto LAB_1000876e;
    //        }
    //
    //        const u32 size = result->Size - ALIGN_FRAME_OFFSETS(result->frameoffsets[0]) + 8;
    //
    //        result->preloadptr = AllocateBinkMem(result, size);
    //
    //        if (result->preloadptr != NULL) {
    //            result->bio.SetInfo(&result->bio, NULL, 0, result->Size + 8, local_c);
    //            result->bio.ReadFrame(&result->bio, 0,
    //                ALIGN_FRAME_OFFSETS(result->frameoffsets[0]), result->preloadptr, size);
    //            result->bio.Close(&result->bio);
    //
    //            result->bio.ForegroundTime = 0;
    //            
    //            goto LAB_1000876e;
    //        }
    //
    //        radfree(bink.YPlane);
    //    }
    //    
    //    radfree(result);
    //}

    BinkSetError("Out of memory.");

    if (!(flags & BINKFROMMEMORY)) { bink.bio.Close(&bink.bio); }

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
    if (checkcount != 0)
    {
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
    radstrcpy(BinkError, err);
}

// 0x10007ea0
void RADEXPLINK BinkSetFrameRate(u32 forcerate, u32 forceratediv)
{
    FrameRate = forcerate;
    FrameRateDiv = forceratediv;
}

// 0x10007ed0
void RADEXPLINK BinkSetIO(BINKIOOPEN io)
{
    IO = io;
}

// 0x10007ec0
void RADEXPLINK BinkSetIOSize(u32 iosize)
{
    IOSize = iosize;
}

// 0x1000aff0
void RADEXPLINK BinkSetPan(HBINK bnk, s32 pan)
{
    if (bnk != NULL && bnk->trackindex != DEFAULT_SOUND_TRACK && bnk->bsnd.Pan != NULL) {
        bnk->bsnd.Pan(&bnk->bsnd, pan);
    }
}

// 0x10007ee0
void RADEXPLINK BinkSetSimulate(u32 sim)
{
    Simulate = sim;
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
    if (bnk != NULL && bnk->trackindex != DEFAULT_SOUND_TRACK && bnk->bsnd.Volume != NULL) {
        bnk->bsnd.Volume(&bnk->bsnd, volume);
    }
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
void RADEXPLINK ExpandBundleSizes(struct BUNDLEPOINTERS * pointers, u32 width)
{
    // TODO NOT IMPLEMENTED
}

// 0x10001000
void RADEXPLINK RADSetMemory(RADMEMALLOC a, RADMEMFREE f)
{
    MemAllocate = a;
    MemFree = f;
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

        type = EXTERNAL_MEMORY_ALLOCATION;
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