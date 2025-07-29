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

#include "Cursor.hxx"

s32         CursorIsSoftware;

s32         CursorHeight;

HCURSOR     CursorPointer;
void*       CursorSurface;
u32         CursorBits;
s32         CursorWidth;

// 0x10005900
LRESULT WINAPI BinkCursorWindowProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg)
    {
    case WM_DESTROY: { return 0; }
    case WM_PAINT: {
        PAINTSTRUCT paint;

        BeginPaint(wnd, &paint);
        EndPaint(wnd, &paint);

        return 0;
    }
    case WM_ERASEBKGND: { return 1; }
    }

    return DefWindowProcA(wnd, msg, wp & 0xFFFF, lp);
}