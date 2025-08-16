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

#define DIRECT_SOUND_VALUES_COUNT           128

// Normalizes volume value from the input range of 0 to 65536
// to a range of values from 0 to 127 that can be used to pick the resulting value from the map table.
#define NORMALIZE_VOLUME(x)                 (x * 127 / VOLUME_NORMAL)

// Normalized pan value from the input range of 0 to 65536, where:
// 0 is all the way left, 32768 - equal panning, and 65536 - all the way right.
// to a range of values of 0 to 127 that can be used to pick the resulting value from the map table.
#define NORMALIZE_PAN(x)                    ((x * 127 + (PAN_CENTER + 1)) >> 16)

#define ASSOUNDDATA(X) ((HBINKDIRECTSOUNDDATA)(X->snddata))

LPDIRECTSOUND           DirectSound = DEFAULT_DIRECT_SOUND_INSTANCE;

HWND                    DirectSoundActiveWindow;
u32                     DirectSoundIsManaged;

HMODULE                 DirectSoundModule;
DIRECTSOUNDCREATEACTION DirectSoundCreateAction;
u32                     DirectSoundIsCooperativeLevelSet;
LPDIRECTSOUNDBUFFER     DirectSoundBuffer;
u32                     DirectSoundIsEmulated;
u32                     DirectSoundCount;

static const s16 DirectSoundValues[DIRECT_SOUND_VALUES_COUNT] =
{
    -10000, -4208,  -3606,  -3253,
    -3003,  -2810,  -2651,  -2517,
    -2401,  -2299,  -2208,  -2125,
    -2049,  -1980,  -1915,  -1855,
    -1799,  -1747,  -1697,  -1650,
    -1606,  -1563,  -1523,  -1484,
    -1447,  -1412,  -1378,  -1345,
    -1313,  -1283,  -1253,  -1225,
    -1197,  -1171,  -1145,  -1119,
    -1095,  -1071,  -1048,  -1025,
    -1003,  -982,   -961,   -941,
    -921,   -901,   -882,   -863,
    -845,   -827,   -810,   -792,
    -776,   -759,   -743,   -727,
    -711,   -696,   -681,   -666,
    -651,   -637,   -623,   -609,
    -595,   -582,   -569,   -555,
    -543,   -530,   -517,   -505,
    -493,   -481,   -469,   -457,
    -446,   -435,   -423,   -412,
    -401,   -391,   -380,   -369,
    -359,   -349,   -339,   -329,
    -319,   -309,   -299,   -290,
    -280,   -271,   -261,   -252,
    -243,   -234,   -225,   -216,
    -208,   -199,   -190,   -182,
    -174,   -165,   -157,   -149,
    -141,   -133,   -125,   -117,
    -109,   -101,   -94,    -86,
    -79,    -71,    -64,    -57,
    -49,    -42,    -35,    -28,
    -21,    -14,    -7,      0
};

s32 RADLINK BinkDirectSoundLock(struct BINKSND PTR4* snd, u8 PTR4* PTR4* addr, u32 PTR4* len);
s32 RADLINK BinkDirectSoundOnOff(struct BINKSND PTR4* snd, s32 status);
s32 RADLINK BinkDirectSoundPause(struct BINKSND PTR4* snd, s32 status);
s32 RADLINK BinkDirectSoundReady(struct BINKSND PTR4* snd);
s32 RADLINK BinkDirectSoundUnlock(struct BINKSND PTR4* snd, u32 filled);
void RADLINK BinkDirectSoundClose(struct BINKSND PTR4* snd);
void RADLINK BinkDirectSoundPan(struct BINKSND PTR4* snd, s32 pan);
void RADLINK BinkDirectSoundVolume(struct BINKSND PTR4* snd, s32 volume);

s32 BinkDirectSoundBufferGetCurrentChunk(HBINKDIRECTSOUNDDATA snd);
void BinkDirectSoundBufferFill(HBINKDIRECTSOUNDDATA snd);
void BinkDirectSoundBufferInitialize(HBINKDIRECTSOUNDDATA snd);
void BinkDirectSoundBufferLock(HBINKDIRECTSOUNDDATA snd, u32 indx);
void BinkDirectSoundBufferPlay(HBINKDIRECTSOUNDDATA snd);
void BinkDirectSoundBufferStop(HBINKDIRECTSOUNDDATA snd);
void BinkDirectSoundBufferUnlock(HBINKDIRECTSOUNDDATA snd);

