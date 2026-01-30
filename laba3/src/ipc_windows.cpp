#ifdef _WIN32

#include "ipc.h"
#include <windows.h>

static HANDLE hMap = NULL;

bool init_shared_memory(SharedData*& data) {
    hMap = CreateFileMappingA(
        INVALID_HANDLE_VALUE,
        NULL,
        PAGE_READWRITE,
        0,
        sizeof(SharedData),
        "Laba3SharedMemory"
    );

    if (!hMap) return false;

    data = (SharedData*)MapViewOfFile(
        hMap,
        FILE_MAP_ALL_ACCESS,
        0, 0,
        sizeof(SharedData)
    );

    if (!data) return false;

    // если первый процесс — обнуляем
    if (GetLastError() != ERROR_ALREADY_EXISTS) {
        data->counter = 0;
    }

    return true;
}

void close_shared_memory() {
    if (hMap) CloseHandle(hMap);
}

#endif
