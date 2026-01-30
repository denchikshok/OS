#pragma once

struct SharedData {
    int counter;
};

bool init_shared_memory(SharedData*& data);
void close_shared_memory();