// 1002a890
HWND BinkDirectSoundAcquireActiveWindow()
{
    DWORD pid = GetCurrentProcessId();
    HMODULE mod = GetModuleHandleA(NULL);

    if (DirectSoundActiveWindow == NULL) {
        for (HWND wnd = GetTopWindow(NULL); wnd != NULL; wnd = GetWindow(wnd, GW_HWNDNEXT)) {

            DWORD tpid = 0;
            GetWindowThreadProcessId(wnd, &tpid);

            if (tpid == pid) {
                HINSTANCE inst = (HINSTANCE)GetWindowLongA(wnd, GWL_HINSTANCE);

                if (inst == mod && !(GetWindowLongA(wnd, GWL_STYLE) & WS_CHILD)) {
                    DirectSoundActiveWindow = wnd;
                }

                if (DirectSoundActiveWindow == NULL) { DirectSoundActiveWindow = wnd; }
            }
        }

        if (DirectSoundActiveWindow == NULL) { DirectSoundActiveWindow = GetActiveWindow(); }
    }

    return DirectSoundActiveWindow;
}

// 0x1002aa10
u32 BinkDirectSoundInitialize()
{
    DirectSoundCount = DirectSoundCount + 1;

    if (DirectSoundCount == 1) {
        if (DirectSoundIsManaged) {
            if (DirectSoundCreateAction == NULL) {
                DirectSoundCount = 0;
                return FALSE;
            }

            if (FAILED(DirectSoundCreateAction(NULL, &DirectSound, NULL))) {
                DirectSoundCount = DirectSoundCount - 1;
                return FALSE;
            }

            DirectSoundIsCooperativeLevelSet =
                SUCCEEDED(DirectSound->SetCooperativeLevel(BinkDirectSoundAcquireActiveWindow(), DSSCL_PRIORITY));
        }

        DSCAPS caps;
        radmemset(&caps, 0x00, sizeof(DSCAPS));

        caps.dwSize = sizeof(DSCAPS);

        DirectSoundIsEmulated = FALSE;

        if (SUCCEEDED(DirectSound->GetCaps(&caps))) { DirectSoundIsEmulated = caps.dwFlags & DSCAPS_EMULDRIVER; }
    }

    return TRUE;
}

// 0x1002aad0
void BinkDirectSoundRelease()
{
    DirectSoundCount = DirectSoundCount - 1;

    if (DirectSoundCount == 0 && DirectSoundIsManaged
        && DirectSound != NULL && DirectSound != DEFAULT_DIRECT_SOUND_INSTANCE) {
        DirectSound->Release();
        DirectSound = DEFAULT_DIRECT_SOUND_INSTANCE;
    }
}

