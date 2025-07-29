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

#include "BinkIsSoftwareCursor.hxx"

#include <Cursor.hxx>
#include <DirectDraw.hxx>

#define BINK_IS_SOFTWARE_CURSOR_ADDRESS(X)      ((size_t)X + (0x10005980 - BINK_BASE_ADDRESS))

#define BINK_CURSOR_POINTER_ADDRESS(X)          ((size_t)X + (0x1004166c - BINK_BASE_ADDRESS))
#define BINK_CURSOR_SURFACE_ADDRESS(X)          ((size_t)X + (0x10041670 - BINK_BASE_ADDRESS))
#define BINK_CURSOR_BITS_ADDRESS(X)             ((size_t)X + (0x10041674 - BINK_BASE_ADDRESS))
#define BINK_CURSOR_IS_SOFTWARE_ADDRESS(X)      ((size_t)X + (0x10041420 - BINK_BASE_ADDRESS))

typedef s32(RADEXPLINK *BINKISSOFTWARECURSORACTION)(void PTR4* lpDDSP, HCURSOR cur);

int ExecuteBinkIsSoftwareCursor(HMODULE bink, char* message)
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

    {
        *(HCURSOR*)BINK_CURSOR_POINTER_ADDRESS(bink) = NULL;
        *(void**)BINK_CURSOR_SURFACE_ADDRESS(bink) = NULL;

        *(u32*)BINK_CURSOR_BITS_ADDRESS(bink) = 0;

        *(u32*)BINK_CURSOR_IS_SOFTWARE_ADDRESS(bink) = FALSE;
    }

    {
        CursorPointer = NULL;
        CursorSurface = NULL;

        CursorBits = 0;

        CursorIsSoftware = FALSE;
    }

    // Execute

    BINKISSOFTWARECURSORACTION action =
        (BINKISSOFTWARECURSORACTION)BINK_IS_SOFTWARE_CURSOR_ADDRESS(bink);

    // Compare
    int result = action(surface, NULL) == BinkIsSoftwareCursor(surface, NULL);

    result &= *(HCURSOR*)BINK_CURSOR_POINTER_ADDRESS(bink) == CursorPointer;
    result &= *(void**)BINK_CURSOR_SURFACE_ADDRESS(bink) == CursorSurface;
    result &= *(u32*)BINK_CURSOR_BITS_ADDRESS(bink) == CursorBits;
    result &= *(u32*)BINK_CURSOR_IS_SOFTWARE_ADDRESS(bink) == CursorIsSoftware;


    // Clean
    {
        *(HCURSOR*)BINK_CURSOR_POINTER_ADDRESS(bink) = NULL;
        *(void**)BINK_CURSOR_SURFACE_ADDRESS(bink) = NULL;

        *(u32*)BINK_CURSOR_BITS_ADDRESS(bink) = 0;

        *(u32*)BINK_CURSOR_IS_SOFTWARE_ADDRESS(bink) = FALSE;
    }

    {
        CursorPointer = NULL;
        CursorSurface = NULL;

        CursorBits = 0;

        CursorIsSoftware = FALSE;
    }

    surface->Release();
    dd->Release();

    return result;
}