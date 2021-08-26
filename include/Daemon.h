#ifndef DAEMON_H
#define DAEMON_H

#include <Common.h>

#include <unordered_map>

#include "TcpClient.h"

/*
 *  @brief  The main Daemon application class.
 *          Responsible for accepting new tcp connections and creating
 *          connection manager objects.
 */
class Daemon
{
public:
    /*
     *  @brief  Constructor.
     *
     *  @param  port the port id to listen on.
     */
    Daemon(const PortId port);

    //! Destructor.
    ~Daemon();

    /*
     *  @brief  The application main loop.
     */
    void Run();

private:
    /*
     *  @brief  Schedule the new asio connection accept.
     *
     *  @param  pException exception environment if called from
     *          an exception handler.
     */
    void StartAccept(std::exception_ptr pException = nullptr);

    /*
     *  @brief  The asio connection accept completion handler.
     *
     *  @param  pNewClient the new TcpClient object pointer.
     *  @param  error the asio error code.
     */
    void AcceptHandler(TcpClient::Pointer pNewClient, const asio::error_code &error);

    asio::io_context            m_ioContext;    ///< The ASIO IO context.
    asio::ip::tcp::acceptor     m_acceptor;     ///< The tcp connection acceptor object.
};

#endif // DAEMON_H
