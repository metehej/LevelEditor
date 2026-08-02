#include "Log.h"

#include <plog/Init.h>
#include <plog/Formatters/CsvFormatter.h>
#include <plog/Formatters/TxtFormatter.h>
#include <plog/Appenders/RollingFileAppender.h>
#include <plog/Appenders/ColorConsoleAppender.h> 

#include <filesystem>

#include "Config.h"
#include "PathHelper.h"

void InitializeLogging() {
    std::filesystem::create_directories(GetAbsolutePath(Config::LOG_FILE));
    static plog::RollingFileAppender<plog::CsvFormatter> fileAppender(GetAbsolutePath(Config::LOG_FILE).c_str(), Config::LOG_SIZE, 1);
    static plog::ConsoleAppender<plog::TxtFormatter> consoleAppender;
    plog::init(plog::info, &fileAppender).addAppender(&consoleAppender);
}