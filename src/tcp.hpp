#pragma once

#ifdef _WIN32
#include "asio.hpp"


extern asio::ip::tcp::socket data_socket;

void TcpPoll();
void TcpTryConnect(const char *ip_addr, const int port);
#endif


#ifdef __EMSCRIPTEN__

#endif
