#include "process.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/wait.h>
#endif

int run_process(const std::string& command, bool wait, int& exit_code)
{
#ifdef _WIN32

    STARTUPINFOA si{};
    PROCESS_INFORMATION pi{};

    si.cb = sizeof(si);

    char cmd[1024];
    strcpy(cmd, command.c_str());

    if (!CreateProcessA(
            NULL,
            cmd,
            NULL,
            NULL,
            FALSE,
            CREATE_NO_WINDOW,
            NULL,
            NULL,
            &si,
            &pi))
    {
        return -1;
    }

    if (wait)
    {
        WaitForSingleObject(pi.hProcess, INFINITE);

        DWORD code;
        GetExitCodeProcess(pi.hProcess, &code);
        exit_code = (int)code;
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

#else
    pid_t pid = fork();

    if (pid == 0)
    {
        execl("/bin/sh", "sh", "-c", command.c_str(), (char*)nullptr);
        _exit(1);
    }
    else if (pid > 0)
    {
        if (wait)
        {
            int status;
            waitpid(pid, &status, 0);

            if (WIFEXITED(status))
                exit_code = WEXITSTATUS(status);
        }
    }
    else
    {
        return -1;
    }
#endif

    return 0;
}
