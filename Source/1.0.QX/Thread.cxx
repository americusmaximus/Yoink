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

#include "IO.hxx"
#include "Sound.hxx"
#include "Thread.hxx"

// 0x1000b440
BINKTHREADPTR RADEXPLINK CreateBinkThread(THREADACTION action, struct BINK PTR4* bink, void* value)
{
    BINKTHREADPTR thread = (BINKTHREADPTR)radmalloc(sizeof(BINKTHREAD));

    if (thread == NULL) { return NULL; }

    thread->Action = action;
    thread->Bink = bink;
    thread->Value = value;

    thread->Handle = CreateThread(NULL, 0, BinkThreadStartAction, thread, 0, &thread->ID);

    if (thread->Handle == NULL) {
        radfree(thread);
        thread = NULL;
    }

    return thread;
}

// 0x1000b4a0
DWORD WINAPI BinkThreadStartAction(LPVOID lpThreadParameter)
{
    BINKTHREADPTR params = (BINKTHREADPTR)lpThreadParameter;

    if (params->Action != NULL) { params->Action(params->Bink, params->Value); }

    return 0;
}

// 0x10008f90
void RADEXPLINK BinkThreadAction(struct BINK PTR4* bink, void* value)
{
    if (bink->threadcontrol != THREAD_CONTROL_ACTIVE) {
        bink->threadcontrol = THREAD_CONTROL_COMPLETED;
        return;
    }

    u32 start = 0;

    do {
        if (!(bink->OpenFlags & BINKNOTHREADEDIO) && bink->preloadptr == NULL) {
            bink->bio.Idle(&bink->bio);

            if (50 < ((bink->bio.CurBufUsed + 1) * 100) / (bink->bio.CurBufSize + 1)) { Sleep(5); }
        }
        else { Sleep(10); }

        const u32 end = RADTimerRead();

        if ((start < end) && (start = end + 10, bink->trackindex != DEFAULT_SOUND_TRACK)) {
            FUN_10008bb0(bink);
        }

        if (bink->threadcontrol != THREAD_CONTROL_ACTIVE) {
            bink->threadcontrol = THREAD_CONTROL_COMPLETED;
            return;
        }
    } while (TRUE);
}
