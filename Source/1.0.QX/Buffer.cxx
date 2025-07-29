/*
Copyright (c) 2025 Americus Maximus

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

#define MAX_BUFFER_DEPTH_COUNT  4

char                    BufferDescription[MAX_BUFFER_ERROR_LENGTH];
s32                     BufferScreenHeight;
s32                     BufferScreenWidth;
DWORD                   BufferFillColor;
char                    BufferError[MAX_BUFFER_ERROR_LENGTH];

s32                     BufferCount;
LPDIRECTDRAW            BufferDirectDraw;
LPDIRECTDRAWSURFACE     BufferDirectDrawSurface;
u32                     BufferWidth;
u32                     BufferHeight;
u32                     BufferBits;
u32                     BufferBytes;
u32                     BufferScreenBits;
DIRECTDRAWCREATEACTION  BufferDirectDrawCreate;
HMODULE                 BufferDirectDrawModule;
u32                     BufferOverlayCaps;
u32                     BufferBlitCaps;
u32                     BufferIsDeviceContext;
s32                     BufferDeviceContextCount;

// 0x100056e0
void BinkBufferDirectDrawCapabilitiesInitialize()
{
    DDCAPS caps;
    radmemset(&caps, 0x00, sizeof(DDCAPS));

    caps.dwSize = sizeof(DDCAPS);

    BufferDirectDraw->GetCaps(&caps, NULL);

    if (caps.dwFXCaps & DDFXCAPS_OVERLAYSTRETCHX) { BufferOverlayCaps |= BINKBUFFERSTRETCHXINT | BINKBUFFERSTRETCHX; }
    if (caps.dwFXCaps & DDFXCAPS_OVERLAYSTRETCHXN) { BufferOverlayCaps |= BINKBUFFERSTRETCHXINT; }
    if (caps.dwFXCaps & DDFXCAPS_OVERLAYSHRINKX) { BufferOverlayCaps |= BINKBUFFERSHRINKXINT | BINKBUFFERSHRINKX; }
    if (caps.dwFXCaps & DDFXCAPS_OVERLAYSHRINKXN) { BufferOverlayCaps |= BINKBUFFERSHRINKXINT; }
    if (caps.dwFXCaps & DDFXCAPS_OVERLAYSTRETCHY) { BufferOverlayCaps |= BINKBUFFERSTRETCHYINT | BINKBUFFERSTRETCHY; }
    if (caps.dwFXCaps & DDFXCAPS_OVERLAYSTRETCHYN) { BufferOverlayCaps |= BINKBUFFERSTRETCHYINT; }
    if (caps.dwFXCaps & DDFXCAPS_OVERLAYSHRINKY) { BufferOverlayCaps |= BINKBUFFERSHRINKYINT | BINKBUFFERSHRINKY; }
    if (caps.dwFXCaps & DDFXCAPS_OVERLAYSHRINKYN) { BufferOverlayCaps |= BINKBUFFERSHRINKYINT; }

    if (caps.dwFXCaps & DDFXCAPS_BLTSTRETCHX) { BufferBlitCaps |= BINKBUFFERSTRETCHXINT | BINKBUFFERSTRETCHX; }
    if (caps.dwFXCaps & DDFXCAPS_BLTSTRETCHXN) { BufferBlitCaps |= BINKBUFFERSTRETCHXINT; }
    if (caps.dwFXCaps & DDFXCAPS_BLTSHRINKX) { BufferBlitCaps |= BINKBUFFERSHRINKXINT | BINKBUFFERSHRINKX; }
    if (caps.dwFXCaps & DDFXCAPS_BLTSHRINKXN) { BufferBlitCaps |= BINKBUFFERSHRINKXINT; }
    if (caps.dwFXCaps & DDFXCAPS_BLTSTRETCHY) { BufferBlitCaps |= BINKBUFFERSTRETCHYINT | BINKBUFFERSTRETCHY; }
    if (caps.dwFXCaps & DDFXCAPS_BLTSTRETCHYN) { BufferBlitCaps |= BINKBUFFERSTRETCHYINT; }
    if (caps.dwFXCaps & DDFXCAPS_BLTSHRINKY) { BufferBlitCaps |= BINKBUFFERSHRINKYINT | BINKBUFFERSHRINKY; }
    if (caps.dwFXCaps & DDFXCAPS_BLTSHRINKYN) { BufferBlitCaps |= BINKBUFFERSHRINKYINT; }

    DDPIXELFORMAT format;
    radmemset(&format, 0x00, sizeof(DDPIXELFORMAT));

    format.dwSize = sizeof(DDPIXELFORMAT);

    BufferDirectDrawSurface->GetPixelFormat(&format);

    BufferBytes = format.dwRGBBitCount >> 3;

    BufferScreenWidth = GetSystemMetrics(SM_CXSCREEN);
    BufferScreenHeight = GetSystemMetrics(SM_CYSCREEN);

    HDC hdc = GetDC(NULL);
    BufferScreenBits = GetDeviceCaps(hdc, PLANES) * GetDeviceCaps(hdc, BITSPIXEL);
    ReleaseDC(NULL, hdc);

    BufferFillColor = BufferScreenBits == 8
        ? 0xFD : BinkBufferAcquireColor(1, format.dwBBitMask) + BinkBufferAcquireColor(1, format.dwRBitMask);
}

// 0x100058d0
u32 BinkBufferAcquireColor(u32 value, u32 mask)
{
    u32 result = 0;
    u32 mm = 1, vm = 1;

    for (u32 x = 0; x < 32; x++) {
        if (mask & mm) {
            if (value & vm) { result |= mm; }

            vm = vm << 1;
        }

        mm = mm << 1;
    }

    return result;
}

// 0x10005ee0
void BinkBufferMove(HBINKBUFFER buf, u32 mode)
{
    if (buf->ddoverlay) {
        if (IsWindowVisible((HWND)buf->wnd)) {
            RECT src, dst;

            src.right = buf->Width;
            src.bottom = buf->Height;
            src.left = 0;
            src.top = 0;
            dst.left = buf->destx;

            if (dst.left < 1) {
                src.left = -dst.left;
                dst.left = 0;
            }

            dst.top = buf->desty;
            if (buf->desty < 1) {
                dst.top = 0;
                src.top = -buf->desty;
            }

            dst.right = buf->StretchWidth + buf->destx;
            dst.bottom = buf->StretchHeight + buf->desty;

            if (BufferScreenWidth < dst.right) {
                src.right = src.right + BufferScreenWidth - dst.right;
                dst.right = BufferScreenWidth;
            }

            if (BufferScreenHeight < dst.bottom) {
                src.bottom = src.bottom + BufferScreenHeight - dst.bottom;
                dst.bottom = BufferScreenHeight;
            }

            if (src.left <= src.right && src.top <= src.bottom && dst.left <= dst.right && dst.top <= dst.bottom) {
                if (mode == BINKBUFFERMOVEMODE_HIDE) { buf->lastovershow = FALSE; }
                else if (mode == BINKBUFFERMOVEMODE_SHOW) { buf->lastovershow = TRUE; }

                DDOVERLAYFX fx;

                fx.dwSize = sizeof(DDOVERLAYFX);
                fx.dckDestColorkey.dwColorSpaceLowValue = BufferFillColor;
                fx.dckDestColorkey.dwColorSpaceHighValue = BufferFillColor;

                ((LPDIRECTDRAWSURFACE)buf->ddsurface)->UpdateOverlay(&src, BufferDirectDrawSurface, &dst,
                    buf->lastovershow ? (DDOVER_SHOW | DDOVER_KEYDESTOVERRIDE) : (DDOVER_KEYDESTOVERRIDE | DDOVER_HIDE), &fx);
                return;
            }
        }

        ((LPDIRECTDRAWSURFACE)buf->ddsurface)->UpdateOverlay(NULL, BufferDirectDrawSurface, NULL, DDOVER_HIDE, NULL);
    }
}

// 0x10006ac0
void BinkBufferDirectDrawLoad()
{
    UINT uMode = SetErrorMode(SEM_NOOPENFILEERRORBOX);
    BufferDirectDrawModule = LoadLibraryA("DDRAW.DLL");
    SetErrorMode(uMode);

    if (BufferDirectDrawModule >= MAX_INVALID_MODULE_HANDLE) {
        BufferDirectDrawCreate =
            (DIRECTDRAWCREATEACTION)GetProcAddress(BufferDirectDrawModule, "DirectDrawCreate");

        if (BufferDirectDrawCreate != NULL) { return; }

        FreeLibrary(BufferDirectDrawModule);
    }

    BufferDirectDrawModule = MODULE_HANDLE_NOT_LOADED;
}

// 0x100068d0
void BinkBufferDirectDrawInitialize(HWND wnd, u32 fullscreen)
{
    if (BufferCount != 0) {
        BufferCount = BufferCount + 1;
        return;
    }

    if (BufferDirectDrawModule == NULL) { BinkBufferDirectDrawLoad(); }
    if (BufferDirectDrawModule < MAX_INVALID_MODULE_HANDLE) { return; }

    if (FAILED(BufferDirectDrawCreate(NULL, &BufferDirectDraw, NULL))) { return; }

    if (fullscreen && (BufferWidth == 0 || BufferHeight == 0 || BufferBits == 0)) { fullscreen = FALSE; }

    if (SUCCEEDED(BufferDirectDraw->SetCooperativeLevel(wnd,
        fullscreen ? (DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN) : DDSCL_NORMAL))) {
        const u32 bits[MAX_BUFFER_DEPTH_COUNT] = { BufferBits, 32, 24, 16 };

        for (u32 x = 0; x < MAX_BUFFER_DEPTH_COUNT; x++) {
            BufferBits = bits[x];

            if (SUCCEEDED(BufferDirectDraw->SetDisplayMode(BufferWidth, BufferHeight, BufferBits))) {
                HDC hdc = GetDC(wnd);
                BufferScreenBits = GetDeviceCaps(hdc, PLANES) * GetDeviceCaps(hdc, BITSPIXEL);
                ReleaseDC(wnd, hdc);
            }

            if (BufferScreenBits == BufferBits) { break; }
        }

        BufferBits = 0;
        BufferWidth = 0;
        BufferHeight = 0;

        DDSURFACEDESC desc;
        radmemset(&desc, 0x00, sizeof(DDSURFACEDESC));

        desc.dwSize = sizeof(DDSURFACEDESC);
        desc.dwFlags = DDSD_CAPS;
        desc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

        if (SUCCEEDED(BufferDirectDraw->CreateSurface(&desc, &BufferDirectDrawSurface, NULL))) {
            BinkBufferDirectDrawCapabilitiesInitialize();

            BufferCount = BufferCount + 1;

            return;
        }
    }

    BufferDirectDraw->Release();
}

// 0x10006b20
void BinkBufferDirectDrawRelease()
{
    BufferCount = BufferCount - 1;

    if (BufferCount == 0) {
        BufferDirectDrawSurface->Release();
        BufferDirectDraw->Release();

        BufferDirectDraw = NULL;
        BufferDirectDrawSurface = NULL;
    }
}

// 0x10006b50
u32 BinkBufferDeviceContextInitialize(HWND wnd, u32 fullscreen)
{
    if (BufferCount != 0) { return FALSE; }

    if (fullscreen && BufferWidth != 0 && BufferHeight != 0 && BufferBits != 0) {
        DEVMODEA mode;
        const u32 bits[MAX_BUFFER_DEPTH_COUNT] = { BufferBits, 32, 24, 16 };

        for (u32 x = 0; x < MAX_BUFFER_DEPTH_COUNT; x++) {
            BufferBits = bits[x];

            mode.dmSize = sizeof(DEVMODEA);
            mode.dmFields = DM_PELSHEIGHT | DM_PELSWIDTH | DM_BITSPERPEL;
            mode.dmPelsWidth = BufferWidth;
            mode.dmPelsHeight = BufferHeight;
            mode.dmBitsPerPel = BufferBits;

            if (ChangeDisplaySettingsA(&mode, CDS_FULLSCREEN) == DISP_CHANGE_SUCCESSFUL) {
                BufferIsDeviceContext = TRUE;

                HDC hdc = GetDC(wnd);
                BufferScreenBits = GetDeviceCaps(hdc, PLANES) * GetDeviceCaps(hdc, BITSPIXEL);
                ReleaseDC(wnd, hdc);
            }

            if (BufferScreenBits == BufferBits) { break; }

            ChangeDisplaySettingsA(NULL, CDS_FULLSCREEN);
        }

        mode.dmSize = sizeof(DEVMODEA);
        mode.dmFields = DM_PELSHEIGHT | DM_PELSWIDTH;
        mode.dmPelsWidth = BufferWidth;
        mode.dmPelsHeight = BufferHeight;

        if (ChangeDisplaySettingsA(&mode, CDS_FULLSCREEN) == DISP_CHANGE_SUCCESSFUL) {
            BufferIsDeviceContext = TRUE;
        }

        BufferBits = 0;
        BufferWidth = 0;
        BufferWidth = 0;
    }

    BufferScreenWidth = GetSystemMetrics(SM_CXSCREEN);
    BufferScreenHeight = GetSystemMetrics(SM_CYSCREEN);

    HDC hdc = GetDC(wnd);
    BufferScreenBits = GetDeviceCaps(hdc, PLANES) * GetDeviceCaps(hdc, BITSPIXEL);
    ReleaseDC(wnd, hdc);

    BufferBytes = BufferScreenBits >> 3;

    BufferDeviceContextCount = BufferDeviceContextCount + 1;

    return TRUE;
}

// 0x10006d80
void BinkBufferDeviceContextRelease()
{
    BufferDeviceContextCount = BufferDeviceContextCount - 1;

    if (BufferDeviceContextCount == 0 && BufferIsDeviceContext) {
        BufferIsDeviceContext = FALSE;
        ChangeDisplaySettingsA(NULL, CDS_FULLSCREEN);
    }
}

// 0x10006ea0
void BinkBufferClear(void* buf, u32 type, u32 pitch, u32 width, u32 height)
{
    switch (type & BINKSURFACEMASK) {
    case BINKSURFACE8P: {
        const u32 length = BufferBytes * width;
        void* ptr = buf;

        for (u32 x = 0; x < height; x++) {
            radmemset(ptr, 0x00, length);
            ptr = (void*)((size_t)ptr + pitch);
        }

        return;
    }
    case BINKSURFACE24:
    case BINKSURFACE32:
    case BINKSURFACE555:
    case BINKSURFACE565:
    case BINKSURFACE655:
    case BINKSURFACE664: {
        const u32 length = BufferBytes * width;
        void* ptr = buf;

        for (u32 x = 0; x < height; x++) {
            radmemset(ptr, 0x00, length);
            ptr = (void*)((size_t)ptr + pitch);
        }

        return;
    }
    case BINKSURFACEYUY2: {
        u16* ptr = (u16*)buf;

        for (u32 x = 0; x < height; x++) {
            for (u32 xx = 0; xx < width; xx++) {
                ptr[xx] = 0x8000;
            }

            ptr = (u16*)((size_t)ptr + pitch);
        }

        return;
    }
    case BINKSURFACEUYVY: {
        u16* ptr = (u16*)buf;

        for (u32 x = 0; x < height; x++) {
            for (u32 xx = 0; xx < width; xx++) {
                ptr[xx] = 0x80;
            }

            ptr = (u16*)((size_t)ptr + pitch);
        }

        return;
    }
    case BINKSURFACEYV12: {
        {
            void* ptr = buf;

            for (u32 x = 0; x < height; x++) {
                radmemset(ptr, 0x10, width * sizeof(u16));
                ptr = (void*)((size_t)ptr + pitch);
            }
        }

        {
            u16* ptr = (u16*)((size_t)buf + (pitch * height));

            for (u32 x = 0; x < height / sizeof(u16); x++) {
                for (u32 xx = 0; xx < width / sizeof(u16); xx++) {
                    ptr[xx] = 0x8080;
                }

                ptr = (u16*)((size_t)ptr + pitch);
            }
        }

        return;
    }
    }
}

// 0x10006db0
LPDIRECTDRAWSURFACE BinkBufferDirectDrawOverlaySurfaceCreate(u32 format, u32 width, u32 height, u32 type)
{
    DDSURFACEDESC desc;
    radmemset(&desc, 0x00, sizeof(DDSURFACEDESC));

    desc.dwSize = sizeof(DDSURFACEDESC);

    desc.dwFlags = DDSD_PIXELFORMAT | DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
    desc.dwWidth = width;
    desc.dwHeight = height;
    desc.ddsCaps.dwCaps = DDSCAPS_VIDEOMEMORY | DDSCAPS_OVERLAY;

    desc.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    desc.ddpfPixelFormat.dwFlags = DDPF_FOURCC;
    desc.ddpfPixelFormat.dwFourCC = format;

    LPDIRECTDRAWSURFACE surface = NULL;
    if (FAILED(BufferDirectDraw->CreateSurface(&desc, &surface, NULL))) { return NULL; }

    if (FAILED(surface->Lock(NULL, &desc, DDLOCK_WAIT, NULL)))
    {
        surface->Release();

        return NULL;
    }

    BinkBufferClear(desc.lpSurface, type, desc.lPitch, width, height);

    surface->Unlock(desc.lpSurface);

    return surface;
}

// 0x10007090
LPDIRECTDRAWSURFACE BinkBufferDirectDrawOffscreenSurfaceCreate(u32 format, u32 width, u32 height, u32 type)
{
    DDSURFACEDESC desc;
    radmemset(&desc, 0x00, sizeof(DDSURFACEDESC));

    desc.dwSize = sizeof(DDSURFACEDESC);

    desc.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
    desc.dwHeight = height;
    desc.dwWidth = width;
    desc.ddsCaps.dwCaps = type == BINKSURFACE24
        ? (DDSCAPS_SYSTEMMEMORY | DDSCAPS_OFFSCREENPLAIN) : (DDSCAPS_VIDEOMEMORY | DDSCAPS_OFFSCREENPLAIN);

    if (0x10 /* TODO */ < format) {
        desc.dwFlags = DDSD_PIXELFORMAT | DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;

        desc.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
        desc.ddpfPixelFormat.dwFlags = DDPF_FOURCC;
        desc.ddpfPixelFormat.dwFourCC = format;
    }

    LPDIRECTDRAWSURFACE surface = NULL;
    if (FAILED(BufferDirectDraw->CreateSurface(&desc, &surface, NULL))) { return NULL; }

    if (FAILED(surface->Lock(NULL, &desc, DDLOCK_WAIT, NULL)))
    {
        surface->Release();

        return NULL;
    }

    BinkBufferClear(desc.lpSurface, type, desc.lPitch, width, height);

    surface->Unlock(desc.lpSurface);

    return surface;
}