// 0x1002ab00
s32 RADLINK BinkDirectSoundOpen(struct BINKSND PTR4* snd, u32 freq, s32 bits, s32 chans, u32 flags, HBINK bink)
{
    WAVEFORMATEX format;

    radmemset(snd, 0x00, sizeof(BINKSND));

    if (!BinkDirectSoundInitialize()) { return FALSE; }

    snd->bits = bits;
    format.wBitsPerSample = (WORD)snd->bits;

    ASSOUNDDATA(snd)->Bits = bits;
    ASSOUNDDATA(snd)->Channels = chans;
    snd->chans = chans;

    format.nChannels = chans & 0xFF;
    format.nBlockAlign = (format.wBitsPerSample >> 3) * (chans & 0xFF);
    format.nAvgBytesPerSec = (format.wBitsPerSample >> 3) * (chans & 0xFF) * freq;

    snd->SoundDroppedOut = FALSE;
    
    ASSOUNDDATA(snd)->Frequency = freq;
    snd->freq = freq;

    format.wFormatTag = WAVE_FORMAT_PCM;
    format.nSamplesPerSec = freq;

    if (DirectSoundIsCooperativeLevelSet) {
        DSBUFFERDESC desc;
        radmemset(&desc, 0x00, sizeof(DSBUFFERDESC));

        desc.dwSize = sizeof(DSBUFFERDESC);
        desc.dwFlags = DSBCAPS_PRIMARYBUFFER;

        if (SUCCEEDED(DirectSound->CreateSoundBuffer(&desc, &DirectSoundBuffer, NULL))) {
            DirectSoundBuffer->SetFormat(&format);
            DirectSoundBuffer->Play(0, 0, DSBPLAY_LOOPING);
        }

        DirectSoundIsCooperativeLevelSet = FALSE;
    }

    ASSOUNDDATA(snd)->Unk0x8 = 4; // TODO
    ASSOUNDDATA(snd)->ChunkSize = 1;

    {
        const u32 length = (DirectSoundIsEmulated ? 175 : 100) * format.nAvgBytesPerSec / 1000;

        if (1 < length) {
            do {
                ASSOUNDDATA(snd)->ChunkSize = ASSOUNDDATA(snd)->ChunkSize * 2;
            } while (ASSOUNDDATA(snd)->ChunkSize < length);
        }
    }

    ASSOUNDDATA(snd)->Length = ASSOUNDDATA(snd)->ChunkSize * 4;
    snd->Latency = (((ASSOUNDDATA(snd)->ChunkSize * 3) * 1000) / format.nAvgBytesPerSec);

    DSBUFFERDESC desc;
    radmemset(&desc, 0x00, sizeof(DSBUFFERDESC));

    desc.dwSize = sizeof(DSBUFFERDESC);
    desc.dwFlags =
        DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPAN | DSBCAPS_CTRLFREQUENCY;
    desc.dwBufferBytes = ASSOUNDDATA(snd)->Length;
    desc.lpwfxFormat = &format;

    if (FAILED(DirectSound->CreateSoundBuffer(&desc, &ASSOUNDDATA(snd)->Buffer, NULL))) { return FALSE; }

    snd->BestSizeIn16 = ASSOUNDDATA(snd)->ChunkSize;

    if (snd->bits == 8) { snd->BestSizeIn16 = ASSOUNDDATA(snd)->ChunkSize * 2; }

    ASSOUNDDATA(snd)->Volume = VOLUME_NORMAL;
    ASSOUNDDATA(snd)->Pan = PAN_CENTER;

    BinkDirectSoundBufferInitialize(ASSOUNDDATA(snd));
    BinkDirectSoundBufferFill(ASSOUNDDATA(snd));

    ASSOUNDDATA(snd)->ChunkIndex1 = DEFAULT_SOUND_CHUNK_INDEX;
    ASSOUNDDATA(snd)->ChunkIndex2 = DEFAULT_SOUND_CHUNK_INDEX;

    ASSOUNDDATA(snd)->Unk0x24 = 0; // TODO

    ASSOUNDDATA(snd)->Unk0x28 = FALSE;
    ASSOUNDDATA(snd)->IsStopped = FALSE;

    snd->OnOff = TRUE;

    snd->Ready = BinkDirectSoundReady;
    snd->Lock = BinkDirectSoundLock;
    snd->Unlock = BinkDirectSoundUnlock;
    snd->Volume = BinkDirectSoundVolume;
    snd->Pan = BinkDirectSoundPan;
    snd->Pause = BinkDirectSoundPause;
    snd->SetOnOff = BinkDirectSoundOnOff;
    snd->Close = BinkDirectSoundClose;

    return TRUE;
}

