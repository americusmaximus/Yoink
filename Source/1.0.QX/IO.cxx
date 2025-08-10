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

#define BINKIO_BUFFER_SIZE          0x1000
#define BINKIO_BUFFER_SIZE_MASK     0xFFFFF000
#define ALIGNBINKIOBUFFERSIZE(X)    ((X + (BINKIO_BUFFER_SIZE - 1)) & BINKIO_BUFFER_SIZE_MASK)

BINKIOOPEN  IO;
u32         IOSize = DEFAULT_IO_SIZE;

typedef struct BINKIODATA
{
    HANDLE          Handle;             // 0x0
    s32             Offset;             // 0x4
    s32             Unk0x8;             // 0x8  // TODO Another file offset
    void*           Unk0xC;             // 0xC  // TODO Buffer
    s32             BufSize;            // 0x10
    volatile s8     Unk0x14;            // 0x14 // TODO Some sort of lock
    volatile s8     Unk0x18;            // 0x18 // TODO Some sort of lock
    void*           Unk0x1C;            // 0x1C // TODO Buffer
    void*           Unk0x20;            // 0x20 // TODO Buffer
    void*           Unk0x24;            // 0x24 // TODO Buffer
    u32             IsExternal;         // 0x28
    u32             Pointer;            // 0x2c
    s32             Size;               // 0x30

    // This value is set in bytes per second,
    // use 150,000 for a 1xCD, 300,000 for a 2xCD, and 600,000 for a 4xCD. 
    u32             Simulate;           // 0x34

    u32             Unk0x38;            // 0x38 // TODO Simulate delay
    s32             Unk0x3C;            // 0x3C // TODO


    // TODO
} BINKIODATA, * BINKIODATAPTR;

#define ASIODATA(X) ((BINKIODATAPTR)(X->iodata))

// 0x100011b0
s32 RADLINK BinkOpenFile(struct BINKIO PTR4* io, const char PTR4* name, u32 flags)
{
    radmemset(io, 0x00, sizeof(BINKIO));

    if (!(flags & BINKFILEHANDLE)) {
        ASIODATA(io)->Handle = CreateFileA(name, GENERIC_READ,
            FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN | FILE_ATTRIBUTE_NORMAL, NULL);

        if (ASIODATA(io)->Handle == INVALID_HANDLE_VALUE) {
            ASIODATA(io)->Handle = CreateFileA(name, GENERIC_READ,
                FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, OPEN_EXISTING,
                FILE_FLAG_SEQUENTIAL_SCAN | FILE_ATTRIBUTE_NORMAL, NULL);
        }

        if (ASIODATA(io)->Handle == INVALID_HANDLE_VALUE) { return FALSE; }
    }
    else {
        HANDLE handle = (HANDLE)name;

        ASIODATA(io)->Handle = handle;
        ASIODATA(io)->IsExternal = TRUE;
        ASIODATA(io)->Pointer = SetFilePointer(handle, 0, NULL, FILE_CURRENT);
    }

    io->ReadHeader      = BinkReadFileHeader;
    io->ReadFrame       = BinkReadFileFrame;
    io->GetBufferSize   = BinkGetFileBufferSize;
    io->SetInfo         = BinkSetFileInfo;
    io->Idle            = BinkReadFile;
    io->Close           = BinkCloseFile;

    return TRUE;
}

// 0x10001270
u32 RADLINK BinkReadFileHeader(struct BINKIO PTR4* io, s32 offset, void PTR4* dest, u32 size)
{
    ASIODATA(io)->Unk0x14++;

    while (ASIODATA(io)->Unk0x18) {
        Sleep(1);
        BinkService(io->bink);
    }

    if (offset != -1 && ASIODATA(io)->Offset != offset) {
        SetFilePointer(ASIODATA(io)->Handle, ASIODATA(io)->Pointer + offset, NULL, FILE_BEGIN);
        ASIODATA(io)->Offset = offset;
    }

    DWORD read = 0;
    ReadFile(ASIODATA(io)->Handle, dest, size, &read, NULL);

    ASIODATA(io)->Offset = ASIODATA(io)->Offset + read;
    ASIODATA(io)->Unk0x8 = ASIODATA(io)->Offset;
    
    u32 buffSize = ASIODATA(io)->Size - ASIODATA(io)->Offset;

    if (io->BufSize <= buffSize) { buffSize = io->BufSize; }

    io->CurBufSize = buffSize;

    ASIODATA(io)->Unk0x14--;

    return read;
}