// 0x10007190
u32 BinkBufferDeviceContextSurfaceType(HDC hdc, LPRGBQUAD quad)
{
    if (BufferBytes >= 2) {
        quad[0].rgbBlue = 0x00;
        quad[0].rgbGreen = 0x00;
        quad[0].rgbRed = 0xFF;
        quad[0].rgbReserved = 0x00;

        quad[1].rgbBlue = 0x00;
        quad[1].rgbGreen = 0xFF;
        quad[1].rgbRed = 0x00;
        quad[1].rgbReserved = 0x00;

        quad[2].rgbBlue = 0xFF;
        quad[2].rgbGreen = 0x00;
        quad[2].rgbRed = 0x00;
        quad[2].rgbReserved = 0x00;

        return BufferBytes == 3 ? BINKSURFACE24 : BINKSURFACE32;
    }

    u8 bmpmem[BINK_BUFFER_BITMAP_MEMORY_SIZE];
    radmemset(bmpmem, 0x00, BINK_BUFFER_BITMAP_MEMORY_SIZE);

    LPBITMAPINFO bmpi = (LPBITMAPINFO)bmpmem;

    bmpi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);

    HBITMAP bmp = CreateCompatibleBitmap(hdc, 1, 1);

    if (bmp != NULL) {
        GetDIBits(hdc, bmp, 0, 1, NULL, bmpi, DIB_RGB_COLORS);
        GetDIBits(hdc, bmp, 0, 1, NULL, bmpi, DIB_RGB_COLORS);
        DeleteObject(bmp);

        const RGBQUAD color = { 0x00, 0x00, 0xF8, 0x00 };

        if (bmpi->bmiHeader.biCompression == BI_BITFIELDS
            && radmemcmp(&bmpi->bmiColors[0], &color, sizeof(RGBQUAD))) {
            quad[0].rgbBlue = 0x00;
            quad[0].rgbGreen = 0xF8;
            quad[0].rgbRed = 0x00;
            quad[0].rgbReserved = 0x00;

            quad[1].rgbBlue = 0xE0;
            quad[1].rgbGreen = 0x07;
            quad[1].rgbRed = 0x00;
            quad[1].rgbReserved = 0x00;

            quad[2].rgbBlue = 0x1F;
            quad[2].rgbGreen = 0x00;
            quad[2].rgbRed = 0x00;
            quad[2].rgbReserved = 0x00;

            return BINKSURFACE565;
        }
    }

    quad[0].rgbBlue = 0x00;
    quad[0].rgbGreen = 0x7C;
    quad[0].rgbRed = 0x00;
    quad[0].rgbReserved = 0x00;

    quad[1].rgbBlue = 0xE0;
    quad[1].rgbGreen = 0x03;
    quad[1].rgbRed = 0x00;
    quad[1].rgbReserved = 0x00;

    quad[2].rgbBlue = 0x1F;
    quad[2].rgbGreen = 0x00;
    quad[2].rgbRed = 0x00;
    quad[2].rgbReserved = 0x00;

    return BINKSURFACE555;
}