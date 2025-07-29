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

#include <Yoink.hxx>
#include <State.hxx>

#define MAX_MODULE_NAME_PATH_LENGTH 128

// 0x100010d0
BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID)
{
    if (reason != DLL_PROCESS_ATTACH) { return TRUE; }

    Instance = instance;

    char name[MAX_MODULE_NAME_PATH_LENGTH];
    GetModuleFileNameA(instance, name, MAX_MODULE_NAME_PATH_LENGTH);

    const u32 length = radstrlen(name);

    for (u32 x = length; x != 0; x--) {
        if (name[x] == ':') { break; } // Stop when disk identifier is found, i.e. C:

        if (name[x] == '\\') { name[x] = NULL; break; } // Truncate path at the last backslash.
    }

    char directory[MAX_MODULE_NAME_PATH_LENGTH];
    GetWindowsDirectoryA(directory, MAX_MODULE_NAME_PATH_LENGTH);

    if (radstricmp(name, directory) != 0) {
        GetSystemDirectoryA(directory, MAX_MODULE_NAME_PATH_LENGTH);

        if (radstricmp(name, directory) != 0) { return TRUE; }
    }

    MessageBoxA(NULL, "The BinkW32.DLL file is incorrectly installed in the Windows or Windows system directory.", "Bink Error", MB_ICONHAND | MB_OK);

    return FALSE;
}