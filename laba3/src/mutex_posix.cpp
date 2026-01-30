#include "mutex.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <pthread.h>

static pthread_mutex_t* mutex = nullptr;

bool init_mutex() {
    int fd = shm_open("/laba3_mutex", O_CREAT | O_RDWR, 0666);
    if (fd < 0) return false;

    ftruncate(fd, sizeof(pthread_mutex_t));

    void* mem = mmap(nullptr, sizeof(pthread_mutex_t),
                     PROT_READ | PROT_WRITE,
                     MAP_SHARED, fd, 0);

    mutex = (pthread_mutex_t*)mem;

    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);

    pthread_mutex_init(mutex, &attr);
    return true;
}

void lock_mutex() {
    pthread_mutex_lock(mutex);
}

void unlock_mutex() {
    pthread_mutex_unlock(mutex);
}

void close_mutex() {
    // не удаляем, т.к. другие процессы могут использовать
}