// 0x1002ad40
void BinkDirectSoundBufferInitialize(HBINKDIRECTSOUNDDATA snd)
{
    if (snd->Buffer == NULL) { return; }

    LONG pan = DSBPAN_CENTER;
    LONG volume = snd->Volume;

    if (VOLUME_NORMAL < volume) { volume = VOLUME_NORMAL; }

    // Pan value must be normalized to a range of 0 to 127.
    //
    // Unlike volume that varies in range of DSBVOLUME_MIN (-10000) to DSBVOLUME_MAX (0),
    // pan ranges from DSBPAN_LEFT (-10000) to DSBPAN_RIGHT (10000), therefore,
    // depending on the normalized pan value:
    //
    // 1. For normalized pan value is 0 to 63 sound values can be reused directly.
    // 2. Normalized pan value of 64, midpoint, means DSBPAN_CENTER (0).
    // 3. Sound values map has to be indexed in reverse for normalized pan values of 65 to 127,
    //      and the resulting value negated to bring it to range of DSBPAN_CENTER (0) to DSBPAN_RIGHT (10000).
    //
    // Additionally, normalized pan values between -24 and 24 are considered the same and reulsting in DSBPAN_CENTER (0).

    if (snd->Pan < PAN_RIGHT) {
        pan = NORMALIZE_PAN(snd->Pan);

        if (64 < pan) {
            pan = -DirectSoundValues[126 - 2 * (pan - 64)];

            if (pan >= -24 && pan <= 24) { pan = DSBPAN_CENTER; }
        }
        else if (pan < 64) {
            pan = DirectSoundValues[pan * 2];

            if (pan >= -24 && pan <= 24) { pan = DSBPAN_CENTER; }
        }
        else { pan = DSBPAN_CENTER; }
    }
    else { pan = DSBPAN_LEFT; }

    snd->Buffer->SetFrequency(snd->Frequency);
    snd->Buffer->SetPan(pan);

    // Normalize input volume to an index in the map, and use the value from the map.
    snd->Buffer->SetVolume(DirectSoundValues[NORMALIZE_VOLUME(volume)]);
}

// 0x1002ae10
void BinkDirectSoundBufferFill(HBINKDIRECTSOUNDDATA snd)
{
    void* a1;
    void* a2;
    DWORD l1, l2;

    while (TRUE) {
        const HRESULT result = snd->Buffer->Lock(0, snd->Length, &a1, &l1, &a2, &l2, 0);

        if (SUCCEEDED(result) && l1 == snd->Length) {
            radmemset(a1, snd->Bits == 16 ? 0x00 : 0x80, l1);

            snd->Buffer->Unlock(a1, l1, a2, l2);

            return;
        }

        if (result != DSERR_BUFFERLOST) { break; }

        Sleep(10);
        snd->Buffer->Restore();
    }
}

// 0x1002aec0
s32 RADLINK BinkDirectSoundReady(struct BINKSND PTR4* snd)
{
    if (ASSOUNDDATA(snd)->IsStopped && !snd->OnOff) { return FALSE; }

    if (!ASSOUNDDATA(snd)->Unk0x28) {
        if (ASSOUNDDATA(snd)->ChunkIndex2 == DEFAULT_SOUND_CHUNK_INDEX) {
            u32 iVar7 = ASSOUNDDATA(snd)->Unk0x24; // TODO

            if (ASSOUNDDATA(snd)->Unk0x8 <= iVar7) {
                ASSOUNDDATA(snd)->Unk0x28 = TRUE;

                BinkDirectSoundBufferPlay(ASSOUNDDATA(snd));

                ASSOUNDDATA(snd)->ChunkIndex2 = DEFAULT_SOUND_CHUNK_INDEX;

                ASSOUNDDATA(snd)->ChunkIndex1 = BinkDirectSoundBufferGetCurrentChunk(ASSOUNDDATA(snd));

                return FALSE;
            }

            ASSOUNDDATA(snd)->ChunkIndex2 = iVar7;
            ASSOUNDDATA(snd)->Unk0x24 = iVar7 + 1;

            return TRUE;
        }
    }
    else {
        const s32 current = BinkDirectSoundBufferGetCurrentChunk(ASSOUNDDATA(snd));

        if (ASSOUNDDATA(snd)->ChunkIndex2 == DEFAULT_SOUND_CHUNK_INDEX) {
            const s32 indx = ASSOUNDDATA(snd)->ChunkIndex1; // TODO

            if (indx == current) { return FALSE; }

            s32 diff = indx < current ? current - indx - 1 : current - indx + 3;

            if (diff != 0) {
                snd->SoundDroppedOut = TRUE;

                do {
                    BinkDirectSoundBufferLock(ASSOUNDDATA(snd), ASSOUNDDATA(snd)->ChunkIndex1);

                    radmemset(ASSOUNDDATA(snd)->Lock1Data,
                        ASSOUNDDATA(snd)->Bits == 16 ? 0x00 : 0x80, ASSOUNDDATA(snd)->Lock1Length);

                    BinkDirectSoundBufferUnlock(ASSOUNDDATA(snd));

                    ASSOUNDDATA(snd)->Lock1Data = NULL;
                    ASSOUNDDATA(snd)->ChunkIndex1 = ASSOUNDDATA(snd)->ChunkIndex1 + 1;

                    if (3 < ASSOUNDDATA(snd)->ChunkIndex1) { ASSOUNDDATA(snd)->ChunkIndex1 = 0; }

                    diff = diff - 1;
                } while (diff != 0);
            }

            ASSOUNDDATA(snd)->ChunkIndex2 = current == 0 ? 3 : current - 1;
            ASSOUNDDATA(snd)->ChunkIndex1 = current;
        }
        else if (current != ASSOUNDDATA(snd)->ChunkIndex1) {
            do {
                snd->SoundDroppedOut = TRUE;
                BinkDirectSoundBufferLock(ASSOUNDDATA(snd), ASSOUNDDATA(snd)->ChunkIndex2);

                radmemset(ASSOUNDDATA(snd)->Lock1Data,
                    ASSOUNDDATA(snd)->Bits == 16 ? 0x00 : 0x80, ASSOUNDDATA(snd)->Lock1Length);

                BinkDirectSoundBufferUnlock(ASSOUNDDATA(snd));

                ASSOUNDDATA(snd)->Lock1Data = NULL;
                ASSOUNDDATA(snd)->ChunkIndex2 = ASSOUNDDATA(snd)->ChunkIndex2 + 1;

                if (3 < ASSOUNDDATA(snd)->ChunkIndex2) { ASSOUNDDATA(snd)->ChunkIndex2 = 0; }

                ASSOUNDDATA(snd)->ChunkIndex1 = ASSOUNDDATA(snd)->ChunkIndex1 + 1;

                if (3 < ASSOUNDDATA(snd)->ChunkIndex1) { ASSOUNDDATA(snd)->ChunkIndex1 = 0; }
            } while (current != ASSOUNDDATA(snd)->ChunkIndex1);

            return TRUE;
        }
    }

    return TRUE;
}

