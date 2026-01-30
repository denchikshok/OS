#ifndef _WIN32

#include "ipc.h"
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

static int shm_fd = -1;

bool init_shared_memory(SharedData*& data) {
    shm_fd = shm_open("/laba3_shm", O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) return false;

    ftruncate(shm_fd, sizeof(SharedData));

    data = (SharedData*)mmap(
        nullptr,
        sizeof(SharedData),
        PROT_READ | PROT_WRITE,
        MAP_SHARED,
        shm_fd,
        0
    );

    if (data == MAP_FAILED) return false;

    return true;
}

void close_shared_memory() {
    if (shm_fd != -1)
        close(shm_fd);
}

#endif
