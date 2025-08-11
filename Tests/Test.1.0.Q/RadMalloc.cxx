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

#include "RadMalloc.hxx"

#include <Yoink.hxx>

#include <stdio.h>

#define ALLOCATION_SIZE 128

int ExecuteRadMalloc(HMODULE bink, char* message)
{
    // Init
    RADMEMALLOC rm = (RADMEMALLOC)GetProcAddress(bink, "_radmalloc@4");
    RADMEMFREE rf = (RADMEMFREE)GetProcAddress(bink, "_radfree@4");

    void* mo = rm(ALLOCATION_SIZE);
    if (mo == NULL) {
        strcpy(message, "Unable to allocate memory.");
        return FALSE;
    }

    void* mi = radmalloc(ALLOCATION_SIZE);
    if (mo == NULL) {
        rf(mo);
        strcpy(message, "Unable to allocate memory.");
        return FALSE;
    }

    const u32 result = memcmp(mi, mo, ALLOCATION_SIZE) != 0;

    if (!result) {
        strcpy(message, "Allocated memory blocks' content match!");
    }

    rf(mo);
    radfree(mi);

    return result;
}