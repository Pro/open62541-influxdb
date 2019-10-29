//
// Created by profanter on 11/29/18.
// Copyright (c) 2018 fortiss GmbH. All rights reserved.
//

#ifndef LOGGING_H
#define LOGGING_H
#pragma once

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include <map>
#include <limits.h>
#include <stdlib.h>

namespace fortiss {
    namespace log {
        namespace {
            class LoggerFactory {
            public:
                static std::shared_ptr<spdlog::logger> createLogger(const ::std::string &loggerName) {

                    /*std::string logPath = "logs/" + loggerName + ".log";
                    char *full_path = realpath(logPath.c_str(), NULL);*/




                    // This other example use a single logger with multiple sinks.
                    // This means that the same log_msg is forwarded to multiple sinks;
                    // Each sink can have it's own log level and a message will be logged.
                    std::vector<spdlog::sink_ptr> sinks;
                    sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
                    // create a thread safe sink which will keep its file size to a maximum of 6MB and a maximum of 3 rotated files.
                    //sinks.push_back( std::make_shared<spdlog::sinks::rotating_file_sink_mt>(logPath, 1048576 * 5, 3) );
                    auto console_multisink = std::make_shared<spdlog::logger>(loggerName, sinks.begin(), sinks.end());
                    console_multisink->set_level(spdlog::level::info);
                    sinks[0]->set_level(spdlog::level::trace);  // console. Allow everything.  Default value
                    //sinks[1]->set_level( spdlog::level::trace);  //  regular file. Allow everything.  Default value
                    /*console_multisink.warn("warn: will print only on console and regular file");
                    if( enable_debug )
                    {
                        console_multisink.set_level( spdlog::level::debug); // level of the logger
                        sinks[1]->set_level( spdlog::level::debug);  // regular file
                        sinks[2]->set_level( spdlog::level::debug);  // debug file
                    }
                    console_multisink.debug("Debug: you should see this on console and both files");*/
                    console_multisink->set_pattern("[%D %H:%M:%S.%e] [%^%-8l%$] [%n] %v");
                    return console_multisink;
                }
            };
        }

        static std::shared_ptr<spdlog::logger> get(const ::std::string &loggerName) {
            auto logger = spdlog::get(loggerName);
            if (logger == nullptr) {
                return LoggerFactory::createLogger(loggerName);
            }
            return logger;
        }

    }
}

#endif //LOGGING_H
