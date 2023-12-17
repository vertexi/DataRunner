#pragma once

#ifdef _WIN32
#include "asio.hpp"


extern asio::ip::tcp::socket data_socket;
#endif


#ifdef __EMSCRIPTEN__

#endif
