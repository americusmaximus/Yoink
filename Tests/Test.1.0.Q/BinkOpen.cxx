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

#include "BinkOpen.hxx"

#include <Yoink.hxx>

#include <Mem.hxx> // TODO Temp

#include <stdio.h>

typedef HBINK(RADEXPLINK *BINKOPENACTION)(const char* name, u32 flags);
typedef void(RADEXPLINK *BINKCLOSEACTION)(HBINK bnk);

#define MAX_MEM_ITEM_COUNT  128

#define BINK_MEM_SIZE_ADDRESS(X)    ((size_t)X + (0x10041b80 - BINK_BASE_ADDRESS))
#define BINK_MEM_COUNT_ADDRESS(X)   ((size_t)X + (0x10041b84 - BINK_BASE_ADDRESS))

#define BINK_MEM_SIZES_ADDRESS(X)   ((size_t)X + (0x1004167c - BINK_BASE_ADDRESS))
#define BINK_MEM_ITEMS_ADDRESS(X)   ((size_t)X + (0x1004187c - BINK_BASE_ADDRESS))

typedef struct BINKIODATA
{
    HANDLE          Handle;             // 0x0
    s32             HeaderOffset;       // 0x4 // TODO Name
    s32             DataOffset;         // 0x8 // TODO Name
    void*           Buffer;             // 0xC // TODO Name
    s32             BufferSize;         // 0x10 // TODO Name
    volatile s32    ReadCount;          // 0x14
    volatile s32    Unk0x18;            // 0x18 // TODO Some sort of lock
    void*           Unk0x1C;            // 0x1C // TODO Buffer
    void*           Unk0x20;            // 0x20 // TODO Buffer
    void*           Unk0x24;            // 0x24 // TODO Buffer
    u32             IsExternal;         // 0x28
    u32             FilePointer;        // 0x2c
    s32             FileSize;           // 0x30

    // This value is set in bytes per second,
    // use 150,000 for a 1xCD, 300,000 for a 2xCD, and 600,000 for a 4xCD. 
    u32             SimulateRate;       // 0x34
    u32             SimulateDelay;      // 0x38
    s32             Unk0x3C;            // 0x3C // TODO
} BINKIODATA, * BINKIODATAPTR;

#define ASIODATA(X) ((BINKIODATAPTR)(X->iodata))

