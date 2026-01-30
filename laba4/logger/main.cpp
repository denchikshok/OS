#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <thread>
#include <iomanip>
#include <sstream>
#include <string>
#include <algorithm>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <fcntl.h>
    #include <unistd.h>
#endif

using Clock = std::chrono::system_clock;

// ===================== структуры =====================

struct Measurement
{
    Clock::time_point time;
    double temperature;
};

struct HourAverage
{
    std::time_t hour_start;
    double average;
};

struct DayAverage
{
    std::time_t day_start;
    double average;
};

// ===================== вспомогательные функции =====================

std::string format_time(std::time_t t)
{
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

std::string format_day(std::time_t t)
{
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d");
    return oss.str();
}

std::time_t hour_start(std::time_t t)
{
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    tm.tm_min = 0;
    tm.tm_sec = 0;
    return std::mktime(&tm);
}

std::time_t day_start(std::time_t t)
{
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    tm.tm_hour = 0;
    tm.tm_min  = 0;
    tm.tm_sec  = 0;
    return std::mktime(&tm);
}

// ===================== запись измерений в файл =====================

void rewrite_measurements_log(const std::vector<Measurement>& measurements)
{
    std::ofstream file("measurements.log", std::ios::trunc); // перезаписать файл
    for (const auto& m : measurements)
    {
        std::time_t tt = Clock::to_time_t(m.time);
        std::tm tm{};
#ifdef _WIN32
        localtime_s(&tm, &tt);
#else
        localtime_r(&tt, &tm);
#endif
        file << std::put_time(&tm, "%Y-%m-%d %H:%M:%S")
             << " temp=" << m.temperature << "\n";
    }
}

// ===================== MAIN =====================

int main()
{
#ifdef _WIN32
    const char* pipe_name = R"(\\.\pipe\temp_device)";
    HANDLE pipe;
    std::cout << "Waiting for device...\n";
    while (true)
    {
        pipe = CreateFileA(pipe_name, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
        if (pipe != INVALID_HANDLE_VALUE)
            break;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
#else
    const char* fifo_name = "/tmp/temp_device";
    int fd = open(fifo_name, O_RDONLY);
    if (fd < 0)
    {
        perror("open");
        return 1;
    }
#endif

    std::vector<Measurement> measurements;
    std::vector<HourAverage> hourly;
    std::vector<DayAverage> daily;

    std::time_t current_hour = 0;
    std::time_t current_day  = 0;

    char buffer[128];

    while (true)
    {
#ifdef _WIN32
        DWORD bytesRead;
        if (!ReadFile(pipe, buffer, sizeof(buffer)-1, &bytesRead, NULL))
            break;
        buffer[bytesRead] = '\0';
#else
        ssize_t bytesRead = read(fd, buffer, sizeof(buffer)-1);
        if (bytesRead <= 0)
            break;
        buffer[bytesRead] = '\0';
#endif

        double temp = std::stod(buffer);
        auto now = Clock::now();
        std::time_t now_tt = Clock::to_time_t(now);

        // ---------- добавить измерение ----------
        measurements.push_back({ now, temp });

        // ---------- удалить старые измерения (24 часа) ----------
        auto limit24 = now - std::chrono::hours(24);
        measurements.erase(
            std::remove_if(measurements.begin(), measurements.end(),
                [&](const Measurement& m){ return m.time < limit24; }),
            measurements.end()
        );

        // ---------- переписать measurements.log ----------
        rewrite_measurements_log(measurements);

        // ===================== часовые усреднения =====================
        std::time_t h = hour_start(now_tt);
        if (current_hour == 0)
            current_hour = h;
        else if (h != current_hour)
        {
            double sum = 0;
            int cnt = 0;
            for (auto& m : measurements)
            {
                if (hour_start(Clock::to_time_t(m.time)) == current_hour)
                {
                    sum += m.temperature;
                    cnt++;
                }
            }
            if (cnt > 0)
            {
                double avg = sum / cnt;
                hourly.push_back({ current_hour, avg });
                std::ofstream f("hourly.log", std::ios::app);
                f << format_time(current_hour) << " avg=" << avg << "\n";
            }
            current_hour = h;
        }

        // ===================== дневные усреднения =====================
        std::time_t d = day_start(now_tt);
        if (current_day == 0)
            current_day = d;
        else if (d != current_day)
        {
            double sum = 0;
            int cnt = 0;
            for (auto& m : measurements)
            {
                if (day_start(Clock::to_time_t(m.time)) == current_day)
                {
                    sum += m.temperature;
                    cnt++;
                }
            }
            if (cnt > 0)
            {
                double avg = sum / cnt;
                daily.push_back({ current_day, avg });
                std::ofstream f("daily.log", std::ios::app);
                f << format_day(current_day) << " avg=" << avg << "\n";
            }
            current_day = d;
        }

        std::cout << "Temp: " << temp << std::endl;
    }

#ifdef _WIN32
    CloseHandle(pipe);
#else
    close(fd);
#endif

    return 0;
}
