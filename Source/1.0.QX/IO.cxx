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

// 0x100011b0
s32 RADLINK BinkOpenFile(struct BINKIO PTR4* io, const char PTR4* name, u32 flags)
{
    radmemset(io, 0x00, sizeof(BINKIO));

    if (flags & BINKFILEHANDLE) {
        HANDLE handle = (HANDLE)name;

        ASIODATA(io)->Handle = handle;
        ASIODATA(io)->IsExternal = TRUE;
        ASIODATA(io)->FilePointer = SetFilePointer(handle, 0, NULL, FILE_CURRENT);
    }
    else {
        ASIODATA(io)->Handle = CreateFileA(name, GENERIC_READ,
            FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN | FILE_ATTRIBUTE_NORMAL, NULL);

        if (ASIODATA(io)->Handle == INVALID_HANDLE_VALUE) {
            ASIODATA(io)->Handle = CreateFileA(name, GENERIC_READ,
                FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, OPEN_EXISTING,
                FILE_FLAG_SEQUENTIAL_SCAN | FILE_ATTRIBUTE_NORMAL, NULL);
        }

        if (ASIODATA(io)->Handle == INVALID_HANDLE_VALUE) { return FALSE; }
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
    ASIODATA(io)->ReadCount = ASIODATA(io)->ReadCount + 1;

    while (ASIODATA(io)->Unk0x18 != 0) {
        Sleep(1);
        BinkService(io->bink);
    }

    if (offset != -1 && ASIODATA(io)->HeaderOffset != offset) {
        SetFilePointer(ASIODATA(io)->Handle, ASIODATA(io)->FilePointer + offset, NULL, FILE_BEGIN);
        ASIODATA(io)->HeaderOffset = offset;
    }

    DWORD read = 0;
    ReadFile(ASIODATA(io)->Handle, dest, size, &read, NULL);

    ASIODATA(io)->HeaderOffset = ASIODATA(io)->HeaderOffset + read;
    ASIODATA(io)->DataOffset = ASIODATA(io)->HeaderOffset;

    u32 buffSize = ASIODATA(io)->FileSize - ASIODATA(io)->HeaderOffset;

    if (io->BufSize <= buffSize) { buffSize = io->BufSize; }

    io->CurBufSize = buffSize;

    ASIODATA(io)->ReadCount = ASIODATA(io)->ReadCount - 1;

    return read;
}

// 0x10001320
u32 RADLINK BinkReadFileFrame(struct BINKIO PTR4* io, u32 frame, s32 origofs, void PTR4* dest, u32 size)
{
    // TODO refactor this!
    if (io->ReadError) { return 0; }

    u32 result = 0;
    u32 enter = FALSE;

    const u32 start = RADTimerRead();

    if (origofs != -1 && ASIODATA(io)->DataOffset != origofs) {
        enter = TRUE;

        ASIODATA(io)->ReadCount = ASIODATA(io)->ReadCount + 1;

        while (ASIODATA(io)->Unk0x18 != 0) {
            Sleep(1);
            BinkService(io->bink);
        }

        if (ASIODATA(io)->DataOffset < origofs && origofs <= ASIODATA(io)->HeaderOffset) {
            const s32 offset = origofs - ASIODATA(io)->DataOffset;

            ASIODATA(io)->DataOffset = origofs;
            ASIODATA(io)->BufferSize = ASIODATA(io)->BufferSize + offset;

            io->CurBufUsed = io->CurBufUsed - offset;

            ASIODATA(io)->Buffer = (void*)((size_t)ASIODATA(io)->Buffer + offset);

            if (ASIODATA(io)->Unk0x20 < ASIODATA(io)->Buffer) {
                ASIODATA(io)->Buffer = (void*)((size_t)ASIODATA(io)->Buffer - io->BufSize);
            }
        }
        else {
            SetFilePointer(ASIODATA(io)->Handle, ASIODATA(io)->FilePointer + origofs, NULL, FILE_BEGIN);

            ASIODATA(io)->HeaderOffset = origofs;
            ASIODATA(io)->DataOffset = origofs;
            ASIODATA(io)->BufferSize = io->BufSize;

            io->CurBufUsed = 0;

            ASIODATA(io)->Buffer = ASIODATA(io)->Unk0x1C;
            ASIODATA(io)->Unk0x24 = ASIODATA(io)->Unk0x1C;
        }
    }

    do {
        if (io->CurBufUsed != 0) {
            const u32 used = size < io->CurBufUsed ? size : io->CurBufUsed;

            size = size - used;

            ASIODATA(io)->DataOffset = ASIODATA(io)->DataOffset + used;

            result = result + used;

            const size_t length = (size_t)ASIODATA(io)->Unk0x20 - (size_t)ASIODATA(io)->Buffer;

            if (length <= used) {
                radmemcpy(dest, ASIODATA(io)->Buffer, length);

                dest = (void*)((size_t)dest + length);

                ASIODATA(io)->Buffer = ASIODATA(io)->Unk0x1C;

                io->CurBufUsed = io->CurBufUsed - length;

                ASIODATA(io)->BufferSize = ASIODATA(io)->BufferSize + length;

                if (used == length) goto LAB_100014ad; // TODO
            }

            radmemcpy(dest, ASIODATA(io)->Buffer, used);

            dest = (void*)((size_t)dest + used);

            ASIODATA(io)->Buffer = (void*)((size_t)ASIODATA(io)->Buffer + used);

            io->CurBufUsed = io->CurBufUsed - used;

            ASIODATA(io)->BufferSize = ASIODATA(io)->BufferSize + used;
        }

    LAB_100014ad:
        if (size == 0) {
            io->ForegroundTime = io->ForegroundTime + RADTimerRead() - start;

            const u32 length = ASIODATA(io)->FileSize - ASIODATA(io)->DataOffset;

            io->CurBufSize = io->BufSize <= length ? io->BufSize : length;

            if (io->CurBufSize < io->CurBufUsed + BINKIO_BUFFER_SIZE) {
                io->CurBufSize = io->CurBufUsed;
            }

            if (enter) { ASIODATA(io)->ReadCount = ASIODATA(io)->ReadCount - 1; }

            return result;
        }

        if (enter) {
            const u32 time = RADTimerRead();

            DWORD read = 0;
            ReadFile(ASIODATA(io)->Handle, dest, size, &read, NULL);

            if (read < size) { io->ReadError = TRUE; }

            ASIODATA(io)->HeaderOffset = ASIODATA(io)->HeaderOffset + read;
            ASIODATA(io)->DataOffset = ASIODATA(io)->DataOffset + read;

            io->BytesRead = io->BytesRead + read;

            result = result + read;

            if (ASIODATA(io)->SimulateRate != 0) { BinkReadFileSimulate(io, read, time); }

            const u32 end = RADTimerRead();

            io->TotalTime = io->TotalTime + end - time;
            io->ForegroundTime = io->ForegroundTime + end - time;

            const u32 length = ASIODATA(io)->FileSize - ASIODATA(io)->DataOffset;

            io->CurBufSize = io->BufSize <= length ? io->BufSize : length;

            if (io->CurBufSize < io->CurBufUsed + BINKIO_BUFFER_SIZE) { io->CurBufSize = io->CurBufUsed; }

            if (enter) { ASIODATA(io)->ReadCount = ASIODATA(io)->ReadCount - 1; }

            return result;
        }

        enter = TRUE;

        ASIODATA(io)->ReadCount = ASIODATA(io)->ReadCount + 1;

        while (ASIODATA(io)->Unk0x18 != 0) {
            Sleep(1);
            BinkService(io->bink);
        }
    } while (TRUE);
}

// 0x100015d0
void BinkReadFileSimulate(struct BINKIO PTR4* io, u32 size, u32 time)
{
    u32 start = RADTimerRead();

    ASIODATA(io)->SimulateDelay =
        ASIODATA(io)->SimulateDelay + (size * 1000 / ASIODATA(io)->SimulateRate) - start + time;

    while (0 < ASIODATA(io)->SimulateDelay) {
        u32 current = 0;

        do {
            Sleep(1);
            current = RADTimerRead();
        } while (current - start < ASIODATA(io)->SimulateDelay);

        ASIODATA(io)->SimulateDelay = ASIODATA(io)->SimulateDelay + start - current;

        start = current;
    }
}

// 0x10001660
u32 RADLINK BinkGetFileBufferSize(struct BINKIO PTR4* io, u32 size)
{
    ASIODATA(io)->ReadCount = ASIODATA(io)->ReadCount + 1;

    while (ASIODATA(io)->Unk0x18 != 0) {
        Sleep(1);
        BinkService(io->bink);
    }
    
    ASIODATA(io)->ReadCount = ASIODATA(io)->ReadCount - 1;

    return ALIGNBINKIOBUFFERSIZE(size);
}

// 0x100016c0
void RADLINK BinkSetFileInfo(struct BINKIO PTR4* io, void PTR4* buf, u32 size, u32 fileSize, u32 simulate)
{
    ASIODATA(io)->ReadCount = ASIODATA(io)->ReadCount + 1;

    while (ASIODATA(io)->Unk0x18 != 0) {
        Sleep(1);
        BinkService(io->bink);
    }

    const u32 length = size & BINKIO_BUFFER_SIZE_MASK;

    ASIODATA(io)->Unk0x1C = buf;
    ASIODATA(io)->Buffer = buf;
    ASIODATA(io)->Unk0x24 = buf;
    ASIODATA(io)->Unk0x20 = (void*)((size_t)buf + length);

    io->BufSize = length;
    
    ASIODATA(io)->BufferSize = length;

    io->CurBufUsed = 0;

    ASIODATA(io)->FileSize = fileSize;
    ASIODATA(io)->SimulateRate = simulate;

    ASIODATA(io)->ReadCount = ASIODATA(io)->ReadCount - 1;
}

// 0x10001740
void RADLINK BinkCloseFile(struct BINKIO PTR4* io)
{
    ASIODATA(io)->ReadCount = ASIODATA(io)->ReadCount + 1;

    while (ASIODATA(io)->Unk0x18 != 0) {
        Sleep(1);
        BinkService(io->bink);
    }

    if (!ASIODATA(io)->IsExternal) { CloseHandle(ASIODATA(io)->Handle); }

    ASIODATA(io)->ReadCount = ASIODATA(io)->ReadCount - 1;
}

// 0x100017a0
u32 RADLINK BinkReadFile(struct BINKIO PTR4* io)
{
    DWORD read = 0;
    BOOL wait = FALSE;

    const u32 working = io->Working;

    if (io->ReadError) { return 0; }

    ASIODATA(io)->Unk0x18 = ASIODATA(io)->Unk0x18 + 1;
    ASIODATA(io)->ReadCount = ASIODATA(io)->ReadCount + 1;

    if (ASIODATA(io)->Unk0x18 == 1 && ASIODATA(io)->ReadCount == 1) {
        if (ASIODATA(io)->BufferSize < BINKIO_BUFFER_SIZE
            || (ASIODATA(io)->FileSize - ASIODATA(io)->HeaderOffset < BINKIO_BUFFER_SIZE)) {
            io->CurBufSize = io->CurBufUsed;
        }
        else {
            const u32 start = RADTimerRead();

            io->DoingARead = TRUE;
            ReadFile(ASIODATA(io)->Handle, ASIODATA(io)->Unk0x24, BINKIO_BUFFER_SIZE, &read, NULL);
            io->DoingARead = FALSE;

            if (read < BINKIO_BUFFER_SIZE) { io->ReadError = TRUE; }

            io->BytesRead = io->BytesRead + read;

            ASIODATA(io)->HeaderOffset = ASIODATA(io)->HeaderOffset + read;
            ASIODATA(io)->Unk0x24 = (void*)((size_t)ASIODATA(io)->Unk0x24 + read);

            if (ASIODATA(io)->Unk0x20 <= ASIODATA(io)->Unk0x24) { ASIODATA(io)->Unk0x24 = ASIODATA(io)->Unk0x1C; }
            
            ASIODATA(io)->BufferSize = ASIODATA(io)->BufferSize - read;

            io->CurBufUsed = io->CurBufUsed + read;

            if (io->BufHighUsed < io->CurBufUsed) { io->BufHighUsed = io->CurBufUsed; }

            if (ASIODATA(io)->SimulateRate != 0) { BinkReadFileSimulate(io, read, start); }

            const u32 duration = RADTimerRead() - start;

            io->TotalTime = io->TotalTime + duration;

            if (!working && !io->Working) { io->IdleTime = io->IdleTime + duration; }
            else { io->ThreadTime = io->ThreadTime + duration; }
        }
    }
    else { wait = TRUE; read = INVALID_FILE_SIZE; }

    ASIODATA(io)->ReadCount = ASIODATA(io)->ReadCount - 1;
    ASIODATA(io)->Unk0x18 = ASIODATA(io)->Unk0x18 - 1;

    if (wait) { Sleep(1); }

    return read;
}