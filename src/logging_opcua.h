//
// Created by profanter on 11/29/18.
// Copyright (c) 2018 fortiss GmbH. All rights reserved.
//

#ifndef LOGGING_OPCUA_H
#define LOGGING_OPCUA_H
#pragma once

#include <open62541/plugin/log.h>
#include <spdlog/logger.h>

namespace fortiss {
    namespace log {
        namespace opcua {

            namespace {
                const char *LogsCategoryNames[7] = {"network", "channel", "session", "server", "client", "userland", "security_policy"};
            }

            inline void UA_Log_Spdlog_log(void *context, UA_LogLevel level, UA_LogCategory category, const char *msg, va_list args) {
                auto logger = (spdlog::logger*)context;
                char tmpStr[400];
                snprintf(tmpStr, 400, "[OPC UA/%s] ", LogsCategoryNames[category]);
                char *start = &tmpStr[strlen(tmpStr)];

                vsprintf(start, msg, args);

                size_t len = strlen(tmpStr);
                tmpStr[len] = '\0';

                switch (level) {
                    case UA_LOGLEVEL_TRACE:
                        logger->trace(tmpStr);
                        break;
                    case UA_LOGLEVEL_DEBUG:
                        logger->debug(tmpStr);
                        break;
                    case UA_LOGLEVEL_INFO:
                        logger->info(tmpStr);
                        break;
                    case UA_LOGLEVEL_WARNING:
                        logger->warn(tmpStr);
                        break;
                    case UA_LOGLEVEL_ERROR:
                    case UA_LOGLEVEL_FATAL:
                        logger->error(tmpStr);
                        break;
                }
            }

            inline void
            UA_Log_Spdlog_clear(void *logContext) {
            }
        }
    }
}


#endif //LOGGING_OPCUA_H
