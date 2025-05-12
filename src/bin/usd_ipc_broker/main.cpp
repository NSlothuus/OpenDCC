// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include <zmq.h>
#include <thread>
#include <cassert>
#include <iostream>
#include <string>
#include <cstring>
#include "opendcc/base/vendor/cli11/CLI11.hpp"

namespace
{
    struct ConnectionSettings
    {
        std::string hostname = "*";
        uint32_t listener_port = 5561;
        uint32_t publisher_port = 5562;
        uint32_t sync_sender_port = 5560;
        uint32_t sync_receiver_port = 5559;
    };

    bool parse_args(int argc, char** argv, ConnectionSettings* result)
    {
        assert(result);

        CLI::App options_parser { "Options" };
        std::string host_val, listener_val, publisher_val;
        options_parser.add_option("--host", host_val, "default is *");
        options_parser.add_option("--listener", listener_val, "default = 5561");
        options_parser.add_option("--publisher", publisher_val, "default = 5562");
        std::string sync_sender_port_val, sync_receiver_port_val;
        options_parser.add_option("--sync-sender-port", sync_sender_port_val, "default = 5560");
        options_parser.add_option("--sync-receiver-port", sync_receiver_port_val, "default = 5559");

        CLI11_PARSE(options_parser, argc, argv);

        ConnectionSettings settings;
        if (!host_val.empty())
        {
            settings.hostname = host_val;
        }

        if (!listener_val.empty())
        {
            settings.listener_port = std::stoul(listener_val);
        }

        if (!publisher_val.empty())
        {
            settings.publisher_port = std::stoul(publisher_val);
        }

        if (!sync_sender_port_val.empty())
        {
            settings.sync_sender_port = std::stoul(sync_sender_port_val);
        }

        if (!sync_receiver_port_val.empty())
        {
            settings.sync_receiver_port = std::stoul(sync_receiver_port_val);
        }

        *result = settings;
        return true;
    }

    std::string make_tcp_address(const std::string& hostname, uint32_t port)
    {
        return "tcp://" + hostname + ":" + std::to_string(port);
    }

#ifdef WIN32
#define ZMQ_ERRNO zmq_errno()
#else
#define ZMQ_ERRNO errno
#endif

    void err_handle(size_t line)
    {
        const auto error_code = ZMQ_ERRNO;
        if (error_code == 0 || error_code == ETERM)
            return;

        std::cerr << "ZMQ_ERROR " << error_code << ": at line " << line << " -- " << zmq_strerror(error_code) << '\n';
    }

};

#define CHECK_ZMQ_ERROR_AND_RETURN_IT(action) \
    do                                        \
    {                                         \
        const int rc = (action);              \
        if (rc == -1)                         \
        {                                     \
            err_handle(__LINE__);             \
            return rc;                        \
        }                                     \
    } while (false)

int main(int argc, char** argv)
{
    ConnectionSettings settings;
    parse_args(argc, argv, &settings);

    void* ctx = zmq_ctx_new();

    void* xsub = zmq_socket(ctx, ZMQ_XSUB);
    CHECK_ZMQ_ERROR_AND_RETURN_IT(zmq_bind(xsub, make_tcp_address(settings.hostname, settings.publisher_port).c_str()));
    void* xpub = zmq_socket(ctx, ZMQ_XPUB);
    CHECK_ZMQ_ERROR_AND_RETURN_IT(zmq_bind(xpub, make_tcp_address(settings.hostname, settings.listener_port).c_str()));

    void* router = zmq_socket(ctx, ZMQ_ROUTER);
    CHECK_ZMQ_ERROR_AND_RETURN_IT(zmq_bind(router, make_tcp_address(settings.hostname, settings.sync_receiver_port).c_str()));
    void* dealer = zmq_socket(ctx, ZMQ_DEALER);
    CHECK_ZMQ_ERROR_AND_RETURN_IT(zmq_bind(dealer, make_tcp_address(settings.hostname, settings.sync_sender_port).c_str()));

    auto thread = std::thread([router, dealer] { zmq_proxy(router, dealer, nullptr); });

    zmq_proxy(xsub, xpub, nullptr);

    thread.join();
    return 0;
}
