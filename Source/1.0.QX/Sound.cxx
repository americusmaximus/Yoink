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

#include "Sound.hxx"

s32             SoundTrack = BINKNOSOUND;

BINKSNDSYSOPEN  SoundOpen;
BINKSNDOPEN     SoundSystem;
u32             SoundCounter;

// 0x10008bb0
void RADEXPLINK BinkSoundFrameFill(HBINK bink)
{
    if (bink->bsnd.Ready != NULL) {
        bink->sndreenter = bink->sndreenter + 1;

        if (bink->sndreenter == 1) {
            while (bink->bsnd.Ready(&bink->bsnd)) {
                if (bink->sndamt <= bink->bsnd.BestSizeIn16
                    && (bink->FrameNum <= bink->sndendframe || bink->sndamt == 0)) {
                    break;
                }

                if (bink->bsnd.SoundDroppedOut) {
                    bink->bsnd.SoundDroppedOut = FALSE;

                    if (1 < bink->FrameNum && bink->FrameNum <= bink->sndendframe) {
                        const u32 time = RADTimerRead();

                        if (bink->sndpad < time) {
                            bink->soundskips = bink->soundskips + 1;

                            BinkSoundFrameClear(bink);

                            bink->startsynctime = 0;
                            bink->sndpad = time + 0x2EE;
                        }
                    }
                }

                u8* addr; u32 filled;
                u8* readpos;

                if (bink->bsnd.Lock(&bink->bsnd, &addr, &filled)) {
                    if (bink->sndconvert8) { filled = filled * 2; }
                    if (bink->sndamt < filled) { filled = bink->sndamt; }

                    bink->sndamt = bink->sndamt - (bink->sndend - bink->sndreadpos);

                    if ((size_t)(bink->sndend - bink->sndreadpos) < filled) {
                        if (bink->sndend - bink->sndreadpos != 0) {
                            if (bink->sndconvert8) {
                                addr = BinkSoundConvertTo8Bits(addr, (u16*)bink->sndreadpos, bink->sndend - bink->sndreadpos);
                            }
                            else {
                                radmemcpy(addr, bink->sndreadpos, bink->sndend - bink->sndreadpos);

                                addr = (u8*)((size_t)addr + (size_t)(bink->sndend - bink->sndreadpos));
                            }
                        }

                        if (bink->sndconvert8) {
                            BinkSoundConvertTo8Bits(addr, (u16*)bink->sndbuf, filled - (bink->sndend - bink->sndreadpos));
                            readpos = (u8*)((size_t)bink->sndbuf - (bink->sndend - bink->sndreadpos));
                        }
                        else {
                            radmemcpy(addr, bink->sndbuf, filled - (bink->sndend - bink->sndreadpos));
                            readpos = (u8*)((size_t)bink->sndbuf - (bink->sndend - bink->sndreadpos));
                        }
                    }
                    else {
                        if (bink->sndconvert8) { BinkSoundConvertTo8Bits(addr, (u16*)bink->sndreadpos, filled); }
                        else { radmemcpy(addr, bink->sndreadpos, filled); }

                        readpos = bink->sndreadpos;
                    }

                    bink->sndreadpos = (u8*)((size_t)readpos + filled);

                    if (bink->sndconvert8) { filled = filled / 2; }

                    bink->bsnd.Unlock(&bink->bsnd, filled);
                }
            }
        }

        bink->sndreenter = bink->sndreenter - 1;
    }
}

// 0x10008de0
u8* BinkSoundConvertTo8Bits(u8* dst, u16* src, size_t size)
{
    for (size_t x = 0; x < size; x++) {
        dst[x] = ((s8)(src[x] >> 8) - 128);
    }

    return dst;
}

// 0x10008e10
u32 BinkSoundFrameClear(HBINK bnk)
{
    if (bnk->sndprime <= bnk->sndamt) { return FALSE; }

    bnk->sndreadpos = (u8*)((size_t)bnk->sndreadpos - (bnk->sndprime - bnk->sndamt));

    if (bnk->sndreadpos < bnk->sndbuf) {
        bnk->sndreadpos = bnk->sndreadpos + bnk->sndbufsize;

        const size_t length = bnk->sndprime - bnk->sndamt
            - (bnk->sndbuf - (bnk->sndreadpos - (bnk->sndprime - bnk->sndamt)));

        radmemset(bnk->sndbuf, 0x00, length);

        radmemset(bnk->sndreadpos, 0x00, bnk->sndbuf - (bnk->sndreadpos - (bnk->sndprime - bnk->sndamt)));
    }
    else {
        radmemset((u8*)((size_t)bnk->sndreadpos - (bnk->sndprime - bnk->sndamt)), 0x00, bnk->sndprime - bnk->sndamt);
    }

    bnk->sndamt = bnk->sndamt + (bnk->sndprime - bnk->sndamt);

    return ((bnk->sndprime - bnk->sndamt) * 1000) / (bnk->bsnd.chans * bnk->bsnd.bits * bnk->bsnd.freq >> 3);
}

// 0x10028890
HBINKSNDCOMP BinkSoundCompressionInitialize(u32 freq, u32 chans, u32 type)
{
    HBINKSNDCOMP comp =
        (HBINKSNDCOMP)radmalloc(type == 0 ? 44304 : 18056); // TODO

    if (comp == NULL) { return NULL; }

    radmemset(comp, 0x00, 9864); // TODO

    // TODO

    return comp;
}

// 0x10028f80
void RADEXPLINK BinkSoundCompressorRelease(void* comp)
{
    radfree(comp);
}
