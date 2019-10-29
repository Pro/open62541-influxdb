//
// Created by profanter on 29/10/2019.
// Copyright (c) 2019 fortiss GmbH. All rights reserved.
//

#include "ServerWatcher.h"

#include <utility>

ServerWatcher::ServerWatcher(std::shared_ptr<spdlog::logger> _logger, std::string _configDir) :
        logger(std::move(_logger)), configDir(std::move(_configDir)) {

}

void ServerWatcher::onServerAnnounce(const UA_ServerOnNetwork *serverOnNetwork, UA_Boolean isServerAnnounce, UA_Boolean isTxtReceived) {

}

bool ServerWatcher::start() {
    return false;
}
