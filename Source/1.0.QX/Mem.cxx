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

#include "Mem.hxx"

RADMEMALLOC MemAllocate;
RADMEMFREE  MemFree;

#define MAX_MEM_ITEM_COUNT  128

size_t  MemSize;                        // 0x10041b80
u32     MemCount;                       // 0x10041b84

size_t  MemSizes[MAX_MEM_ITEM_COUNT];   // 0x1004167c
void*   MemItems[MAX_MEM_ITEM_COUNT];   // 0x1004187c

#define MEM_ALIGN(size) (size + 0xF & 0xFFFFFFF0) /* x64 */

// 0x10007d50
void RADEXPLINK QueueBinkMem(void PTR4* mem, size_t size)
{
    const size_t length = MEM_ALIGN(size)
        + (((MemSize >> 5) - (size + 0xf >> 5 & 0x1F)) + 1 & 0x1F) * 0x20; // TODO

    MemSize = MemSize + length;

    MemSizes[MemCount] = length;
    MemItems[MemCount] = mem;

    MemCount = MemCount + 1;
}

// 0x10007db0
void* RADEXPLINK AllocateBinkMem(size_t size)
{
    const size_t length = MEM_ALIGN(size);

    u8* result = (u8*)radmalloc(MemSize + length);

    MemSize = 0;

    if (result != NULL) {
        u8* item = (u8*)((size_t)result + length);

        for (u32 x = 0; x < MemCount; x++) {
            MemItems[x] = item;
            item = (u8*)((size_t)item + MemSizes[x]);
        }
    }

    MemCount = 0;

    return result;
}

// 0x10008ac0
void* AllocateBinkMem(struct BINK PTR4* bink, size_t size)
{
    bink->totalmem = bink->totalmem + MemSize + size;

    return AllocateBinkMem(size);
}