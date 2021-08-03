#ifndef COMMON_H
#define COMMON_H

#include <arpa/inet.h>

#include <netinet/in.h>
#include <netinet/ip.h>

#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>

#define ASIO_STANDALONE
#include <asio.hpp>

#include <atomic>
#include <errno.h>
#include <exception>
#include <fcntl.h>
#include <functional>
#include <memory>
#include <iostream>
#include <stdbool.h>
#include <stdexcept>
#include <stdio.h>
#include <stdlib.h>
#include <system_error>
#include <unistd.h>

#include <stdio.h>
#define LOG(fmt, ...) do { fprintf(stderr, "%s:%d:%s(): " fmt "\n", __FILE__, __LINE__, __func__, __VA_ARGS__); } while (0)

#define TRACE() std::cout << "TRACE - " << __func__ << ":" << __LINE__ << '\n'

#define TRACE_FUN() std::cout << "TRACE - " << __func__ << '\n'

typedef int SocketHandle;   ///< Socket handle typedef.
typedef in_port_t PortId;   ///< Port Id typedef.

typedef size_t ClientId;    ///< Client Id typedef.

#endif // COMMON_H
