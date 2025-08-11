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

#define MAX_WAVE_OUT_ITEM_COUNT     16

typedef struct BINKWAVEOUTDATA
{
    u32         Volume;         // 0x0
    u32         Pan;            // 0x4
    s32         Unk0x8;         // 0x8 // TODO
    u32         IsPaused;       // 0xC
    s32         LockChunk;      // 0x10
    s32         WriteChunk;     // 0x14
    u32         WriteLength;    // 0x18
    s32         Unk0x1C;        // 0x1C // TODO
    u32         Length;         // 0x20
    HWAVEOUT    WaveOut;        // 0x24
    LPWAVEHDR*  Headers;        // 0x28
    void*       Mem;            // 0x2C
} BINKWAVEOUTDATA, * HBINKWAVEOUTDATA;

#define ASSOUNDDATA(X) ((HBINKWAVEOUTDATA)(X->snddata))

s32 RADLINK BinkWaveOutOnOff(struct BINKSND PTR4* snd, s32 status);
void RADLINK BinkWaveOutClose(struct BINKSND PTR4* snd);
void RADLINK BinkWaveOutVolume(struct BINKSND PTR4* snd, s32 volume);
void RADLINK BinkWaveOutPan(struct BINKSND PTR4* snd, s32 pan);
s32 RADLINK BinkWaveOutPause(struct BINKSND PTR4* snd, s32 status);
s32 RADLINK BinkWaveOutLock(struct BINKSND PTR4* snd, u8 PTR4* PTR4* addr, u32 PTR4* len);
s32 RADLINK BinkWaveOutUnlock(struct BINKSND PTR4* snd, u32 filled);
s32 RADLINK BinkWaveOutReady(struct BINKSND PTR4* snd);

void BinkWaveOutWrite(BINKSND* snd, u32 indx, u32 filled);
u32 BinkWaveOutInitialize(BINKSND* snd);
void BinkWaveOutRelease(BINKSND* snd);
void BinkWaveOutPause(BINKSND* snd);
void BinkWaveOutVolume(BINKSND* snd);
void BinkWaveOutRestart(BINKSND* snd);

// 0x1002b3f0
s32 RADLINK BinkWaveOutOpen(struct BINKSND PTR4* snd, u32 freq, s32 bits, s32 chans, u32 flags, HBINK bink)
{
    radmemset(snd, 0x00, sizeof(BINKSND));

    snd->SoundDroppedOut = FALSE;

    snd->freq = freq;
    snd->bits = bits;
    snd->chans = chans;

    ASSOUNDDATA(snd)->Volume = VOLUME_NORMAL;
    ASSOUNDDATA(snd)->Pan = PAN_CENTER;
    ASSOUNDDATA(snd)->IsPaused = FALSE;

    snd->OnOff = TRUE;

    snd->Ready = BinkWaveOutReady;
    snd->Lock = BinkWaveOutLock;
    snd->Unlock = BinkWaveOutUnlock;
    snd->Volume = BinkWaveOutVolume;
    snd->Pan = BinkWaveOutPan;
    snd->Pause = BinkWaveOutPause;
    snd->SetOnOff = BinkWaveOutOnOff;
    snd->Close = BinkWaveOutClose;

    if (!BinkWaveOutInitialize(snd)) { return FALSE; }

    snd->BestSizeIn16 = ASSOUNDDATA(snd)->Length;

    if (snd->bits != 16) { snd->BestSizeIn16 = ASSOUNDDATA(snd)->Length * 2; }

    snd->Latency = 0;

    return TRUE;
}