// 0x10001320
u32 RADLINK BinkReadFileFrame(struct BINKIO PTR4* io, u32 frame, s32 origofs, void PTR4* dest, u32 size)
{
    /*
    TODO

    bVar4 = false;
    local_8 = 0;

    if (io->ReadError) { return 0; }

    const u32 start = RADTimerRead();
    iVar3 = origofs;

    if ((origofs != -1) && ((io->iodata).Unk0x8 != origofs)) {
        origofs = (int)&(io->iodata).Unk0x14;
        bVar4 = true;
        LOCK();
        *(char*)origofs = *(char*)origofs + '\x01';
        UNLOCK();

        while (ASIODATA(io)->Unk0x18) {
            Sleep(1);
            BinkService(io->bink);
        }

        uVar6 = (io->iodata).Unk0x8;
        if ((uVar6 < (uint)iVar3) && ((uint)iVar3 <= (uint)(io->iodata).Offset)) {
            io = (BINKIO*)(iVar3 - uVar6);
            (io->iodata).Unk0x8 = iVar3;
            (io->iodata).BufSize = (int)&io->ReadHeader + (io->iodata).BufSize;
            io->CurBufUsed = io->CurBufUsed - (int)io;
            uVar6 = (int)&io->ReadHeader + io->iodata.Unk0xC;
            (io->iodata).Unk0xC = uVar6;

            if ((uint)(io->iodata).Unk0x20 < uVar6) {
                (io->iodata).Unk0xC = uVar6 - io->BufSize;
            }
        }
        else {
            SetFilePointer(ASIODATA(io)->Handle, ASIODATA(io)->Pointer + iVar3, NULL, FILE_BEGIN);

            (io->iodata).Offset = iVar3;
            (io->iodata).Unk0x8 = iVar3;
            (io->iodata).BufSize = io->BufSize;

            io->CurBufUsed = 0;

            uVar2 = (io->iodata).Unk0x1C;
            (io->iodata).Unk0xC = uVar2;
            (io->iodata).Unk0x24 = uVar2;
        }
    }

    do {
        if (io->CurBufUsed != 0) {
            uVar6 = io->CurBufUsed;
            if (size < io->CurBufUsed) {
                uVar6 = size;
            }
            puVar9 = (undefined4*)(io->iodata).Unk0xC;
            size = size - uVar6;
            puVar10 = &(io->iodata).Unk0x8;
            *puVar10 = *puVar10 + uVar6;
            origofs = (io->iodata).Unk0x20 - (int)puVar9;
            local_8 = local_8 + uVar6;
            if ((uint)origofs <= uVar6) {
                puVar10 = (undefined4*)dest;
                for (uVar7 = (uint)origofs >> 2; uVar7 != 0; uVar7 = uVar7 - 1) {
                    *puVar10 = *puVar9;
                    puVar9 = puVar9 + 1;
                    puVar10 = puVar10 + 1;
                }
                for (uVar7 = origofs & 3; uVar7 != 0; uVar7 = uVar7 - 1) {
                    *(undefined*)puVar10 = *(undefined*)puVar9;
                    puVar9 = (undefined4*)((int)puVar9 + 1);
                    puVar10 = (undefined4*)((int)puVar10 + 1);
                }
                dest = (void*)((int)dest + origofs);
                (io->iodata).Unk0xC = (io->iodata).Unk0x1C;
                uVar6 = uVar6 - origofs;
                io->CurBufUsed = io->CurBufUsed - origofs;
                (io->iodata).BufSize = (io->iodata).BufSize + origofs;
                if (uVar6 == 0) goto LAB_100014ad;
            }
            puVar10 = (undefined4*)(io->iodata).Unk0xC;
            puVar9 = (undefined4*)dest;
            for (uVar7 = uVar6 >> 2; uVar7 != 0; uVar7 = uVar7 - 1) {
                *puVar9 = *puVar10;
                puVar10 = puVar10 + 1;
                puVar9 = puVar9 + 1;
            }
            for (uVar7 = uVar6 & 3; uVar7 != 0; uVar7 = uVar7 - 1) {
                *(undefined*)puVar9 = *(undefined*)puVar10;
                puVar10 = (undefined4*)((int)puVar10 + 1);
                puVar9 = (undefined4*)((int)puVar9 + 1);
            }
            dest = (void*)((int)dest + uVar6);
            (io->iodata).Unk0xC = (io->iodata).Unk0xC + uVar6;
            io->CurBufUsed = io->CurBufUsed - uVar6;
            (io->iodata).BufSize = (io->iodata).BufSize + uVar6;
        }
    LAB_100014ad:
        if (size == 0) {
            uVar6 = RADTimerRead();
            uVar5 = (uVar6 - start) + io->ForegroundTime;
            goto LAB_1000157f;
        }
        if (bVar4) {
            uVar7 = RADTimerRead();

            DWORD read = 0;
            ReadFile(ASIODATA(io)->Handle, dest, size, &read, NULL);

            if (read < size) { io->ReadError = TRUE; }

            ASIODATA(io)->Offset = (int)&io->ReadHeader + ASIODATA(io)->Offset;
            puVar10 = &(io->iodata).Unk0x8;
            *puVar10 = (int)&io->ReadHeader + *puVar10;
            io->BytesRead = (int)&io->ReadHeader + io->BytesRead;

            local_8 = (int)&io->ReadHeader + local_8;

            if (ASIODATA(io)->Simulate) { BinkReadFileSimulate(io, read, uVar7); }

            uVar6 = RADTimerRead();
            io->TotalTime = (uVar6 - uVar7) + io->TotalTime;
            uVar5 = (uVar6 - uVar5) + io->ForegroundTime;
        LAB_1000157f:
            io->ForegroundTime = uVar5;
            uVar5 = (io->iodata).Size - (io->iodata).Unk0x8;
            if (io->BufSize <= uVar5) {
                uVar5 = io->BufSize;
            }
            io->CurBufSize = uVar5;
            if (io->CurBufSize < io->CurBufUsed + BINKIO_BUFFER_SIZE) {
                io->CurBufSize = io->CurBufUsed;
            }
            if (bVar4) {
                pcVar8 = &(io->iodata).Unk0x14;
                LOCK();
                *pcVar8 = *pcVar8 + -1;
                UNLOCK();
            }
            return local_8;
        }
        origofs = (int)&(io->iodata).Unk0x14;
        bVar4 = true;
        LOCK();
        *(char*)origofs = *(char*)origofs + '\x01';
        UNLOCK();

        while (ASIODATA(io)->Unk0x18) {
            Sleep(1);
            BinkService(io->bink);
        }
    } while (true);
    */
    return 0; // TODO
}

