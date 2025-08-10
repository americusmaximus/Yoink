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

#include "Frame.hxx"
#include "Sound.hxx"

// 0x10008af0
void BinkFrameRead(BINK* bnk, u32 frame)
{
    if (frame != 0) { frame = frame - 1; }

    if (bnk->trackindex != BINKNOSOUND) { BinkSoundFrameFill(bnk); }

    bnk->LastFrameNum = bnk->FrameNum;

    if (frame == 0) {
        if (bnk->soundon) {
            BinkSetSoundOnOff(bnk, FALSE);
            BinkSetSoundOnOff(bnk, TRUE);
        }

        bnk->startsynctime = 0;
    }

    if (bnk->preloadptr == NULL) {
        bnk->bio.ReadFrame(&bnk->bio, frame, ALIGN_FRAME_OFFSETS(bnk->frameoffsets[frame]),
            bnk->compframe, ALIGN_FRAME_OFFSETS(bnk->frameoffsets[frame + 1]) - ALIGN_FRAME_OFFSETS(bnk->frameoffsets[frame]));
    }
    else {
        bnk->compframe =
            (void*)((size_t)bnk->preloadptr + ALIGN_FRAME_OFFSETS(bnk->frameoffsets[frame]) - ALIGN_FRAME_OFFSETS(bnk->frameoffsets[0]));
    }

    if (bnk->trackindex != BINKNOSOUND) { BinkSoundFrameFill(bnk); }

    bnk->FrameNum = frame + 1;

    return;
}