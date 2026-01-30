#ifdef _WIN32

#include "leader.h"
#include <windows.h>

static HANDLE hMutex = NULL;
static bool leader = false;

bool is_leader() {
    hMutex = CreateMutexA(NULL, TRUE, "Laba3LeaderMutex");

    if (!hMutex)
        return false;

    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        leader = false;
    } else {
        leader = true;
    }

    return leader;
}

void release_leader() {
    if (leader && hMutex) {
        ReleaseMutex(hMutex);
        CloseHandle(hMutex);
    }
}

#endif
