//
// Created by profanter on 29/10/2019.
// Copyright (c) 2019 fortiss GmbH. All rights reserved.
//


#include <spdlog/spdlog.h>
#include "CLI/CLI.hpp"
#include "libconfig.h++"
#include <open62541/server.h>
#include <open62541/server_config.h>
#include <open62541/server_config_default.h>
#include <open62541/plugin/nodestore_default.h>

#include "logging_opcua.h"
#include "logging.h"
#include "ServerWatcher.h"

std::shared_ptr<spdlog::logger> logger;

using namespace fortiss;

bool running = true;

static void stopHandler(int) {
    running = false;
    logger->warn("Received Ctrl-C. Shutting down...");
}

static UA_StatusCode
UA_ServerConfig_setUriName(UA_ServerConfig *uaServerConfig, const char *uri, const char *name) {
    // delete pre-initialized values
    UA_String_deleteMembers(&uaServerConfig->applicationDescription.applicationUri);
    UA_LocalizedText_deleteMembers(&uaServerConfig->applicationDescription.applicationName);

    std::string opcUaUri(uri);
    std::string opcUaName(name);

    uaServerConfig->applicationDescription.applicationUri = UA_String_fromChars(opcUaUri.c_str());
    uaServerConfig->applicationDescription.applicationName.locale = UA_STRING_NULL;
    uaServerConfig->applicationDescription.applicationName.text = UA_String_fromChars(opcUaName.c_str());

    for (size_t i = 0; i < uaServerConfig->endpointsSize; i++) {
        UA_String_deleteMembers(&uaServerConfig->endpoints[i].server.applicationUri);
        UA_LocalizedText_deleteMembers(
                &uaServerConfig->endpoints[i].server.applicationName);

        UA_String_copy(&uaServerConfig->applicationDescription.applicationUri,
                       &uaServerConfig->endpoints[i].server.applicationUri);

        UA_LocalizedText_copy(&uaServerConfig->applicationDescription.applicationName,
                              &uaServerConfig->endpoints[i].server.applicationName);
    }

    return UA_STATUSCODE_GOOD;
}

static UA_StatusCode
initServerConfig(const std::shared_ptr<spdlog::logger>& logger, UA_ServerConfig *config, const std::string& appUri, const std::string& appName,
                 UA_UInt16 serverPort) {
    memset(config, 0, sizeof(UA_ServerConfig));

    UA_StatusCode retval = UA_ServerConfig_setBasics(config);
    if (retval != UA_STATUSCODE_GOOD) {
        logger->error("Can not set server configuration. Basics. {}", UA_StatusCode_name(retval));
        return retval;
    }
    retval = UA_ServerConfig_addNetworkLayerTCP(config, serverPort, 0, 0);

    if (retval != UA_STATUSCODE_GOOD) {
        logger->error("Can not set server configuration. AddNetworkLayer. {}", UA_StatusCode_name(retval));
        return retval;
    }

    /* Allocate the SecurityPolicies */
    retval = UA_ServerConfig_addSecurityPolicyNone(config, NULL);
    if (retval != UA_STATUSCODE_GOOD) {
        logger->error("Can not set server configuration. AddSecurityPolicyNone. {}", UA_StatusCode_name(retval));
        return retval;
    }


    UA_Nodestore_HashMap(&config->nodestore);

    retval = UA_ServerConfig_addAllEndpoints(config);
    if (retval != UA_STATUSCODE_GOOD) {
        logger->error("Can not set server configuration. AddEndpoint. {}", UA_StatusCode_name(retval));
        return retval;
    }


    config->logger.log = fortiss::log::opcua::UA_Log_Spdlog_log;
    config->logger.context = logger.get();
    config->logger.clear = fortiss::log::opcua::UA_Log_Spdlog_clear;

    config->discovery.mdnsEnable = true;
    config->discovery.mdns.mdnsServerName = UA_String_fromChars(appUri.c_str());
    config->discovery.mdns.serverCapabilitiesSize = 2;
    auto *caps = (UA_String *) UA_Array_new(2, &UA_TYPES[UA_TYPES_STRING]);
    caps[0] = UA_String_fromChars("LDS");
    caps[1] = UA_String_fromChars("NA");
    config->discovery.mdns.serverCapabilities = caps;

    UA_ServerConfig_setUriName(config, appUri.c_str(),
                               appName.c_str());

    return UA_STATUSCODE_GOOD;
}

struct serverOnNetworkCallbackData {
    std::function<void(const UA_ServerOnNetwork *)> onLdsAnnounce;
};

