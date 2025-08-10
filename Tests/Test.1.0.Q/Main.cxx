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
#include "BinkBufferClear.hxx"
#include "BinkBufferDirectDrawCapabilitiesInitialize.hxx"
#include "BinkBufferOpen.hxx"
#include "BinkIsSoftwareCursor.hxx"
#include "BinkOpen.hxx"
#include "BinkSoundUseDirectSound.hxx"

#include <stdio.h>

#define MAX_ERROR_MESSAGE_LENGTH    256

#define EXECUTE(X)                                              \
{                                                               \
    printf(#X" ... ");                                          \
    if (Execute##X(bink, message)) { printf("OK!\r\n"); }       \
    else { printf("ERROR @ %s\r\n", message); }                 \
}

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("Usage:\r\n\t%s binkw32.dll\r\n", argv[0]);
        return -1;
    }

    HMODULE bink = LoadLibraryA(argv[1]);

    if (bink == NULL) {
        printf("Unable to load %s\r\n", argv[1]);
        return -1;
    }

    char message[MAX_ERROR_MESSAGE_LENGTH];
    memset(message, 0x00, MAX_ERROR_MESSAGE_LENGTH);

    //EXECUTE(BinkBufferAcquireColor);
    //EXECUTE(BinkBufferClear);
    //EXECUTE(BinkBufferDirectDrawCapabilitiesInitialize);
    //EXECUTE(BinkBufferOpen);

    //EXECUTE(BinkIsSoftwareCursor);

    //EXECUTE(BinkOpen);

    EXECUTE(BinkSoundUseDirectSound);

    // TODO Waveout testing

    FreeLibrary(bink);

    return 0;
}