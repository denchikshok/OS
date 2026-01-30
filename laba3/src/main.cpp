#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <atomic>

#include "ipc.h"
#include "leader.h"
#include "mutex.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/wait.h>
#endif

struct ChildProcess {
    bool running = false;
#ifdef _WIN32
    HANDLE handle = NULL;
#else
    pid_t pid = -1;
#endif
};

// --------------------------------------------------
// PID
// --------------------------------------------------
int get_pid() {
#ifdef _WIN32
    return GetCurrentProcessId();
#else
    return getpid();
#endif
}

// --------------------------------------------------
// Текущее время с миллисекундами
// --------------------------------------------------
std::string current_time() {
    using namespace std::chrono;

    auto now = system_clock::now();
    auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

    std::time_t tt = system_clock::to_time_t(now);
    std::tm tm{};

#ifdef _WIN32
    localtime_s(&tm, &tt);
#else
    localtime_r(&tt, &tm);
#endif

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S")
        << "." << std::setw(3) << std::setfill('0') << ms.count();

    return oss.str();
}

// --------------------------------------------------
// Запуск дочернего процесса
// --------------------------------------------------
void start_child(const std::string& arg, ChildProcess& child) {
#ifdef _WIN32
    std::string cmd = "laba3.exe " + arg;

    STARTUPINFOA si{};
    PROCESS_INFORMATION pi{};
    si.cb = sizeof(si);

    if (CreateProcessA(
            NULL,
            cmd.data(),
            NULL, NULL, FALSE,
            0, NULL, NULL,
            &si, &pi)) {

        CloseHandle(pi.hThread);

        child.handle = pi.hProcess;
        child.running = true;
    }
#else
    pid_t pid = fork();
    if (pid == 0) {
        execl("./laba3", "laba3", arg.c_str(), nullptr);
        exit(0);
    } else if (pid > 0) {
        child.pid = pid;
        child.running = true;
    }
#endif
}

bool is_child_finished(ChildProcess& child) {
    if (!child.running)
        return true;

#ifdef _WIN32
    DWORD result = WaitForSingleObject(child.handle, 0);
    if (result == WAIT_OBJECT_0) {
        CloseHandle(child.handle);
        child.running = false;
        return true;
    }
    return false;
#else
    int status;
    pid_t res = waitpid(child.pid, &status, WNOHANG);
    if (res == child.pid) {
        child.running = false;
        return true;
    }
    return false;
#endif
}


// --------------------------------------------------
// MAIN
// --------------------------------------------------
int main(int argc, char* argv[]) {

    // --------------------------------------------------
    // Инициализация IPC
    // --------------------------------------------------
    SharedData* data = nullptr;

    if (!init_shared_memory(data)) {
        std::cerr << "Shared memory init failed\n";
        return 1;
    }

    init_mutex();

    bool leader = is_leader();

    // --------------------------------------------------
    // Открываем лог
    // --------------------------------------------------
    {
        lock_mutex();
        std::ofstream log("laba3.log", std::ios::app);
        log << "[" << current_time() << "] PID=" << get_pid()
            << " START";

        if (argc > 1)
            log << " " << argv[1];

        log << std::endl;
        unlock_mutex();
    }

    // --------------------------------------------------
    // CHILD 1
    // --------------------------------------------------
    if (argc > 1 && std::string(argv[1]) == "--child1") {
        lock_mutex();
        data->counter += 10;
        unlock_mutex();

        lock_mutex();
        std::ofstream log("laba3.log", std::ios::app);
        log << "[" << current_time() << "] PID=" << get_pid()
            << " EXIT --child1" << std::endl;
        unlock_mutex();

        return 0;
    }

    // --------------------------------------------------
    // CHILD 2
    // --------------------------------------------------
    if (argc > 1 && std::string(argv[1]) == "--child2") {
        lock_mutex();
        data->counter *= 2;
        unlock_mutex();

        std::this_thread::sleep_for(std::chrono::seconds(2));

        lock_mutex();
        data->counter /= 2;
        unlock_mutex();

        lock_mutex();
        std::ofstream log("laba3.log", std::ios::app);
        log << "[" << current_time() << "] PID=" << get_pid()
            << " EXIT --child2" << std::endl;
        unlock_mutex();

        return 0;
    }

    // --------------------------------------------------
    // Поток увеличения счётчика (все процессы)
    // --------------------------------------------------
    std::thread counter_thread([&]() {
        while (true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
            lock_mutex();
            data->counter++;
            unlock_mutex();
        }
    });

    // --------------------------------------------------
    // Поток логирования (ТОЛЬКО лидер)
    // --------------------------------------------------
    std::thread log_thread([&]() {
        if (!leader) return;

        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(1));

            lock_mutex();
            std::ofstream log("laba3.log", std::ios::app);
            log << "[" << current_time() << "] "
                << "PID=" << get_pid()
                << " COUNTER=" << data->counter
                << std::endl;
            unlock_mutex();
        }
    });

    // --------------------------------------------------
    // Поток запуска копий (ТОЛЬКО лидер)
    // --------------------------------------------------
    ChildProcess child1;
    ChildProcess child2;

    std::thread spawn_thread([&]() {
        if (!leader) return;

        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(3));

            bool c1_done = is_child_finished(child1);
            bool c2_done = is_child_finished(child2);

            if (!c1_done || !c2_done) {
                lock_mutex();
                std::ofstream log("laba3.log", std::ios::app);
                log << "[" << current_time() << "] "
                    << "Previous child still running"
                    << std::endl;
                unlock_mutex();
                continue;
            }

            start_child("--child1", child1);
            start_child("--child2", child2);
        }
    });


    counter_thread.join();
    log_thread.join();
    spawn_thread.join();

    close_shared_memory();
    close_mutex();
    release_leader();

    return 0;
}
