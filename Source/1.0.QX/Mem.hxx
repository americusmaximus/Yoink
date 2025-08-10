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

// Memory allocation and freeing approach:
//
// 1. Allocate requested size + 64 bytes.
// 2. Align memory by 32 bytes (via & 0x1F)
// 3. Store offset to allocate memoty into -1 byte offset to the return pointer.
// 4. Store the type of memory allocation into -2 byte offset to the return pointer.
// 5. Store the memory freeing function pointer into -8 byte, if needed.

#define INTERNAL_MEMORY_ALLOCATION              0x00
#define EXTERNAL_MEMORY_ALLOCATION              0x03

#define MEMORY_ALLOCATION_EXTRA_SPACE           0x40
#define MEMORY_ALLOCATION_MASK                  0x1F

#define INVALID_MEMORY_ALLOCATION_SIZE          (-1)
#define INVALID_MEMORY_ALLOCATION_VALUE         (void*)(-1)

#define GET_MEMORY_OFFSET_VALUE(X)              (*(u8*)((size_t)X - 1))
#define SET_MEMORY_OFFSET_VALUE(X, V)           GET_MEMORY_OFFSET_VALUE(X) = V

#define GET_MEMORY_TYPE_VALUE(X)                (*(u8*)((size_t)X - 2))
#define SET_MEMORY_TYPE_VALUE(X, V)             GET_MEMORY_TYPE_VALUE(X) = V

#define GET_MEMORY_FREE_FUNCTION_VALUE(X)       (*(RADMEMFREE*)((size_t)X - 8))
#define SET_MEMORY_FREE_FUNCTION_VALUE(X, V)    GET_MEMORY_FREE_FUNCTION_VALUE(X) = V

#define CALCULATE_MEMORY_OFFSET_VALUE(X)        (MEMORY_ALLOCATION_EXTRA_SPACE - (((size_t)X) & MEMORY_ALLOCATION_MASK))
#define CALCULATE_MEMORY_OFFSET_POINTER(X)      ((void*)((size_t)X - (size_t)GET_MEMORY_OFFSET_VALUE(X)))

extern RADMEMALLOC  MemAllocate;    // 0x10041418
extern RADMEMFREE   MemFree;        // 0x1004141c

void RADEXPLINK BinkQueueMem(void PTR4* mem, size_t size);
void* BinkAllocateMem(struct BINK PTR4* bink, size_t size);
void* RADEXPLINK BinkAllocateMem(size_t size);


// TODO TEMP
#define MAX_MEM_ITEM_COUNT  128
extern size_t  MemSize;                        // 0x10041b80
extern u32     MemCount;                       // 0x10041b84

extern size_t  MemSizes[MAX_MEM_ITEM_COUNT];   // 0x1004167c
extern void* MemItems[MAX_MEM_ITEM_COUNT];   // 0x1004187c