// 0x1002b0a0
void BinkDirectSoundBufferLock(HBINKDIRECTSOUNDDATA snd, u32 indx)
{
    while (TRUE) {
        const HRESULT result = snd->Buffer->Lock(snd->ChunkSize * indx, snd->ChunkSize,
            (LPVOID*)&snd->Lock1Data, &snd->Lock1Length, (LPVOID*)&snd->Lock2Data, &snd->Lock2Length, 0);

        if (result != DSERR_BUFFERLOST) { break; }

        Sleep(10);
        snd->Buffer->Restore();
    }
}

// 0x1002b0f0
void BinkDirectSoundBufferUnlock(HBINKDIRECTSOUNDDATA snd)
{
    snd->Buffer->Unlock(snd->Lock1Data, snd->Lock1Length, snd->Lock2Data, snd->Lock2Length);
}

// 0x1002b110
void BinkDirectSoundBufferPlay(HBINKDIRECTSOUNDDATA snd)
{
    while (TRUE) {
        const HRESULT result = snd->Buffer->Play(0, 0, DSBPLAY_LOOPING);

        if (result != DSERR_BUFFERLOST) { break; }

        Sleep(10);
        snd->Buffer->Restore();
    }

    snd->IsStopped = FALSE;
}

// 0x1002b150
s32 BinkDirectSoundBufferGetCurrentChunk(HBINKDIRECTSOUNDDATA snd)
{
    DWORD play, write;

    if (FAILED(snd->Buffer->GetCurrentPosition(&play, &write))) { return DEFAULT_SOUND_CHUNK_INDEX; }

    return play / snd->ChunkSize;
}

// 0x1002b180
s32 RADLINK BinkDirectSoundLock(struct BINKSND PTR4* snd, u8 PTR4* PTR4* addr, u32 PTR4* len)
{
    if (ASSOUNDDATA(snd)->ChunkIndex2 != DEFAULT_SOUND_CHUNK_INDEX && ASSOUNDDATA(snd)->Lock1Data == NULL) {
        BinkDirectSoundBufferLock(ASSOUNDDATA(snd), ASSOUNDDATA(snd)->ChunkIndex2);

        *addr = ASSOUNDDATA(snd)->Lock1Data;
        *len = ASSOUNDDATA(snd)->Lock1Length;

        return TRUE;
    }

    return FALSE;
}

