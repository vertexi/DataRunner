#include "tcp.hpp"

#include "hello_imgui/hello_imgui.h"

/**
 * Windows-specific TCP networking code using Asio.
 * Creates an Asio IO service, TCP socket, and handlers for connecting,
 * polling, and timing out. Allows asynchronously connecting to a TCP
 * server, configuring socket options, and logging status.
 */
#ifdef _WIN32
asio::io_service io_service;
asio::ip::tcp::socket data_socket(io_service);


void TcpPoll()
{
    io_service.poll();
}
void TcpServiceStart()
{

}
void TcpServiceStop()
{
    io_service.stop();
}

void ConnectHandler(const asio::error_code &error)
{
    if (!error)
    {
        // Connection succeeded
        data_socket.set_option(asio::ip::tcp::no_delay(true));
        // data_socket.set_option(asio::socket_base::receive_buffer_size(1920 * 1080 * 4));
        // data_socket.set_option(asio::socket_base::send_buffer_size(1920 * 1080 * 4));
        data_socket.set_option(asio::detail::socket_option::integer<SOL_SOCKET, SO_RCVTIMEO>{200});
        HelloImGui::Log(HelloImGui::LogLevel::Info, "Connection ok");
    }
    else
    {
        // Connection failed
        HelloImGui::Log(HelloImGui::LogLevel::Error, ("Connection failed: " + error.message()).c_str());
    }
}

void ConnectTimeoutHandler(const asio::error_code &error)
{
    HelloImGui::Log(HelloImGui::LogLevel::Error, "Connect timeout.");
    if (!error)
    {
        data_socket.close();
    }
    else
    {
        HelloImGui::Log(HelloImGui::LogLevel::Error, ("Connect timeout failed: " + error.message()).c_str());
    }
}

void TcpTryConnect(const char *ip_addr, const int port)
{
    if (data_socket.is_open())
        data_socket.close();
    data_socket.async_connect(asio::ip::tcp::endpoint(asio::ip::address::from_string(ip_addr), (uint_least16_t)port), ConnectHandler);
    static asio::steady_timer connectionTimer(io_service, asio::chrono::seconds(2));
    connectionTimer.expires_after(asio::chrono::seconds(2));
    connectionTimer.async_wait(&ConnectTimeoutHandler);
}

#endif



#ifdef __EMSCRIPTEN__

#endif
