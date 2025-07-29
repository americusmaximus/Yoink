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

#include "BinkBufferAcquireColor.hxx"

#include <Buffer.hxx>

#define BINK_BUFFER_ACQUIRE_COLOR_ADDRESS(X)    ((size_t)X + (0x100058d0 - BINK_BASE_ADDRESS))

typedef u32(*BINKBUFFERACQUIRECOLORACTION)(u32 value, u32 mask);

u32 Execute(u32 value, u32 mask, BINKBUFFERACQUIRECOLORACTION bnk, BINKBUFFERACQUIRECOLORACTION imp) {
    return bnk(value, mask) == imp(value, mask);
}

#define TEST(V, M) if (!Execute(V, M, action, BinkBufferAcquireColor)) { strcpy(message, #V":"#M); return FALSE; }

int ExecuteBinkBufferAcquireColor(HMODULE bink, char* message) {
    BINKBUFFERACQUIRECOLORACTION action =
        (BINKBUFFERACQUIRECOLORACTION)BINK_BUFFER_ACQUIRE_COLOR_ADDRESS(bink);

    TEST(1, 0xFF000000);
    TEST(1, 0x00FF0000);
    TEST(1, 0x0000FF00);
    TEST(1, 0x000000FF);
    TEST(1, 0x0000F800);
    TEST(1, 0x00007C00);
    TEST(1, 0x000003E0);
    TEST(1, 0x000000F0);
    TEST(1, 0x0000001F);
    TEST(1, 0x0000000F);

    return TRUE;
}