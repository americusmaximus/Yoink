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

#include "Yoink.hxx"

#define DIRECTSOUND_VERSION 0x0700
#include <dsound.h>

#define DEFAULT_DIRECT_SOUND_INSTANCE   ((LPDIRECTSOUND)0xFFFFFFFF) /* x64 */

typedef HRESULT(WINAPI* DIRECTSOUNDCREATEACTION)(LPCGUID pcGuidDevice, LPDIRECTSOUND* ppDS, LPUNKNOWN pUnkOuter);

extern LPDIRECTSOUND            DirectSound;                        // 0x1003e81c

extern HWND                     DirectSoundActiveWindow;            // 0x1004f1b4
extern u32                      DirectSoundIsManaged;               // 0x1004f1b8

extern HMODULE                  DirectSoundModule;                  // 0x1004f1c0
extern DIRECTSOUNDCREATEACTION  DirectSoundCreateAction;            // 0x1004f1c4
extern u32                      DirectSoundIsCooperativeLevelSet;   // 0x1004f1c8
extern LPDIRECTSOUNDBUFFER      DirectSoundBuffer;                  // 0x1004f1cc
extern u32                      DirectSoundIsEmulated;              // 0x1004f1d0
extern u32                      DirectSoundCount;                   // 0x1004f1d4

HWND BinkDirectSoundAcquireActiveWindow();
u32 BinkDirectSoundInitialize();
void BinkDirectSoundRelease();
s32 RADLINK BinkDirectSoundOpen(struct BINKSND PTR4* snd, u32 freq, s32 bits, s32 chans, u32 flags, HBINK bink);