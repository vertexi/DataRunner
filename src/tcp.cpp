#include "tcp.hpp"

#include "hello_imgui/hello_imgui.h"

#ifdef _WIN32
asio::io_service io_service;
asio::ip::tcp::socket data_socket(io_service);

void TcpPoll()
{
    io_service.run();
    // assert(0);
}

void ConnectHandler(const asio::error_code & error)
{
    if (!error) {
        // Connection succeeded
        data_socket.set_option(asio::ip::tcp::no_delay(true));
        // data_socket.set_option(asio::socket_base::receive_buffer_size(1920 * 1080 * 4));
        // data_socket.set_option(asio::socket_base::send_buffer_size(1920 * 1080 * 4));
        data_socket.set_option(asio::detail::socket_option::integer<SOL_SOCKET, SO_RCVTIMEO>{10});
        HelloImGui::Log(HelloImGui::LogLevel::Info, "Connection ok");
    } else {
        // Connection failed
        HelloImGui::Log(HelloImGui::LogLevel::Error, ("Connection failed: " + error.message()).c_str());
    }
}

void ConnectTimeoutHandler(const asio::error_code & error)
{
    HelloImGui::Log(HelloImGui::LogLevel::Error, "Connect timeout.");
    if (!error) {
        data_socket.close();
    } else {
        HelloImGui::Log(HelloImGui::LogLevel::Error, ("Connect timeout failed: " + error.message()).c_str());
    }
}

void TcpTryConnect(const char *ip_addr, const int port)
{
    data_socket.async_connect(asio::ip::tcp::endpoint(asio::ip::address::from_string(ip_addr), (uint_least16_t)port), ConnectHandler);
    static asio::steady_timer connectionTimer = asio::steady_timer(io_service, asio::chrono::seconds(2));
    connectionTimer = asio::steady_timer(io_service, asio::chrono::seconds(2));
    connectionTimer.async_wait(&ConnectTimeoutHandler);
}

#endif



#ifdef __EMSCRIPTEN__

#endif