// 0x1002b4a0
u32 BinkWaveOutInitialize(BINKSND* snd)
{
    const u32 uVar9 = (snd->freq * snd->bits * snd->chans >> 3) * 1000; // TODO

    ASSOUNDDATA(snd)->Length = (uVar9 / 16000 + 0x1F) & 0xFFFFFFE0;
    ASSOUNDDATA(snd)->Unk0x1C = (uVar9 / 2000 + 0x1F) & 0xFFFFFFE0;

    ASSOUNDDATA(snd)->Unk0x8 = 0;
    ASSOUNDDATA(snd)->WriteLength = 0;
    ASSOUNDDATA(snd)->WriteChunk = DEFAULT_SOUND_CHUNK_INDEX;
    ASSOUNDDATA(snd)->LockChunk = DEFAULT_SOUND_CHUNK_INDEX;

    // Allocate one single chunk of memory that will be used as:
    // 
    // 1. Continuous space for all audio buffers.
    // 2. An array of pointers to WAVEHDR structures.
    // 3. Continuous space for all headers at the end of the memory region.

    ASSOUNDDATA(snd)->Mem =
        radmalloc((ASSOUNDDATA(snd)->Length + sizeof(WAVEHDR) + sizeof(LPWAVEHDR)) * MAX_WAVE_OUT_ITEM_COUNT);

    if (ASSOUNDDATA(snd)->Mem == NULL) { return FALSE; }

    ASSOUNDDATA(snd)->Headers =
        (LPWAVEHDR*)((size_t)ASSOUNDDATA(snd)->Mem + (size_t)(ASSOUNDDATA(snd)->Length * MAX_WAVE_OUT_ITEM_COUNT));

    for (u32 x = 0; x < MAX_WAVE_OUT_ITEM_COUNT; x++) {
        ASSOUNDDATA(snd)->Headers[x] = (LPWAVEHDR)((size_t)ASSOUNDDATA(snd)->Headers
            + sizeof(LPWAVEHDR) * MAX_WAVE_OUT_ITEM_COUNT + x * sizeof(WAVEHDR));

        radmemset(ASSOUNDDATA(snd)->Headers[x], 0x00, sizeof(WAVEHDR));

        ASSOUNDDATA(snd)->Headers[x]->lpData =
            (LPSTR)((size_t)ASSOUNDDATA(snd)->Mem + x * ASSOUNDDATA(snd)->Length);
    }

    radmemset(ASSOUNDDATA(snd)->Mem, snd->bits == 16 ? 0x00 : 0x80, ASSOUNDDATA(snd)->Length * MAX_WAVE_OUT_ITEM_COUNT);

    WAVEFORMATEX format;
    format.wFormatTag = WAVE_FORMAT_PCM;
    format.nChannels = (WORD)snd->chans;
    format.nSamplesPerSec = snd->freq;
    format.nAvgBytesPerSec = (snd->bits >> 3) * snd->chans * snd->freq;
    format.nBlockAlign = (WORD)((snd->bits >> 3) * snd->chans);
    format.wBitsPerSample = (WORD)snd->bits;
    format.cbSize = 0;

    ASSOUNDDATA(snd)->WaveOut = NULL;

    if (waveOutOpen(&ASSOUNDDATA(snd)->WaveOut, WAVE_MAPPER, &format, NULL, NULL, 0) != MMSYSERR_NOERROR) {
        radfree(ASSOUNDDATA(snd)->Mem);
        return FALSE;
    }

    for (u32 x = 0; x < MAX_WAVE_OUT_ITEM_COUNT; x++) {
        ASSOUNDDATA(snd)->Headers[x]->dwBufferLength = ASSOUNDDATA(snd)->Length;

        waveOutPrepareHeader(ASSOUNDDATA(snd)->WaveOut, ASSOUNDDATA(snd)->Headers[x], sizeof(WAVEHDR));

        ASSOUNDDATA(snd)->Headers[x]->dwFlags = ASSOUNDDATA(snd)->Headers[x]->dwFlags | WHDR_DONE;
    }

    return TRUE;
}

