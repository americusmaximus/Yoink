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

#include "DirectSound.hxx"
#include "WaveOut.hxx"

#define VOLUME_NONE     0
#define VOLUME_NORMAL   32767
#define VOLUME_MAX      65535

#define PAN_LEFT        0
#define PAN_CENTER      32767
#define PAN_RIGHT       65537

// TODO check all +1, +2, -1 values of volume and pan

#define DEFAULT_SOUND_CHUNK_INDEX               (-1)

#define SOUND_TRACK_TYPE_FREQUENCY(X)           (X & 0xFFFF)
#define SOUND_TRACK_TYPE_BITS(X)                (((X >> 30) & 1) * 8 + 8)
#define SOUND_TRACK_TYPE_CHANNELS(X)            (((X >> 29) & 1) + 1)
#define SOUND_TRACK_TYPE_CONVERT_TO_8BITS(X)    (((X >> 27) & 8) == 0)
#define SOUND_TRACK_TYPE_IS_ACTIVE(X)           (X & 0x80000000)

#define SOUND_BUFFER_SIZE(X)                    ((X + 0xFF) & 0xFFFFFF00)

extern s32              SoundTrack;         // 0x1003a07c

extern BINKSNDSYSOPEN   SoundOpen;          // 0x10041b88
extern BINKSNDOPEN      SoundSystem;        // 0x10041b8c
extern u32              SoundCounter;       // 0x10041b90

typedef struct BINKSNDCOMP
{
    // TODO
} BINKSNDCOMP, * HBINKSNDCOMP;

u32 BinkSoundFrameClear(HBINK bnk);
void RADEXPLINK BinkSoundCompressorRelease(void* comp);
void RADEXPLINK BinkSoundFrameFill(HBINK bink);
u8* BinkSoundConvertTo8Bits(u8* dst, u16* src, size_t size);

HBINKSNDCOMP BinkSoundCompressionInitialize(u32 freq, u32 chans, u32 type);