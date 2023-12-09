#include "tcp.hpp"

#ifdef _WIN32
asio::io_service io_service;
asio::ip::tcp::socket data_socket(io_service);

void TCP_tryConnect(const char *ip_addr, const int port)
{
    // data_socket.async_connect(asio::ip::tcp::endpoint(asio::ip::address::from_string(ip_addr), port));
}

#endif



#ifdef __EMSCRIPTEN__

#endif
