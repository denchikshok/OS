#include <iostream>
#include <chrono>
#include <thread>
#include <random>
#include <string>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <sys/stat.h>
    #include <fcntl.h>
    #include <unistd.h>
#endif

double generate_temperature()
{
    static std::default_random_engine eng(
        static_cast<unsigned>(std::chrono::system_clock::now().time_since_epoch().count())
    );
    static std::uniform_real_distribution<double> dist(20.0, 30.0);

    return dist(eng);
}

int main()
{
#ifdef _WIN32
    const char* pipe_name = R"(\\.\pipe\temp_device)";

    std::cout << "Waiting for logger to connect...\n";

    HANDLE pipe = CreateNamedPipeA(
        pipe_name,
        PIPE_ACCESS_OUTBOUND,
        PIPE_TYPE_MESSAGE | PIPE_WAIT,
        1,
        1024,
        1024,
        0,
        NULL
    );

    if (pipe == INVALID_HANDLE_VALUE)
    {
        std::cerr << "CreateNamedPipe failed\n";
        return 1;
    }

    ConnectNamedPipe(pipe, NULL);
    std::cout << "Logger connected\n";

#else
    const char* fifo_name = "/tmp/temp_device";

    mkfifo(fifo_name, 0666);

    int fd = open(fifo_name, O_WRONLY);
    if (fd < 0)
    {
        perror("open");
        return 1;
    }
#endif

    while (true)
    {
        double temp = generate_temperature();
        std::string line = std::to_string(temp) + "\n";

#ifdef _WIN32
        DWORD written;
        WriteFile(pipe, line.c_str(), (DWORD)line.size(), &written, NULL);
#else
        write(fd, line.c_str(), line.size());
#endif

        std::cout << "Sent: " << temp << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

#ifdef _WIN32
    CloseHandle(pipe);
#else
    close(fd);
#endif

    return 0;
}