// 0x100015d0
void BinkReadFileSimulate(struct BINKIO PTR4* io, u32 size, u32 time)
{
    u32 start = RADTimerRead();

    ASIODATA(io)->Unk0x38 =
        ASIODATA(io)->Unk0x38 + (size * 1000 / ASIODATA(io)->Simulate) - start + time;

    while (0 < ASIODATA(io)->Unk0x38) {
        u32 current = 0;

        do {
            Sleep(1);
            current = RADTimerRead();
        } while (current - start < ASIODATA(io)->Unk0x38);

        ASIODATA(io)->Unk0x38 = ASIODATA(io)->Unk0x38 + start - current;

        start = current;
    }
}

// 0x10001660
u32 RADLINK BinkGetFileBufferSize(struct BINKIO PTR4* io, u32 size)
{
    ASIODATA(io)->Unk0x14++;

    while (ASIODATA(io)->Unk0x18) {
        Sleep(1);
        BinkService(io->bink);
    }
    
    ASIODATA(io)->Unk0x14--;

    return ALIGNBINKIOBUFFERSIZE(size);
}

// 0x100016c0
void RADLINK BinkSetFileInfo(struct BINKIO PTR4* io, void PTR4* buf, u32 size, u32 fileSize, u32 simulate)
{
    ASIODATA(io)->Unk0x14++;

    while (ASIODATA(io)->Unk0x18) {
        Sleep(1);
        BinkService(io->bink);
    }

    const u32 length = size & BINKIO_BUFFER_SIZE_MASK;

    ASIODATA(io)->Unk0x1C = buf;
    ASIODATA(io)->Unk0xC = buf;
    ASIODATA(io)->Unk0x24 = buf;
    ASIODATA(io)->Unk0x20 = (void*)((size_t)buf + length);

    io->BufSize = length;
    
    ASIODATA(io)->BufSize = length;

    io->CurBufUsed = 0;

    ASIODATA(io)->Size = fileSize;
    ASIODATA(io)->Simulate = simulate;

    ASIODATA(io)->Unk0x14--;
}

