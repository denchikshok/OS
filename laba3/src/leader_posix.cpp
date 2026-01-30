#ifndef _WIN32

#include "leader.h"
#include <semaphore.h>
#include <fcntl.h>

static sem_t* sem = nullptr;
static bool leader = false;

bool is_leader() {
    sem = sem_open("/laba3_leader", O_CREAT | O_EXCL, 0666, 1);

    if (sem != SEM_FAILED) {
        leader = true;
    } else {
        sem = sem_open("/laba3_leader", 0);
        leader = false;
    }

    return leader;
}

void release_leader() {
    if (leader && sem) {
        sem_close(sem);
        sem_unlink("/laba3_leader");
    }
}

#endif