static u32 CompareBink(HBINK a, HBINK b)
{
    u32 result = TRUE;

    result &= a->Width == b->Width;
    result &= a->Height == b->Height;
    result &= a->Frames == b->Frames;
    result &= a->FrameNum == b->FrameNum;
    result &= a->LastFrameNum == b->LastFrameNum;

    result &= a->FrameRate == b->FrameRate;
    result &= a->FrameRateDiv == b->FrameRateDiv;

    result &= a->ReadError == b->ReadError;
    result &= a->OpenFlags == b->OpenFlags;
    result &= a->BinkType == b->BinkType;

    result &= a->Size == b->Size;
    result &= a->FrameSize == b->FrameSize;
    result &= a->SndSize == b->SndSize;

    for (u32 x = 0; x < BINKMAXDIRTYRECTS; x++) {
        result &= a->FrameRects[x].Left == b->FrameRects[x].Left;
        result &= a->FrameRects[x].Top == b->FrameRects[x].Top;
        result &= a->FrameRects[x].Width == b->FrameRects[x].Width;
        result &= a->FrameRects[x].Height == b->FrameRects[x].Height;
    }

    result &= a->NumRects == b->NumRects;

    result &= a->PlaneNum == b->PlaneNum;
    result &= (a->YPlane == NULL && b->YPlane == NULL) || (a->YPlane != NULL && b->YPlane != NULL);

    // TODO Compare YPlane [0] and [1] content

    result &= (a->APlane == NULL && b->APlane == NULL) || (a->APlane != NULL && b->APlane != NULL);

    // TODO Compare APlane [0] and [1] content

    result &= a->YWidth == b->YWidth;
    result &= a->YHeight == b->YHeight;
    result &= a->UVWidth == b->UVWidth;
    result &= a->UVHeight == b->UVHeight;

    result &= (a->MaskPlane == NULL && b->MaskPlane == NULL) || (a->MaskPlane != NULL && b->MaskPlane != NULL);

    // TODO Compare MaskPlane

    result &= a->MaskPitch == b->MaskPitch;
    result &= a->MaskLength == b->MaskLength;

    result &= a->LargestFrameSize == b->LargestFrameSize;
    result &= a->InternalFrames == b->InternalFrames;

    result &= a->NumTracks == b->NumTracks;

    result &= a->Highest1SecRate == b->Highest1SecRate;
    result &= a->Highest1SecFrame == b->Highest1SecFrame;

    result &= a->Paused == b->Paused;

    result &= (a->BackgroundThread == NULL && b->BackgroundThread == NULL) || (a->BackgroundThread != NULL && b->BackgroundThread != NULL);

    // TODO Compare BackgroundThread

    result &= (a->compframe == NULL && b->compframe == NULL) || (a->compframe != NULL && b->compframe != NULL);

    // TODO Compare compframe

    result &= (a->preloadptr == NULL && b->preloadptr == NULL) || (a->preloadptr != NULL && b->preloadptr != NULL);

    // TODO Compare preloadptr

    result &= (a->frameoffsets == NULL && b->frameoffsets == NULL) || (a->frameoffsets != NULL && b->frameoffsets != NULL);

    if (a->frameoffsets != NULL && b->frameoffsets != NULL) {
        for (u32 x = 0; x < a->Frames; x++) {
            result &= a->frameoffsets[x] == b->frameoffsets[x];
        }
    }

    // IO
    result &= a->bio.bink == a && b->bio.bink == b;
    result &= a->bio.ReadError == b->bio.ReadError;
    result &= a->bio.DoingARead == b->bio.DoingARead;
    result &= a->bio.BytesRead == b->bio.BytesRead;
    result &= a->bio.Working == b->bio.Working;
    //result &= a->bio.TotalTime == b->bio.TotalTime;
    //result &= a->bio.ForegroundTime == b->bio.ForegroundTime;
    //result &= a->bio.IdleTime == b->bio.IdleTime;;
    //result &= a->bio.ThreadTime == b->bio.ThreadTime;
    result &= a->bio.BufSize == b->bio.BufSize;
    result &= a->bio.BufHighUsed == b->bio.BufHighUsed;
    result &= a->bio.CurBufSize == b->bio.CurBufSize;
    result &= a->bio.CurBufUsed == b->bio.CurBufUsed;

    {
        BINKIODATAPTR aio = (BINKIODATAPTR)a->bio.iodata;
        BINKIODATAPTR bio = (BINKIODATAPTR)b->bio.iodata;

        result &= (aio->Handle == NULL && bio->Handle == NULL)
            || (aio->Handle != NULL && bio->Handle != NULL);

        result &= aio->HeaderOffset == bio->HeaderOffset;
        result &= aio->DataOffset == bio->DataOffset;

        result &= (aio->Buffer == NULL && bio->Buffer == NULL)
            || (aio->Buffer != NULL && bio->Buffer != NULL);

        // TODO Buffer content

        result &= aio->BufferSize == bio->BufferSize;
        result &= aio->ReadCount == bio->ReadCount;
        result &= aio->Unk0x18 == bio->Unk0x18;

        result &= (aio->Unk0x1C == NULL && bio->Unk0x1C == NULL)
            || (aio->Unk0x1C != NULL && bio->Unk0x1C != NULL);

        result &= (aio->Unk0x20 == NULL && bio->Unk0x20 == NULL)
            || (aio->Unk0x20 != NULL && bio->Unk0x20 != NULL);

        result &= (aio->Unk0x24 == NULL && bio->Unk0x24 == NULL)
            || (aio->Unk0x24 != NULL && bio->Unk0x24 != NULL);

        result &= aio->IsExternal == bio->IsExternal;
        result &= aio->FilePointer == bio->FilePointer;
        result &= aio->FileSize == bio->FileSize;
        result &= aio->SimulateRate == bio->SimulateRate;
        result &= aio->SimulateDelay == bio->SimulateDelay;
        result &= aio->Unk0x3C == bio->Unk0x3C;
    }

    result &= (a->ioptr == NULL && b->ioptr == NULL) || (a->ioptr != NULL && b->ioptr != NULL);

    // TODO Compare ioptr

    result &= a->iosize == b->iosize;
    result &= a->decompwidth == b->decompwidth;
    result &= a->decompheight == b->decompheight;

    result &= a->trackindex == b->trackindex;

    result &= (a->tracksizes == NULL && b->tracksizes == NULL) || (a->tracksizes != NULL && b->tracksizes != NULL);

    if (a->tracksizes != NULL && b->tracksizes != NULL) {
        for (s32 x = 0; x < a->NumTracks; x++) {
            result &= a->tracksizes[x] == b->tracksizes[x];
        }
    }

    result &= (a->tracktypes == NULL && b->tracktypes == NULL) || (a->tracktypes != NULL && b->tracktypes != NULL);

    if (a->tracktypes != NULL && b->tracktypes != NULL) {
        for (s32 x = 0; x < a->NumTracks; x++) {
            result &= a->tracktypes[x] == b->tracktypes[x];
        }
    }

    result &= (a->trackIDs == NULL && b->trackIDs == NULL) || (a->trackIDs != NULL && b->trackIDs != NULL);

    if (a->trackIDs != NULL && b->trackIDs != NULL) {
        for (s32 x = 0; x < a->NumTracks; x++) {
            result &= a->trackIDs[x] == b->trackIDs[x];
        }
    }

    result &= a->numrects == b->numrects;

    result &= a->playedframes == b->playedframes;
    result &= a->firstframetime == b->firstframetime;
    result &= a->startframetime == b->startframetime;
    result &= a->startblittime == b->startblittime;
    result &= a->startsynctime == b->startsynctime;
    result &= a->startsyncframe == b->startsyncframe;
    result &= a->twoframestime == b->twoframestime;
    result &= a->entireframetime == b->entireframetime;

    result &= a->slowestframetime == b->slowestframetime;
    result &= a->slowestframe == b->slowestframe;
    result &= a->slowest2frametime == b->slowest2frametime;
    result &= a->slowest2frame == b->slowest2frame;

    result &= a->soundon == b->soundon;
    result &= a->videoon == b->videoon;

    result &= a->totalmem == b->totalmem;
    result &= a->timevdecomp == b->timevdecomp;
    result &= a->timeadecomp == b->timeadecomp;
    result &= a->timeblit == b->timeblit;

    // result &= a->timeopen == b->timeopen; // Will not match, time sensitive!

    result &= a->fileframerate == b->fileframerate;
    result &= a->fileframeratediv == b->fileframeratediv;

    result &= a->threadcontrol == b->threadcontrol;

    result &= a->runtimeframes == b->runtimeframes;
    result &= a->runtimemoveamt == b->runtimemoveamt;

    result &= (a->rtframetimes == NULL && b->rtframetimes == NULL) || (a->rtframetimes != NULL && b->rtframetimes != NULL);

    if (a->rtframetimes != NULL && b->rtframetimes != NULL) {
        for (u32 x = 0; x < a->runtimeframes; x++) {
            //result &= a->rtframetimes[x] == b->rtframetimes[x]; // TODO Breaks here!
        }
    }

    result &= (a->rtadecomptimes == NULL && b->rtadecomptimes == NULL) || (a->rtadecomptimes != NULL && b->rtadecomptimes != NULL);

    if (a->rtadecomptimes != NULL && b->rtadecomptimes != NULL) {
        for (u32 x = 0; x < a->runtimeframes; x++) {
            //result &= a->rtadecomptimes[x] == b->rtadecomptimes[x]; // TODO Breaks here!
        }
    }

    // TODO
    //u32 PTR4* rtvdecomptimes;   // decompress times for runtime frames
    //u32 PTR4* rtblittimes;      // blit times for runtime frames
    //u32 PTR4* rtreadtimes;      // read times for runtime frames
    //u32 PTR4* rtidlereadtimes;  // idle read times for runtime frames
    //u32 PTR4* rtthreadreadtimes;// thread read times for runtime frames

    result &= a->lastblitflags == b->lastblitflags;
    result &= a->lastdecompframe == b->lastdecompframe;

    result &= a->sndbufsize == b->sndbufsize;

    // TODO
    //u8 PTR4* sndbuf;            // sound buffer
    //u8 PTR4* sndend;            // end of the sound buffer
    //u8 PTR4* sndwritepos;       // current write position
    //u8 PTR4* sndreadpos;        // current read position
    //void* sndcomp;              // sound compression handle

    result &= a->sndamt == b->sndamt;
    result &= a->sndreenter == b->sndreenter;
    result &= a->sndconvert8 == b->sndconvert8;

    // TODO
    //BINKSND bsnd;               // SND structure

    result &= a->skippedlastblit == b->skippedlastblit;
    result &= a->skippedblits == b->skippedblits;
    result &= a->soundskips == b->soundskips;
    result &= a->sndendframe == b->sndendframe;
    result &= a->sndprime == b->sndprime;
    result &= a->sndpad == b->sndpad;

    // TODO
    //BUNDLEPOINTERS bunp;        // pointers to internal temporary memory

    return result;
}