static void
serverOnNetworkCallback(const UA_ServerOnNetwork *serverOnNetwork, UA_Boolean isServerAnnounce,
                        UA_Boolean isTxtReceived, void *data) {

    ServerWatcher* watcher = static_cast<ServerWatcher*>(data);

    watcher->onServerAnnounce(serverOnNetwork, isServerAnnounce, isTxtReceived);
}


int main(int argc, char *argv[]) {
    // ------------- General initialization -------------------

    signal(SIGINT, stopHandler);
    signal(SIGTERM, stopHandler);


    CLI::App app{"open62541 InfluxDB"};

    std::string configFile = "server.cfg";
    app.add_option("--config", configFile, "Configuration file path", true);

    CLI11_PARSE(app, argc, argv);

    libconfig::Config cfg;
    // Read the file. If there is an error, report it and exit.
    try {
        cfg.readFile(configFile.c_str());
    }
    catch (const libconfig::FileIOException& fioex) {
        std::cerr << "I/O error while reading configuration file." << fioex.what() << std::endl;
        return (EXIT_FAILURE);
    }
    catch (const libconfig::ParseException& pex) {
        std::cerr << "Configuration file parse error at " << pex.getFile() << ":" << pex.getLine()
                  << " - " << pex.getError() << std::endl;
        return (EXIT_FAILURE);
    }
    const libconfig::Setting& settings = cfg.getRoot();


    logger = spdlog::get("opcua-influxdb");
    if (logger == nullptr) {
        logger = fortiss::log::LoggerFactory::createLogger("opcua-influxdb");
    }

    UA_ServerConfig uaServerConfig;
    UA_Server *uaServer = nullptr;
    int exitCode = EXIT_SUCCESS;

    try {
        std::string logLevel = settings["log"];
        if (logLevel == "trace")
            logger->set_level(spdlog::level::level_enum::trace);
        else if (logLevel == "debug")
            logger->set_level(spdlog::level::level_enum::debug);
        else if (logLevel == "info")
            logger->set_level(spdlog::level::level_enum::info);
        else if (logLevel == "warn")
            logger->set_level(spdlog::level::level_enum::warn);
        else if (logLevel == "err")
            logger->set_level(spdlog::level::level_enum::err);
        else if (logLevel == "critical")
            logger->set_level(spdlog::level::level_enum::critical);
        else if (logLevel == "off")
            logger->set_level(spdlog::level::level_enum::off);
        else {
            std::cerr
                    << "Invalid 'log' setting in configuration file. Must be one of [trace, debug, info, warn, err, critical, off]"
                    << std::endl;
            return (EXIT_FAILURE);
        }

        logger->info("Starting open62541 InfluxDB Server ...");

        // ------------- OPC UA initialization -------------------
        if (initServerConfig(
                logger,
                &uaServerConfig,
                "fortiss.component.camera.generic",
                "fortiss - Sensor - Camera",
                (UA_UInt16) ((int) settings["opcua"]["port"])) != UA_STATUSCODE_GOOD) {
            return EXIT_FAILURE;
        }

        uaServer = UA_Server_newWithConfig(&uaServerConfig);
        if (!uaServer) {
            logger->error("Can not create server instance");

            return EXIT_FAILURE;
        }

        const libconfig::Setting& root = cfg.getRoot();

        UA_StatusCode retval = UA_Server_run_startup(uaServer);
        if (retval != UA_STATUSCODE_GOOD) {
            throw std::runtime_error("Starting up the server failed with " + std::string(UA_StatusCode_name(retval)));
        }

        ServerWatcher watcher(logger, settings["watcher"]["config_dir"]);

        UA_Server_setServerOnNetworkCallback(uaServer, serverOnNetworkCallback, (void*)(&watcher));

        if (!watcher.start()) {
            logger->error("Can not start watcher");
            return EXIT_FAILURE;
        }


        while (running) {
            UA_Server_run_iterate(uaServer, false);

            std::this_thread::yield();
        }

    }
    catch (const libconfig::SettingNotFoundException& nfex) {
        logger->error("Setting missing in configuration file. {} ", nfex.what());
        logger->flush();
        return (EXIT_FAILURE);
    }
    catch (const std::runtime_error& rex) {
        logger->critical("Could not initialize gripper control. {} ", rex.what());
        logger->flush();
        exitCode = EXIT_FAILURE;
    }

    if (uaServer)
        UA_Server_run_shutdown(uaServer);

    UA_Server_delete(uaServer);
    logger->flush();
    return exitCode;
}