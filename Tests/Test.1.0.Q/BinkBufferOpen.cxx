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

#include "BinkBufferOpen.hxx"

#include <Yoink.hxx>

#define WINDOW_CLASS_NAME   "BinkBufferOpen"
#define WINDOW_TITLE_NAME   "BinkBufferOpen"

#define WINDOW_WIDTH        640
#define WINDOW_HEIGHT       480

typedef char* (RADEXPLINK* BINKBUFFERGETERRORACTION)();
typedef HBINKBUFFER(RADEXPLINK *BINKBUFFEROPENACTION)(HWND wnd, u32 width, u32 height, u32 bufferflags);
typedef void(RADEXPLINK* BINKBUFFERCLOSEACTION)(HBINKBUFFER buf);

#define BINK_BUFFER_OPEN_ADDRESS(X)     ((size_t)X + (0x10006050 - BINK_BASE_ADDRESS))

static int BinkBufferCompare(HBINKBUFFER a, char* ea, HBINKBUFFER b, char* eb)
{
    if ((a == NULL && b != NULL) || (a != NULL && b == NULL)) { return FALSE; }

    if (a == NULL && b == NULL) { return strcmp(ea, eb) == 0; }

    int result = TRUE;                                              // Example

    result &= a->Width == b->Width;                                 // 640
    result &= a->Height == b->Height;                               // 480
    result &= a->WindowWidth == b->WindowWidth;                     // 660
    result &= a->WindowHeight == b->WindowHeight;                   // 523
    result &= a->SurfaceType == b->SurfaceType;                     // 0x84000003

    result &= (a->Buffer == NULL && b->Buffer == NULL)
        || (a->Buffer != NULL && b->Buffer != NULL);                // NULL

    result &= a->BufferPitch == b->BufferPitch;                     // 0
    result &= a->ClientOffsetX == b->ClientOffsetX;                 // 10
    result &= a->ClientOffsetY == b->ClientOffsetY;                 // 33
    result &= a->ScreenWidth == b->ScreenWidth;                     // 1920
    result &= a->ScreenHeight == b->ScreenHeight;                   // 1080
    result &= a->ScreenDepth == b->ScreenDepth;                     // 32
    result &= a->ExtraWindowWidth == b->ExtraWindowWidth;           // 20
    result &= a->ExtraWindowHeight == b->ExtraWindowHeight;         // 43
    result &= a->ScaleFlags == b->ScaleFlags;                       // 0x0
    result &= a->StretchWidth == b->StretchWidth;                   // 640
    result &= a->StretchHeight == b->StretchHeight;                 // 480

    result &= a->surface == b->surface;                             // NULL
    result &= (a->ddsurface == NULL && b->ddsurface == NULL)
        || (a->ddsurface != NULL && b->ddsurface != NULL);          // NULL
    result &= a->ddclipper == a->ddclipper;                         // NULL
    result &= a->destx == b->destx;                                 // 0
    result &= a->desty == b->desty;                                 // 0
    result &= a->wndx == b->wndx;                                   // 0
    result &= a->wndy == b->wndy;                                   // 0
    result &= a->wnd == b->wnd;                                     // 0
    result &= a->ddoverlay == b->ddoverlay;                         // 0
    result &= a->ddoffscreen == b->ddoffscreen;                     // 0
    result &= a->lastovershow == b->lastovershow;                   // 0

    // Note:
    // Soft cursor is being detected...
    
    //result &= a->issoftcur == b->issoftcur;                       // 0

    result &= a->cursorcount == b->cursorcount;                     // 0
    result &= a->buffertop == b->buffertop;                         // NULL
    result &= a->type == b->type;                                   // 0
    result &= a->noclipping == b->noclipping;                       // 0

    result &= a->loadeddd == b->loadeddd;                           // 0
    result &= a->loadedwin == b->loadedwin;                         // 0

    result &= (a->dibh == NULL && b->dibh == NULL)
        || (a->dibh != NULL && b->dibh != NULL);                    // NULL
    result &= (a->dibbuffer == NULL && b->dibbuffer == NULL)
        || (a->dibbuffer != NULL && b->dibbuffer != NULL);          // NULL
    
    result &= a->dibpitch == b->dibpitch;                           // 0
    
    result &= (a->dibinfo == NULL && b->dibinfo == NULL)
        || (a->dibinfo != NULL && b->dibinfo != NULL);              // NULL
    result &= (a->dibdc == NULL && b->dibdc == NULL)
        || (a->dibdc != NULL && b->dibdc != NULL);                  // NULL
    result &= (a->diboldbitmap == NULL && b->diboldbitmap == NULL)
        || (a->diboldbitmap != NULL && b->diboldbitmap != NULL);    // NULL

    result &= strcmp(ea, eb) == 0;

    return result;
}

