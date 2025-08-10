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

#include "BinkSoundUseDirectSound.hxx"

#include <DirectSound.hxx>

#include <stdio.h>

typedef HBINK(RADEXPLINK* BINKOPENACTION)(const char* name, u32 flags);
typedef void(RADEXPLINK* BINKCLOSEACTION)(HBINK bnk);
typedef s32(RADEXPLINK* BINKSETSOUNDSYSTEMACTION)(BINKSNDSYSOPEN open, void* param);
typedef BINKSNDOPEN(RADEXPLINK* BINKOPENDIRECTSOUNDACTION)(void* param);
typedef void(RADEXPLINK* BINKSETVOLUMEACTION)(HBINK bnk, s32 volume);
typedef void(RADEXPLINK* BINKSETPANACTION)(HBINK bnk, s32 pan);

int ExecuteBinkSoundUseDirectSound(HMODULE bink, char* message)
{
    LPDIRECTSOUND ds1 = NULL;
    if (FAILED(DirectSoundCreate(NULL, &ds1, NULL))) {
        strcpy(message, "Unable to create DirectSound.\r\n");

        return FALSE;
    }

    LPDIRECTSOUND ds2 = NULL;
    if (FAILED(DirectSoundCreate(NULL, &ds2, NULL))) {
        strcpy(message, "Unable to create DirectSound.\r\n");

        ds1->Release();

        return FALSE;
    }

    // Init
    BINKOPENACTION open = (BINKOPENACTION)GetProcAddress(bink, "_BinkOpen@8");
    BINKCLOSEACTION close = (BINKCLOSEACTION)GetProcAddress(bink, "_BinkClose@4");
    BINKSETSOUNDSYSTEMACTION sss =
        (BINKSETSOUNDSYSTEMACTION)GetProcAddress(bink, "_BinkSetSoundSystem@8");
    BINKOPENDIRECTSOUNDACTION ods =
        (BINKOPENDIRECTSOUNDACTION)GetProcAddress(bink, "_BinkOpenDirectSound@4");
    BINKSETVOLUMEACTION sv = (BINKSETVOLUMEACTION)GetProcAddress(bink, "_BinkSetVolume@8");
    BINKSETPANACTION sp = (BINKSETPANACTION)GetProcAddress(bink, "_BinkSetPan@8");

    sss(ods, ds1);
    BinkSetSoundSystem(BinkOpenDirectSound, ds2);

    // Open
    HBINK o = open("..\\Content\\cdv.bik", 0);
    HBINK i = BinkOpen("..\\Content\\cdv.bik", 0);

    u32 result = TRUE;

    LPDIRECTSOUNDBUFFER sbo = (LPDIRECTSOUNDBUFFER)(((u32*)o->bsnd.snddata)[0]);
    LPDIRECTSOUNDBUFFER sbi = (LPDIRECTSOUNDBUFFER)(((u32*)i->bsnd.snddata)[0]);

    // Volume
    for (u32 x = 0; x < 65536 + 1; x++) {
        sv(o, x);
        BinkSetVolume(i, x);

        const u32 svo = ((u32*)o->bsnd.snddata)[7];
        const u32 svi = ((u32*)i->bsnd.snddata)[7];

        if (svo != svi) {
            result = FALSE;
            strcpy(message, "Bink volume values are not matching.");
            break;
        }

        LONG sbvo, sbvi;
        sbo->GetVolume(&sbvo);
        sbi->GetVolume(&sbvi);

        if (sbvo != sbvi) {
            result = FALSE;
            strcpy(message, "DirectSound buffer volume values are not matching.");
            break;
        }
    }

    // Pan
    for (u32 x = 0; x < 65536 + 1; x++) {
        sp(o, x);
        BinkSetPan(i, x);

        u32 po = ((u32*)o->bsnd.snddata)[8];
        u32 pi = ((u32*)i->bsnd.snddata)[8];

        if (po != pi) {
            result = FALSE;
            strcpy(message, "Bink pan values are not matching.");
            break;
        }

        LONG sbpo, sbpi;
        sbo->GetPan(&sbpo);
        sbi->GetPan(&sbpi);

        if (sbpo != sbpi) {
            result = FALSE;
            strcpy(message, "DirectSound buffer pan values are not matching.");
            break;
        }
    }

    // Clean
    close(o);
    BinkClose(i);

    sss(ods, NULL);
    BinkSetSoundSystem(BinkOpenDirectSound, NULL);

    ds1->Release();
    ds2->Release();

    return result;
}