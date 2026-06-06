/*
The MIT License (MIT)

Copyright (c) 2026 Tomoki Shiono

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/


#include <windows.h>
#include "ThreadSetCursorPos.h"

static HANDLE l_threadSetCursorPos = NULL;
static HANDLE l_eventSetCursorPos = NULL;
static volatile bool l_abortSetCursorPos = false;
static POINT l_targetSetCursorPos = { 0, 0 };

DWORD WINAPI ThreadSetCursorPos(LPVOID lpParam)
{
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);     // Raise this thread's priority

    while (!l_abortSetCursorPos) {
        WaitForSingleObject(l_eventSetCursorPos, INFINITE);

        if (l_abortSetCursorPos)
            break;

        SetCursorPos(l_targetSetCursorPos.x, l_targetSetCursorPos.y);
    }

    return 0;
}

void CreateThreadSetCursorPos()
{
    l_abortSetCursorPos = false;

    l_eventSetCursorPos = CreateEvent(
        NULL,
        FALSE,
        FALSE,
        NULL);

    l_threadSetCursorPos = CreateThread(
        NULL,
        0,
        ThreadSetCursorPos,
        NULL,
        0,
        NULL);
}

void RequestSetCursorPos(int x, int y)
{
    l_targetSetCursorPos.x = x;
    l_targetSetCursorPos.y = y;

    SetEvent(l_eventSetCursorPos);
}

void DestroyThreadSetCursorPos()
{
    l_abortSetCursorPos = true;

    SetEvent(l_eventSetCursorPos);

    WaitForSingleObject(l_threadSetCursorPos, INFINITE);

    CloseHandle(l_threadSetCursorPos);
    l_threadSetCursorPos = NULL;

    CloseHandle(l_eventSetCursorPos);
    l_eventSetCursorPos = NULL;
}
