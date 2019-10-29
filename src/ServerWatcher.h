//
// Created by profanter on 29/10/2019.
// Copyright (c) 2019 fortiss GmbH. All rights reserved.
//

#ifndef OPEN62541_INFLUXDB_SERVERWATCHER_H
#define OPEN62541_INFLUXDB_SERVERWATCHER_H


#include <open62541/types_generated.h>
#include <spdlog/logger.h>

class ServerWatcher {

private:
    std::shared_ptr<spdlog::logger> logger;
    const std::string configDir;
public:
    explicit ServerWatcher(std::shared_ptr<spdlog::logger> logger, std::string configDir);

    void onServerAnnounce(const UA_ServerOnNetwork *serverOnNetwork, UA_Boolean isServerAnnounce,
                          UA_Boolean isTxtReceived);

    bool start();
};


#endif //OPEN62541_INFLUXDB_SERVERWATCHER_H