// 0x1002b660
s32 RADLINK BinkWaveOutReady(struct BINKSND PTR4* snd)
{
    if (!ASSOUNDDATA(snd)->IsPaused && snd->OnOff) {
        if (ASSOUNDDATA(snd)->Unk0x8 != 0) { // TODO
            for (u32 x = 0; x < MAX_WAVE_OUT_ITEM_COUNT; x++) {
                if (!(ASSOUNDDATA(snd)->Headers[x]->dwFlags & WHDR_DONE)) {
                    const s32 indx = (ASSOUNDDATA(snd)->WriteChunk + 1) % MAX_WAVE_OUT_ITEM_COUNT;

                    if (ASSOUNDDATA(snd)->Headers[indx]->dwFlags & WHDR_DONE) {
                        ASSOUNDDATA(snd)->LockChunk = indx;

                        return TRUE;
                    }
                }
            }

            snd->SoundDroppedOut = TRUE;

            ASSOUNDDATA(snd)->Unk0x8 = 0; // TODO
            ASSOUNDDATA(snd)->WriteLength = 0;
            ASSOUNDDATA(snd)->WriteChunk = 0;
            ASSOUNDDATA(snd)->LockChunk = DEFAULT_SOUND_CHUNK_INDEX;
        }

        const s32 indx = (ASSOUNDDATA(snd)->WriteChunk + 1) % MAX_WAVE_OUT_ITEM_COUNT;

        if (ASSOUNDDATA(snd)->Headers[indx]->dwFlags & WHDR_DONE) {
            ASSOUNDDATA(snd)->LockChunk = indx;

            return TRUE;
        }
    }

    return FALSE;
}

// 0x1002b6e0
s32 RADLINK BinkWaveOutLock(struct BINKSND PTR4* snd, u8 PTR4* PTR4* addr, u32 PTR4* len)
{
    if (ASSOUNDDATA(snd)->LockChunk == DEFAULT_SOUND_CHUNK_INDEX) { return FALSE; }

    *addr = (u8*)((size_t)ASSOUNDDATA(snd)->Mem + ASSOUNDDATA(snd)->LockChunk * ASSOUNDDATA(snd)->Length);

    *len = ASSOUNDDATA(snd)->Length;

    return TRUE;
}

// 0x1002b710
s32 RADLINK BinkWaveOutUnlock(struct BINKSND PTR4* snd, u32 filled)
{
    if (ASSOUNDDATA(snd)->LockChunk == DEFAULT_SOUND_CHUNK_INDEX) { return FALSE; }

    ASSOUNDDATA(snd)->WriteChunk = ASSOUNDDATA(snd)->LockChunk;
    ASSOUNDDATA(snd)->WriteLength = ASSOUNDDATA(snd)->WriteLength + filled;

    ASSOUNDDATA(snd)->LockChunk = DEFAULT_SOUND_CHUNK_INDEX;

    if (ASSOUNDDATA(snd)->Unk0x8 == 0) {
        if (ASSOUNDDATA(snd)->Unk0x1C < ASSOUNDDATA(snd)->Unk0x8) {
            ASSOUNDDATA(snd)->Unk0x8 = 1;

            BinkWaveOutWrite(snd, 0, ASSOUNDDATA(snd)->WriteLength);

            return TRUE;
        }
    }
    else { BinkWaveOutWrite(snd, ASSOUNDDATA(snd)->WriteChunk, filled); }

    return TRUE;
}

// 0x1002b780
void BinkWaveOutWrite(BINKSND* snd, u32 indx, u32 length)
{
    if (ASSOUNDDATA(snd)->WaveOut != NULL) {
        ASSOUNDDATA(snd)->Headers[indx]->dwBufferLength = length;
        ASSOUNDDATA(snd)->Headers[indx]->dwFlags = ASSOUNDDATA(snd)->Headers[indx]->dwFlags & (~WHDR_DONE);

        waveOutWrite(ASSOUNDDATA(snd)->WaveOut, ASSOUNDDATA(snd)->Headers[indx], sizeof(WAVEHDR));
    }
}

// 0x1002b7c0
void RADLINK BinkWaveOutVolume(struct BINKSND PTR4* snd, s32 volume)
{
    if (VOLUME_MAX < volume) { volume = VOLUME_MAX; }

    ASSOUNDDATA(snd)->Volume = (volume + 1) * VOLUME_NORMAL / (VOLUME_MAX + 1);

    BinkWaveOutVolume(snd);
}

