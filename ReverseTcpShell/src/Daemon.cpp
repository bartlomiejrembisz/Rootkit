#include "Daemon.h"

Daemon::Daemon(const PortId port) :
    m_ioContext(),
    m_acceptor(m_ioContext, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
{
    //! Set the address reuse socket option.
    const asio::socket_base::reuse_address reuseAddressOption(true);
    m_acceptor.set_option(reuseAddressOption);
    
    //! Start accepting new connections.
    StartAccept();
}

Daemon::~Daemon()
{
}

void Daemon::Run()
{
    m_ioContext.run();
}

void Daemon::StartAccept(std::exception_ptr pException)
{
    TcpClient::Pointer pNewClient = TcpClient::Create(m_ioContext);

    m_acceptor.async_accept(pNewClient->GetSocket(), std::bind(&Daemon::AcceptHandler, this, pNewClient, std::placeholders::_1));
}

void Daemon::AcceptHandler(TcpClient::Pointer pNewClient, const asio::error_code &error)
{
    try
    {
        if (!error)
            pNewClient->Start();   
    }
    catch(const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
    
    StartAccept();
}
