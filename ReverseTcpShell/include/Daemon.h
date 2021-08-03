#ifndef DAEMON_H
#define DAEMON_H

#include <Common.h>

#include <unordered_map>

#include "TcpClient.h"

class Daemon
{
public:
    Daemon(const PortId port);

    ~Daemon();

    void Run();

private:
    void StartAccept(std::exception_ptr pException = nullptr);

    void AcceptHandler(TcpClient::Pointer pNewClient, const asio::error_code &error);

    asio::io_context            m_ioContext;    ///< The ASIO IO context.
    asio::ip::tcp::acceptor     m_acceptor;     ///< The tcp connection acceptor.
};

#endif // DAEMON_H
