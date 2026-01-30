#include "mutex.h"
#include <windows.h>

static HANDLE hMutex = NULL;

bool init_mutex() {
    hMutex = CreateMutexA(NULL, FALSE, "Global\\Laba3Mutex");
    return hMutex != NULL;
}

void lock_mutex() {
    WaitForSingleObject(hMutex, INFINITE);
}

void unlock_mutex() {
    ReleaseMutex(hMutex);
}

void close_mutex() {
    if (hMutex)
        CloseHandle(hMutex);
}
