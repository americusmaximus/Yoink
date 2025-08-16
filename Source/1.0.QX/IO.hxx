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

#define DEFAULT_IO_SIZE (-1)

typedef struct BINKIOHEADER {
    u32 Magic;
    u32 Size;           // File size, excluding the first 8 bytes
    u32 Frames;         // Number of frames
    u32 MaxFrameSize;   // Largest frame size in bytes
    u32 InternalFrames; // Number of internal frames
    u32 Width;          // Video width
    u32 Height;         // Height
    u32 Dividend;       // Video frames per second dividend
    u32 Divider;        // Video frame per second divider
    u32 Options;        // Flags
    u32 Tracks;         // Number of audio tracks
} BINKIOHEADER, * BINKIOHEADERPTR;

typedef struct BINKIODATA
{
    HANDLE          Handle;             // 0x0
    s32             HeaderOffset;       // 0x4 // TODO Name
    s32             DataOffset;         // 0x8 // TODO Name
    void*           Buffer;             // 0xC // TODO Name
    s32             BufferSize;         // 0x10 // TODO Name
    volatile s32    ReadCount;          // 0x14
    volatile s32    Unk0x18;            // 0x18 // TODO Some sort of lock
    void*           Unk0x1C;            // 0x1C // TODO Buffer
    void*           Unk0x20;            // 0x20 // TODO Buffer
    void*           Unk0x24;            // 0x24 // TODO Buffer
    u32             IsExternal;         // 0x28
    u32             FilePointer;        // 0x2c
    s32             FileSize;           // 0x30

    // This value is set in bytes per second,
    // use 150,000 for a 1xCD, 300,000 for a 2xCD, and 600,000 for a 4xCD. 
    u32             SimulateRate;       // 0x34
    u32             SimulateDelay;      // 0x38
    s32             Unk0x3C;            // 0x3C // TODO
} BINKIODATA, * BINKIODATAPTR;

#define ASIODATA(X) ((BINKIODATAPTR)(X->iodata))

extern u32          IOSize;     // 0x1003a074
extern BINKIOOPEN   IO;         // 0x10041b94

s32 RADLINK BinkOpenFile(struct BINKIO PTR4* io, const char PTR4* name, u32 flags);
u32 RADLINK BinkGetFileBufferSize(struct BINKIO PTR4* io, u32 size);
u32 RADLINK BinkReadFile(struct BINKIO PTR4* io);
u32 RADLINK BinkReadFileFrame(struct BINKIO PTR4* io, u32 frame, s32 origofs, void PTR4* dest, u32 size);
u32 RADLINK BinkReadFileHeader(struct BINKIO PTR4* io, s32 offset, void PTR4* dest, u32 size);
void RADLINK BinkCloseFile(struct BINKIO PTR4* io);
void RADLINK BinkSetFileInfo(struct BINKIO PTR4* io, void PTR4* buf, u32 size, u32 fileSize, u32 simulate);

void BinkReadFileSimulate(struct BINKIO PTR4* io, u32 size, u32 time);