// 0x1002b1d0
s32 RADLINK BinkDirectSoundUnlock(struct BINKSND PTR4* snd, u32 filled)
{
    if (ASSOUNDDATA(snd)->ChunkIndex2 != DEFAULT_SOUND_CHUNK_INDEX && ASSOUNDDATA(snd)->Lock1Data != NULL) {
        radmemset(ASSOUNDDATA(snd)->Lock1Data,
            ASSOUNDDATA(snd)->Bits == 16 ? 0x00 : 0x80, ASSOUNDDATA(snd)->Lock1Length - filled);

        BinkDirectSoundBufferUnlock(ASSOUNDDATA(snd));

        ASSOUNDDATA(snd)->ChunkIndex2 = DEFAULT_SOUND_CHUNK_INDEX;
        ASSOUNDDATA(snd)->Lock1Data = NULL;
        
        return TRUE;
    }

    return FALSE;
}

// 0x1002b250
void RADLINK BinkDirectSoundVolume(struct BINKSND PTR4* snd, s32 volume)
{
    if (VOLUME_NORMAL < volume) { volume = VOLUME_NORMAL; }

    ASSOUNDDATA(snd)->Volume = volume;

    BinkDirectSoundBufferInitialize(ASSOUNDDATA(snd));
}

// 0x1002b280
void RADLINK BinkDirectSoundPan(struct BINKSND PTR4* snd, s32 pan)
{
    if (pan > (PAN_RIGHT - 1)) { pan = PAN_RIGHT - 1; }

    ASSOUNDDATA(snd)->Pan = pan;

    BinkDirectSoundBufferInitialize(ASSOUNDDATA(snd));
}

// 0x1002b2b0
s32 RADLINK BinkDirectSoundOnOff(struct BINKSND PTR4* snd, s32 status)
{
    if (status) {
        if (!snd->OnOff) {
            snd->snddata[0x2c] = 0xff; // TODO
            snd->snddata[0x2d] = 0xff;
            snd->snddata[0x2e] = 0xff;
            snd->snddata[0x2f] = 0xff;

            ASSOUNDDATA(snd)->ChunkIndex2 = DEFAULT_SOUND_CHUNK_INDEX;

            snd->snddata[0x24] = '\0'; // TODO
            snd->snddata[0x25] = '\0';
            snd->snddata[0x26] = '\0';
            snd->snddata[0x27] = '\0';

            ASSOUNDDATA(snd)->Unk0x28 = FALSE;

            snd->OnOff = TRUE;
        }

        return snd->OnOff;
    }

    if (!status) {
        if (snd->OnOff) {
            ASSOUNDDATA(snd)->Buffer->Stop();
            ASSOUNDDATA(snd)->Buffer->SetCurrentPosition(0);

            BinkDirectSoundBufferFill(ASSOUNDDATA(snd));
            snd->OnOff = FALSE;
        }
    }

    return snd->OnOff;
}

// 0x1002b3b0
void RADLINK BinkDirectSoundClose(struct BINKSND PTR4* snd)
{
    if (snd != NULL) {
        BinkDirectSoundBufferStop(ASSOUNDDATA(snd));

        ASSOUNDDATA(snd)->Buffer->Release();
        ASSOUNDDATA(snd)->Buffer = NULL;

        BinkDirectSoundRelease();
    }
}

// 0x1002b340
s32 RADLINK BinkDirectSoundPause(struct BINKSND PTR4* snd, s32 status)
{
    if (status) {
        BinkDirectSoundBufferStop(ASSOUNDDATA(snd));
        return ASSOUNDDATA(snd)->IsStopped;
    }

    if (ASSOUNDDATA(snd)->Unk0x28) {
        BinkDirectSoundBufferPlay(ASSOUNDDATA(snd));
    }

    ASSOUNDDATA(snd)->IsStopped = FALSE;

    return ASSOUNDDATA(snd)->IsStopped;
}

// 0x1002b390
void BinkDirectSoundBufferStop(HBINKDIRECTSOUNDDATA snd)
{
    snd->Buffer->Stop();
    snd->IsStopped = TRUE;
}
