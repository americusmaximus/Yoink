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

typedef struct BINKWAVEOUTDATA
{
    u32         Volume;         // 0x0
    u32         Pan;            // 0x4
    s32         Unk0x8;         // 0x8 // TODO
    u32         IsPaused;       // 0xC
    s32         LockChunk;      // 0x10
    s32         WriteChunk;     // 0x14
    u32         WriteLength;    // 0x18
    s32         Unk0x1C;        // 0x1C // TODO
    u32         Length;         // 0x20
    HWAVEOUT    WaveOut;        // 0x24
    LPWAVEHDR*  Headers;        // 0x28
    void*       Mem;            // 0x2C
} BINKWAVEOUTDATA, * HBINKWAVEOUTDATA;

s32 RADLINK BinkWaveOutOpen(struct BINKSND PTR4* snd, u32 freq, s32 bits, s32 chans, u32 flags, HBINK bink);