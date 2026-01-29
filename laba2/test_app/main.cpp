#include <iostream>
#include "process.h"

int main()
{
    int exit_code = -1;

#ifdef _WIN32
    std::string cmd = "cmd /c echo Hello from child process";
#else
    std::string cmd = "echo Hello from child process";
#endif

    std::cout << "Running process..." << std::endl;

    run_process(cmd, true, exit_code);

    std::cout << "Process finished, exit code = "
              << exit_code << std::endl;

    return 0;
}
