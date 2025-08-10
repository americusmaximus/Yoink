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

/*
https://wiki.multimedia.cx/index.php/Bink_Container

bytes 0-2     file signature ('BIK', or 'KB2' for Bink Video 2)
byte 3        Bink Video codec revision (0x62, 0x64, 0x66, 0x67, 0x68, 0x69; b,d,f,g,h,i respectively)
              Bink Video 2 codec revision ('a', 'd', 'f', 'g', 'h', 'i')
bytes 4-7     file size not including the first 8 bytes
bytes 8-11    number of frames
bytes 12-15   largest frame size in bytes
bytes 16-19   number of internal frames
bytes 20-23   video width (less than or equal to 32767)
bytes 24-27   video height (less than or equal to 32767)
bytes 28-31   video frames per second dividend
bytes 32-35   video frames per second divider
bytes 36-39   video flags
                 bits 28-31: width and height scaling
                   1 = 2x height doubled
                   2 = 2x height interlaced
                   3 = 2x width doubled
                   4 = 2x width and height-doubled
                   5 = 2x width and height-interlaced
                 bit 20: has alpha plane
                 bit 17: grayscale
bytes 40-43   number of audio tracks (less than or equal to 256)

for each audio track
   two bytes   unknown
   two bytes   audio channels (1 or 2). Not authoritative, see flags below.

for each audio track
   two bytes   audio sample rate (Hz)
   two bytes   flags
                 bit 15: unknown (observed in some samples)
                 bit 14: unknown (observed in some samples)
                 bit 13: stereo flag
                 bit 12: Bink Audio algorithm
                   1 = use Bink Audio DCT
                   0 = use Bink Audio FFT

for each audio track
   four bytes  audio track ID

*/

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

s32 RADLINK BinkOpenFile(struct BINKIO PTR4* io, const char PTR4* name, u32 flags);
u32 RADLINK BinkGetFileBufferSize(struct BINKIO PTR4* io, u32 size);
u32 RADLINK BinkReadFile(struct BINKIO PTR4* io);
u32 RADLINK BinkReadFileFrame(struct BINKIO PTR4* io, u32 frame, s32 origofs, void PTR4* dest, u32 size);
u32 RADLINK BinkReadFileHeader(struct BINKIO PTR4* io, s32 offset, void PTR4* dest, u32 size);
void RADLINK BinkCloseFile(struct BINKIO PTR4* io);
void RADLINK BinkSetFileInfo(struct BINKIO PTR4* io, void PTR4* buf, u32 size, u32 fileSize, u32 simulate);

void BinkReadFileSimulate(struct BINKIO PTR4* io, u32 size, u32 time);

extern u32          IOSize;     // 0x1003a074
extern BINKIOOPEN   IO;         // 0x10041b94