static u32 CompareBink(HMODULE bink, HBINK a, HBINK b)
{
    if (!CompareBink(a, b)) { return FALSE; }

    // Memory Allocator State
    {
        const size_t oms = *(size_t*)BINK_MEM_SIZE_ADDRESS(bink);
        const size_t ims = MemSize;

        const u32 omc = *(u32*)BINK_MEM_COUNT_ADDRESS(bink);
        const u32 imc = MemCount;

        const size_t* omss = (size_t*)BINK_MEM_SIZES_ADDRESS(bink);

        for (u32 x = 0; x < MAX_MEM_ITEM_COUNT; x++) {
            const size_t o = omss[x];
            const size_t i = MemSizes[x];

            if (o != i) { return FALSE; }
        }
    }

    return TRUE;
}

static u32 Execute(HMODULE bink, const char* name, u32 flags)
{
    BINKOPENACTION open = (BINKOPENACTION)GetProcAddress(bink, "_BinkOpen@8");
    BINKCLOSEACTION close = (BINKCLOSEACTION)GetProcAddress(bink, "_BinkClose@4");

    HBINK o = open(name, flags);
    HBINK i = BinkOpen(name, flags);

    u32 result = CompareBink(bink, o, i);

    close(o);
    BinkClose(i);
    
    return result;
}

#define TEST(N, O) if (!Execute(bink, N, O)) { strcpy(message, #N); return FALSE; }

int ExecuteBinkOpen(HMODULE bink, char* message)
{
    TEST("..\\Content\\az.bik", 0);
    TEST("..\\Content\\cdv.bik", 0);
    TEST("..\\Content\\gwi.bik", 0);
    TEST("..\\Content\\thq.bik", 0);
    TEST((const char*)BinkLogoAddress(), BINKFROMMEMORY);

    return TRUE;
}