// 0x1002b810
void BinkWaveOutVolume(BINKSND* snd)
{
    u16 left, right;

    if (ASSOUNDDATA(snd)->Pan < (PAN_CENTER + 1)) {
        right = (u16)ASSOUNDDATA(snd)->Volume;
        left = (u16)(ASSOUNDDATA(snd)->Volume * ASSOUNDDATA(snd)->Pan / PAN_CENTER);
    }
    else {
        right = (u16)((ASSOUNDDATA(snd)->Volume * ((PAN_RIGHT + 1) - ASSOUNDDATA(snd)->Pan)) / (PAN_CENTER + 2));
        left = (u16)ASSOUNDDATA(snd)->Volume;
    }

    if (ASSOUNDDATA(snd)->WaveOut != NULL) {
        // The low-order word contains the left-channel volume setting,
        // and the high-order word contains the right-channel setting.
        // A value of 0xFFFF represents full volume, and a value of 0x0000 is silence.

        waveOutSetVolume(ASSOUNDDATA(snd)->WaveOut, left << 16 | right);
    }
}

// 0x1002b890
void RADLINK BinkWaveOutPan(struct BINKSND PTR4* snd, s32 pan)
{
    if ((PAN_RIGHT - 1) < pan) { pan = PAN_RIGHT - 1; }

    ASSOUNDDATA(snd)->Pan = pan;

    BinkWaveOutVolume(snd);
}

// 0x1002b8c0
s32 RADLINK BinkWaveOutOnOff(struct BINKSND PTR4* snd, s32 status)
{
    if (status) {
        if (!snd->OnOff) {
            if (BinkWaveOutInitialize(snd)) { snd->OnOff = TRUE; }
        }
    }
    else {
        if (status) { return snd->OnOff; }

        if (snd->OnOff) {
            BinkWaveOutRelease(snd);
            snd->OnOff = FALSE;
        }
    }

    return snd->OnOff;
}

// 0x1002b920
void BinkWaveOutRelease(BINKSND* snd)
{
    if (ASSOUNDDATA(snd)->WaveOut != NULL) {
        waveOutReset(ASSOUNDDATA(snd)->WaveOut);

        for (u32 x = 0; x < MAX_WAVE_OUT_ITEM_COUNT; x++) {
            ASSOUNDDATA(snd)->Headers[x]->dwBufferLength = ASSOUNDDATA(snd)->Length;

            waveOutUnprepareHeader(ASSOUNDDATA(snd)->WaveOut,
                ASSOUNDDATA(snd)->Headers[x], sizeof(WAVEHDR));

            ASSOUNDDATA(snd)->Headers[x]->dwFlags = ASSOUNDDATA(snd)->Headers[x]->dwFlags | WHDR_DONE;
        }

        waveOutClose(ASSOUNDDATA(snd)->WaveOut);

        ASSOUNDDATA(snd)->WaveOut = NULL;
    }

    if (ASSOUNDDATA(snd)->Mem != NULL) {
        radfree(ASSOUNDDATA(snd)->Mem);
        ASSOUNDDATA(snd)->Mem = NULL;
    }
}

// 0x1002b9a0
s32 RADLINK BinkWaveOutPause(struct BINKSND PTR4* snd, s32 status)
{
    if (status) {
        BinkWaveOutPause(snd);
        ASSOUNDDATA(snd)->IsPaused = TRUE;
    }
    else {
        BinkWaveOutRestart(snd);
        ASSOUNDDATA(snd)->IsPaused = FALSE;
    }

    return ASSOUNDDATA(snd)->IsPaused;
}

// 0x1002b9e0
void BinkWaveOutPause(BINKSND* snd)
{
    if (ASSOUNDDATA(snd)->WaveOut != NULL) {
        waveOutPause(ASSOUNDDATA(snd)->WaveOut);
    }
}

// 0x1002ba00
void BinkWaveOutRestart(BINKSND* snd)
{
    if (ASSOUNDDATA(snd)->WaveOut != NULL) {
        waveOutRestart(ASSOUNDDATA(snd)->WaveOut);
    }
}

// 0x1002ba20
void RADLINK BinkWaveOutClose(struct BINKSND PTR4* snd)
{
    BinkWaveOutRelease(snd);
}