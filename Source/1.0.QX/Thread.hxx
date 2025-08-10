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

#define THREAD_CONTROL_NONE         0
#define THREAD_CONTROL_ACTIVE       1
#define THREAD_CONTROL_COMPLETED    2

typedef void(RADEXPLINK* THREADACTION)(struct BINK PTR4* bink, void* value);

typedef struct BINKTHREAD
{
    THREADACTION    Action;
    HBINK           Bink;
    void*           Value;
    HANDLE          Handle;
    DWORD           ID;
} BINKTHREAD, * HBINKTHREAD;

HBINKTHREAD RADEXPLINK BinkCreateThread(THREADACTION action, struct BINK PTR4* bink, void* value);
DWORD WINAPI BinkThreadStartAction(LPVOID tps);
void RADEXPLINK BinkThreadAction(struct BINK PTR4* bink, void* value);
void RADEXPLINK BinkThreadClose(HBINKTHREAD thread);