// 0x10001740
void RADLINK BinkCloseFile(struct BINKIO PTR4* io)
{
    ASIODATA(io)->Unk0x14++;

    while (ASIODATA(io)->Unk0x18) {
        Sleep(1);
        BinkService(io->bink);
    }

    if (!ASIODATA(io)->IsExternal) { CloseHandle(ASIODATA(io)->Handle); }

    ASIODATA(io)->Unk0x14--;
}

// 0x100017a0
u32 RADLINK BinkReadFile(struct BINKIO PTR4* io)
{
    DWORD read = 0;
    BOOL wait = FALSE;

    const u32 working = io->Working;

    if (io->ReadError) { return 0; }

    ASIODATA(io)->Unk0x18++;
    ASIODATA(io)->Unk0x14++;

    if (ASIODATA(io)->Unk0x18 == 1 && ASIODATA(io)->Unk0x14 == 1) {
        if (ASIODATA(io)->BufSize < BINKIO_BUFFER_SIZE
            || (ASIODATA(io)->Size - ASIODATA(io)->Offset < BINKIO_BUFFER_SIZE)) {
            io->CurBufSize = io->CurBufUsed;
        }
        else {
            const u32 start = RADTimerRead();

            io->DoingARead = TRUE;
            ReadFile(ASIODATA(io)->Handle, ASIODATA(io)->Unk0x24, BINKIO_BUFFER_SIZE, &read, NULL);
            io->DoingARead = FALSE;

            if (read < BINKIO_BUFFER_SIZE) { io->ReadError = TRUE; }

            io->BytesRead = io->BytesRead + read;

            ASIODATA(io)->Offset = ASIODATA(io)->Offset + read;
            ASIODATA(io)->Unk0x24 = (void*)((size_t)ASIODATA(io)->Unk0x24 + read);

            if (ASIODATA(io)->Unk0x20 <= ASIODATA(io)->Unk0x24) { ASIODATA(io)->Unk0x24 = ASIODATA(io)->Unk0x1C; }
            
            ASIODATA(io)->BufSize = ASIODATA(io)->BufSize - read;
            io->CurBufUsed = io->CurBufUsed + read;

            if (io->BufHighUsed < io->CurBufUsed) { io->BufHighUsed = io->CurBufUsed; }

            if (ASIODATA(io)->Simulate) { BinkReadFileSimulate(io, read, start); }

            const u32 duration = RADTimerRead() - start;

            io->TotalTime = io->TotalTime + duration;

            if (!working && !io->Working) { io->IdleTime = io->IdleTime + duration; }
            else { io->ThreadTime = io->ThreadTime + duration; }
        }
    }
    else { wait = TRUE; read = INVALID_FILE_SIZE; }

    ASIODATA(io)->Unk0x14--;
    ASIODATA(io)->Unk0x18--;

    if (wait) { Sleep(1); }

    return read;
}