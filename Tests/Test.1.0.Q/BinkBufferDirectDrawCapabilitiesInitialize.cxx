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

#include "BinkBufferDirectDrawCapabilitiesInitialize.hxx"

#include <Buffer.hxx>

#define BINK_BUFFER_DIRECTDRAW_CAPABILITIES_INITIALIZE_ADDRESS(X)   ((size_t)X + (0x100056e0 - BINK_BASE_ADDRESS))

#define BINK_BUFFER_DIRECTDRAW_ADDRESS(X)                           ((size_t)X + (0x10041638 - BINK_BASE_ADDRESS))
#define BINK_BUFFER_DIRECTDRAW_SURFACE_ADDRESS(X)                   ((size_t)X + (0x1004163c - BINK_BASE_ADDRESS))

#define BINK_BUFFER_OVERLAY_CAPS_ADDRESS(X)                         ((size_t)X + (0x1004165c - BINK_BASE_ADDRESS))
#define BINK_BUFFER_BLIT_CAPS_ADDRESS(X)                            ((size_t)X + (0x10041660 - BINK_BASE_ADDRESS))

#define BINK_BUFFER_BYTES_ADDRESS(X)                                ((size_t)X + (0x1004164c - BINK_BASE_ADDRESS))

#define BINK_BUFFER_SCREEN_WIDTH_ADDRESS(X)                         ((size_t)X + (0x10041528 - BINK_BASE_ADDRESS))
#define BINK_BUFFER_SCREEN_HEIGHT_ADDRESS(X)                        ((size_t)X + (0x10041524 - BINK_BASE_ADDRESS))
#define BINK_BUFFER_SCREEN_BITS_ADDRESS(X)                          ((size_t)X + (0x10041650 - BINK_BASE_ADDRESS))

#define BINK_BUFFER_FILL_COLOR_ADDRESS(X)                           ((size_t)X + (0x1004152c - BINK_BASE_ADDRESS))

typedef void(*BINKBUFFERDIRECTDRAWCAPABILITIESINITIALIZEACTION)();

int ExecuteBinkBufferDirectDrawCapabilitiesInitialize(HMODULE bink, char* message)
{
    // Init

    LPDIRECTDRAW dd = NULL;
    LPDIRECTDRAWSURFACE surface = NULL;

    if (FAILED(DirectDrawCreate(NULL, &dd, NULL))) { return FALSE; }

    DDSURFACEDESC desc;
    radmemset(&desc, 0x00, sizeof(DDSURFACEDESC));

    if (FAILED(dd->SetCooperativeLevel(NULL, DDSCL_NORMAL))) {
        dd->Release();
        return FALSE;
    }

    desc.dwSize = sizeof(DDSURFACEDESC);
    desc.dwFlags = DDSD_CAPS;
    desc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

    if (FAILED(dd->CreateSurface(&desc, &surface, NULL))) {
        dd->Release();
        return FALSE;
    }

    // Execute

    {
        *((LPDIRECTDRAW*)BINK_BUFFER_DIRECTDRAW_ADDRESS(bink)) = dd;
        *((LPDIRECTDRAWSURFACE*)BINK_BUFFER_DIRECTDRAW_SURFACE_ADDRESS(bink)) = surface;

        BINKBUFFERDIRECTDRAWCAPABILITIESINITIALIZEACTION action =
            (BINKBUFFERDIRECTDRAWCAPABILITIESINITIALIZEACTION)BINK_BUFFER_DIRECTDRAW_CAPABILITIES_INITIALIZE_ADDRESS(bink);

        action();
    }

    {
        BufferDirectDraw = dd;
        BufferDirectDrawSurface = surface;

        BinkBufferDirectDrawCapabilitiesInitialize();
    }

    int result = TRUE;

    // Compare
    {
        const u32 overlay = *(u32*)BINK_BUFFER_OVERLAY_CAPS_ADDRESS(bink);
        if (overlay != BufferOverlayCaps) { result = FALSE; }

        const u32 blit = *(u32*)BINK_BUFFER_BLIT_CAPS_ADDRESS(bink);
        if (blit != BufferBlitCaps) { result = FALSE; }

        const u32 bytes = *(u32*)BINK_BUFFER_BYTES_ADDRESS(bink);
        if (bytes != BufferBytes) { result = FALSE; }

        const s32 w = *(s32*)BINK_BUFFER_SCREEN_WIDTH_ADDRESS(bink);
        if (w != BufferScreenWidth) { result = FALSE; }

        const s32 h = *(s32*)BINK_BUFFER_SCREEN_HEIGHT_ADDRESS(bink);
        if (h != BufferScreenHeight) { result = FALSE; }

        const u32 bits = *(u32*)BINK_BUFFER_SCREEN_BITS_ADDRESS(bink);
        if (bits != BufferScreenBits) { result = FALSE; }

        const u32 color = *(u32*)BINK_BUFFER_FILL_COLOR_ADDRESS(bink);
        if (color != BufferFillColor) { result = FALSE; }
    }

    // Clean
    {
        *((LPDIRECTDRAW*)BINK_BUFFER_DIRECTDRAW_ADDRESS(bink)) = NULL;
        *((LPDIRECTDRAWSURFACE*)BINK_BUFFER_DIRECTDRAW_SURFACE_ADDRESS(bink)) = NULL;

        *(u32*)BINK_BUFFER_OVERLAY_CAPS_ADDRESS(bink) = 0;
        *(u32*)BINK_BUFFER_BLIT_CAPS_ADDRESS(bink) = 0;

        *(u32*)BINK_BUFFER_BYTES_ADDRESS(bink) = 0;
        *(s32*)BINK_BUFFER_SCREEN_WIDTH_ADDRESS(bink) = 0;
        *(s32*)BINK_BUFFER_SCREEN_HEIGHT_ADDRESS(bink) = 0;
        *(u32*)BINK_BUFFER_SCREEN_BITS_ADDRESS(bink) = 0;
        *(u32*)BINK_BUFFER_FILL_COLOR_ADDRESS(bink) = 0;
    }

    {
        BufferDirectDraw = NULL;
        BufferDirectDrawSurface = NULL;

        BufferOverlayCaps = 0;
        BufferBlitCaps = 0;

        BufferBytes = 0;
        BufferScreenWidth = 0;
        BufferScreenHeight = 0;
        BufferScreenBits = 0;
        BufferFillColor = 0;
    }

    surface->Release();
    dd->Release();

    return result;
}