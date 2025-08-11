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

#include "BinkSoundUseWaveOut.hxx"

#include <WaveOut.hxx>

#include <stdio.h>

typedef HBINK(RADEXPLINK* BINKOPENACTION)(const char* name, u32 flags);
typedef void(RADEXPLINK* BINKCLOSEACTION)(HBINK bnk);
typedef s32(RADEXPLINK* BINKSETSOUNDSYSTEMACTION)(BINKSNDSYSOPEN open, void* param);
typedef BINKSNDOPEN(RADEXPLINK* BINKOPENWAVEOUTACTION)(void* param);
typedef void(RADEXPLINK* BINKSETVOLUMEACTION)(HBINK bnk, s32 volume);
typedef void(RADEXPLINK* BINKSETPANACTION)(HBINK bnk, s32 pan);

int ExecuteBinkSoundUseWaveOut(HMODULE bink, char* message)
{
    // Init
    BINKOPENACTION open = (BINKOPENACTION)GetProcAddress(bink, "_BinkOpen@8");
    BINKCLOSEACTION close = (BINKCLOSEACTION)GetProcAddress(bink, "_BinkClose@4");
    BINKSETSOUNDSYSTEMACTION sss =
        (BINKSETSOUNDSYSTEMACTION)GetProcAddress(bink, "_BinkSetSoundSystem@8");
    BINKOPENWAVEOUTACTION ows =
        (BINKOPENWAVEOUTACTION)GetProcAddress(bink, "_BinkOpenWaveOut@4");
    BINKSETVOLUMEACTION sv = (BINKSETVOLUMEACTION)GetProcAddress(bink, "_BinkSetVolume@8");
    BINKSETPANACTION sp = (BINKSETPANACTION)GetProcAddress(bink, "_BinkSetPan@8");

    sss(ows, NULL);
    BinkSetSoundSystem(BinkOpenWaveOut, NULL);

    // Open
    HBINK o = open("..\\Content\\cdv.bik", 0);
    HBINK i = BinkOpen("..\\Content\\cdv.bik", 0);

    u32 result = TRUE;

    HWAVEOUT sbo = (HWAVEOUT)(((u32*)o->bsnd.snddata)[9]);
    HWAVEOUT sbi = (HWAVEOUT)(((u32*)i->bsnd.snddata)[9]);

    // Volume
    for (u32 x = 0; x < 65536 + 1; x++) {
        sv(o, x);
        BinkSetVolume(i, x);

        const u32 svo = ((u32*)o->bsnd.snddata)[0];
        const u32 svi = ((u32*)i->bsnd.snddata)[0];

        if (svo != svi) {
            result = FALSE;
            strcpy(message, "Bink volume values are not matching.");
            break;
        }

        DWORD sbvo, sbvi;
        waveOutGetVolume(sbo, &sbvo);
        waveOutGetVolume(sbi, &sbvi);

        if (sbvo != sbvi) {
            result = FALSE;
            strcpy(message, "WaveOut volume values are not matching.");
            break;
        }
    }

    // Pan
    for (u32 x = 0; x < 65536 + 1; x++) {
        sp(o, x);
        BinkSetPan(i, x);

        u32 po = ((u32*)o->bsnd.snddata)[1];
        u32 pi = ((u32*)i->bsnd.snddata)[1];

        if (po != pi) {
            result = FALSE;
            strcpy(message, "Bink pan values are not matching.");
            break;
        }

        DWORD sbpo, sbpi;
        waveOutGetVolume(sbo, &sbpo);
        waveOutGetVolume(sbi, &sbpi);

        if (sbpo != sbpi) {
            result = FALSE;
            strcpy(message, "WaveOut pan values are not matching.");
            break;
        }
    }

    // Clean
    close(o);
    BinkClose(i);

    sss(ows, NULL);
    BinkSetSoundSystem(BinkOpenWaveOut, NULL);

    return result;
}