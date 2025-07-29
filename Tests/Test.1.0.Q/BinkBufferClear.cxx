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

#include "BinkBufferClear.hxx"

#include <Buffer.hxx>

#include <stdlib.h>

#define BINK_BUFFER_DEPTH               32

#define ALIGN_MEMORY_SIZE(X)            ((X + 0xF) & 0xFFFFFFF0)

#define BINK_BUFFER_CLEAR_ADDRESS(X)    ((size_t)X + (0x10006ea0 - BINK_BASE_ADDRESS))
#define BINK_BUFFER_DEPTH_ADDRESS(X)    ((size_t)X + (0x1004164c - BINK_BASE_ADDRESS))

typedef void(*BINKBUFFERCLEARACTION)(void* buf, u32 type, u32 pitch, u32 width, u32 height);

#define MAX_RESOLUTION_COUNT            3
#define MAX_RESOLUTION_DIMENTION_COUNT  2

const static u32
resolutions[MAX_RESOLUTION_COUNT][MAX_RESOLUTION_DIMENTION_COUNT] =
{
    { 640, 480 }, { 800, 600 }, { 1024, 768 }
};

u32 AcquirePitch(u32 type, u32 width) {
    switch (type)
    {
    case BINKSURFACE8P: { return BufferBytes * width; }
    case BINKSURFACE555:
    case BINKSURFACE565:
    case BINKSURFACE655: {return BufferBytes * width * 2; }
    case BINKSURFACE24:
    case BINKSURFACE24R: { return BufferBytes * width * 3; }
    case BINKSURFACEYUY2:
    case BINKSURFACEUYVY: { return width * 2; }
    case BINKSURFACEYV12: { return width; }
    }

    return BufferBytes * width * 4;
}

void* AcquireMemory(u32 type, u32 size) {
    void* ptr = malloc(size);

    if (ptr == NULL) { return NULL; }

    memset(ptr, 0xFF, size);

    return ptr;
}

u32 Execute(u32 type, BINKBUFFERCLEARACTION bnk, BINKBUFFERCLEARACTION imp) {
    for (s32 i = 0; i < MAX_RESOLUTION_COUNT; i++) {
        const u32 w = resolutions[i][0];
        const u32 h = resolutions[i][1];
        const u32 pitch = AcquirePitch(type, w);

        const u32 size = type == BINKSURFACEYV12
            ? 2 * ALIGN_MEMORY_SIZE(pitch * h) : ALIGN_MEMORY_SIZE(pitch * h);

        void* bo = AcquireMemory(type, size);
        if (bo == NULL) { return FALSE; }

        void* bi = AcquireMemory(type, size);
        if (bi == NULL) { free(bo); return FALSE; }

        bnk(bo, type, pitch, w, h);
        imp(bi, type, pitch, w, h);

        const u32 success = memcmp(bo, bi, size) == 0;

        free(bo);
        free(bi);

        if (!success) { return FALSE; }
    }

    return TRUE;
}

#define TEST(T) if (!Execute(T, action, BinkBufferClear)) { strcpy(message, #T); return FALSE; }

int ExecuteBinkBufferClear(HMODULE bink, char* message) {
    BINKBUFFERCLEARACTION action =
        (BINKBUFFERCLEARACTION)BINK_BUFFER_CLEAR_ADDRESS(bink);

    // Init
    {
        *(u32*)BINK_BUFFER_DEPTH_ADDRESS(bink) = BINK_BUFFER_DEPTH;
        BufferBytes = BINK_BUFFER_DEPTH;
    }

    TEST(BINKSURFACE8P);
    TEST(BINKSURFACE24);
    TEST(BINKSURFACE24R);
    TEST(BINKSURFACE32);
    TEST(BINKSURFACE32R);
    TEST(BINKSURFACE32A);
    TEST(BINKSURFACE32RA);
    TEST(BINKSURFACE4444);
    TEST(BINKSURFACE5551);
    TEST(BINKSURFACE555);
    TEST(BINKSURFACE565);
    TEST(BINKSURFACE655);
    TEST(BINKSURFACE664);
    TEST(BINKSURFACEYUY2);
    TEST(BINKSURFACEUYVY);
    TEST(BINKSURFACEYV12);

    // Clean
    {
        *(u32*)BINK_BUFFER_DEPTH_ADDRESS(bink) = 0;
        BufferBytes = 0;
    }

    return TRUE;
}