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

#include "Bink.hxx"

// 0x10008ef0
u32 BinkCalculateHighest1SecFrameRate(u32 frames, u32* frameoffsets, u32 runtimeframes, u32* high, u32* param_5) // TODO
// TODO move to Frame
{
    // TODO refactor
    u32 count = 0;
    u32 rate = 0;
    u32 value = 0;
    u32 todo = 1; // TODO

    if (runtimeframes < frames) {
        u32* end = frameoffsets + runtimeframes;
        u32* start = frameoffsets;

        do {
            if (rate < *end - *start) {
                rate = *end - *start;
                value = count;
            }

            count = count + 1;
            end = end + 1;
            start = start + 1;
        } while (count < frames - runtimeframes);

        if (rate != 0) goto LAB_10008f54; // TODO
    }

    rate = ((frameoffsets[frames] - frameoffsets[0]) * runtimeframes) / frames;

LAB_10008f54: // TODO
    count = 0;
    if (frames != 0) {
        do {
            if ((*frameoffsets & 1) == 0) {
                todo = 0;
                break;
            }

            count = count + 1;
            frameoffsets = frameoffsets + 1;
        } while (count < frames);
    }

    *param_5 = todo;
    *high = value;

    return rate;
}

// 0x10009b90
void BinkSetStartSyncTime(HBINK bnk)
{
    if (bnk->startsynctime == 0) {
        bnk->startsynctime = RADTimerRead();
        bnk->startsyncframe = bnk->playedframes - 1;
    }
}

// 0x1000a110
void BinkSumTimeBlit(HBINK bnk, u32 time)
{
    if (bnk->startblittime != 0) {
        bnk->timeblit = bnk->timeblit + (time - bnk->startblittime);
        bnk->startblittime = 0;
    }
}

// 0x1000a530
void BinkSumFrameTime(HBINK bnk, u32 time)
{
    if (bnk->entireframetime != 0) {
        const u32 diff = time - bnk->entireframetime;

        bnk->entireframetime = 0;

        if (bnk->slowestframetime < diff) {
            bnk->slowest2frametime = bnk->slowestframetime;
            bnk->slowestframetime = diff;
            bnk->slowest2frame = bnk->slowestframe;
            bnk->slowestframe = bnk->FrameNum;
        }
        else if (bnk->slowest2frametime < diff) {
            bnk->slowest2frametime = diff;
            bnk->slowest2frame = bnk->FrameNum;
        }
    }
}

// 0x100026a0
size_t BinkCalculateBundleSize(u32 param_1, u32 param_2, u32 param_3, u32 param_4) // TODO
{
    if (param_3 < 9) { param_3 = 8; }
    else if (param_3 < 16) { param_3 = 16; }

    return ((param_2 >> 3) * param_3 * param_4 >> 3) + 3 + (param_1 * param_3 >> 3) & 0xfffffffc;
}