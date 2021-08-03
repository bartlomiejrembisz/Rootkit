#ifndef REVERSE_TCP_SHELL_H
#define REVERSE_TCP_SHELL_H

#include <Common.h>

#include <vector>

namespace Task
{

class ReverseTcpShell
{
public:
    typedef asio::local::stream_protocol::socket UnixSocket;

    ReverseTcpShell(asio::io_context &ioContext);

    ~ReverseTcpShell();

    bool Start();

    UnixSocket &GetParentSocket();
    pid_t GetShellPid() const;

    bool HasShellTerminated() const;

private:
    UnixSocket  m_parentSocket;
    UnixSocket  m_childSocket;
    pid_t       m_shellPid;     ///< The pid of the shell.
};

} // namespace Task

#endif // UI_MENU_H
