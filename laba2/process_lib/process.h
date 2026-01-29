#pragma once

#include <string>

// Запуск программы
// command   — команда для запуска
// wait      — ждать завершения или нет
// exit_code — код возврата (если wait == true)
int run_process(const std::string& command, bool wait, int& exit_code);
