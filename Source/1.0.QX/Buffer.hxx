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

#pragma once

#include "DirectDraw.hxx"

#define MAX_BUFFER_ERROR_LENGTH         256
#define MAX_BUFFER_DESCRIPTION_LENGTH   256

#define BINKBUFFERMOVEMODE_NONE         0
#define BINKBUFFERMOVEMODE_HIDE         1
#define BINKBUFFERMOVEMODE_SHOW         2

#define BINK_BUFFER_BITMAP_COLOR_COUNT      256
#define BINK_BUFFER_BITMAP_MEMORY_SIZE      (sizeof(BITMAPINFO) + BINK_BUFFER_BITMAP_COLOR_COUNT * sizeof(RGBQUAD)) 

typedef HRESULT(WINAPI *DIRECTDRAWCREATEACTION)(GUID FAR* lpGUID, LPDIRECTDRAW FAR* lplpDD, IUnknown FAR* pUnkOuter);

extern char                     BufferDescription[MAX_BUFFER_ERROR_LENGTH]; // 0x10041424
extern s32                      BufferScreenHeight;                         // 0x10041524
extern s32                      BufferScreenWidth;                          // 0x10041528
extern DWORD                    BufferFillColor;                            // 0x1004152c
extern char                     BufferError[MAX_BUFFER_ERROR_LENGTH];       // 0x10041530

extern s32                      BufferCount;                                // 0x10041634
extern LPDIRECTDRAW             BufferDirectDraw;                           // 0x10041638
extern LPDIRECTDRAWSURFACE      BufferDirectDrawSurface;                    // 0x1004163c
extern u32                      BufferWidth;                                // 0x10041640
extern u32                      BufferHeight;                               // 0x10041644
extern u32                      BufferBits;                                 // 0x10041648
extern u32                      BufferBytes;                                // 0x1004164c
extern u32                      BufferScreenBits;                           // 0x10041650
extern DIRECTDRAWCREATEACTION   BufferDirectDrawCreate;                     // 0x10041654
extern HMODULE                  BufferDirectDrawModule;                     // 0x10041658
extern u32                      BufferOverlayCaps;                          // 0x1004165c
extern u32                      BufferBlitCaps;                             // 0x10041660
extern u32                      BufferIsDeviceContext;                      // 0x10041664
extern s32                      BufferDeviceContextCount;                   // 0x10041668

LPDIRECTDRAWSURFACE BinkBufferDirectDrawOffscreenSurfaceCreate(u32 format, u32 width, u32 height, u32 type);
LPDIRECTDRAWSURFACE BinkBufferDirectDrawOverlaySurfaceCreate(u32 format, u32 width, u32 height, u32 type);
u32 BinkBufferAcquireColor(u32 value, u32 mask);
u32 BinkBufferDeviceContextInitialize(HWND wnd, u32 fullscreen);
u32 BinkBufferDeviceContextSurfaceType(HDC hdc, LPRGBQUAD quad);
void BinkBufferClear(void* buf, u32 type, u32 pitch, u32 width, u32 height);
void BinkBufferDeviceContextRelease();
void BinkBufferDirectDrawCapabilitiesInitialize();
void BinkBufferDirectDrawInitialize(HWND wnd, u32 fullscreen);
void BinkBufferDirectDrawLoad();
void BinkBufferDirectDrawRelease();
void BinkBufferMove(HBINKBUFFER buf, u32 mode);