static u32 Execute(u32 type, HWND wnd,
    BINKBUFFEROPENACTION bnko, BINKBUFFEROPENACTION impo,
    BINKBUFFERCLOSEACTION bnkc, BINKBUFFERCLOSEACTION impc,
    BINKBUFFERGETERRORACTION bnke, BINKBUFFERGETERRORACTION impe)
{
    HBINKBUFFER bb = bnko(wnd, WINDOW_WIDTH, WINDOW_HEIGHT, type);
    HBINKBUFFER bi = impo(wnd, WINDOW_WIDTH, WINDOW_HEIGHT, type);

    const u32 result = BinkBufferCompare(bb, bnke(), bi, impe());

    bnkc(bb);
    impc(bi);

    return result;
}

#define TEST(T, W) if (!Execute(T, W, open, BinkBufferOpen, close, BinkBufferClose, error, BinkBufferGetError)) { strcpy(message, #T); return FALSE; }

int ExecuteBinkBufferOpen(HMODULE bink, char* message)
{
    WNDCLASSA wndc;
    memset(&wndc, 0x00, sizeof(WNDCLASSA));

    HINSTANCE inst = GetModuleHandleA(NULL);

    wndc.style = CS_DBLCLKS;
    wndc.lpfnWndProc = DefWindowProcA;
    wndc.hInstance = inst;
    wndc.hbrBackground = (HBRUSH)COLOR_WINDOW;
    wndc.lpszClassName = WINDOW_CLASS_NAME;

    RegisterClassA(&wndc);

    HWND wnd = CreateWindowExA(WS_EX_OVERLAPPEDWINDOW,
        WINDOW_CLASS_NAME, WINDOW_TITLE_NAME,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 640, 480,
        NULL, NULL, inst, NULL);

    if (wnd == NULL) {
        strcpy(message, "Unable to create a window.\r\n");
        return FALSE;
    }

    BINKBUFFEROPENACTION open =
        (BINKBUFFEROPENACTION)GetProcAddress(bink, "_BinkBufferOpen@16");
    BINKBUFFERCLOSEACTION close =
        (BINKBUFFERCLOSEACTION)GetProcAddress(bink, "_BinkBufferClose@4");
    BINKBUFFERGETERRORACTION error =
        (BINKBUFFERGETERRORACTION)GetProcAddress(bink, "_BinkBufferGetError@0");

    TEST(BINKBUFFERAUTO, wnd);
    TEST(BINKBUFFERPRIMARY, wnd);
    TEST(BINKBUFFERDIBSECTION, wnd);
    TEST(BINKBUFFERYV12OVERLAY, wnd);
    TEST(BINKBUFFERYUY2OVERLAY, wnd);
    TEST(BINKBUFFERUYVYOVERLAY, wnd);
    TEST(BINKBUFFERYV12OFFSCREEN, wnd);
    TEST(BINKBUFFERYUY2OFFSCREEN, wnd);
    TEST(BINKBUFFERUYVYOFFSCREEN, wnd);
    TEST(BINKBUFFERRGBOFFSCREENVIDEO, wnd);
    TEST(BINKBUFFERRGBOFFSCREENSYSTEM, wnd);
    TEST(BINKBUFFERLAST + 1, wnd);

    CloseWindow(wnd);
    DestroyWindow(wnd);
    UnregisterClassA(WINDOW_CLASS_NAME, inst);

    return TRUE;
}