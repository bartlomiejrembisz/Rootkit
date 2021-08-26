#include <iostream>

#include <signal.h>
#include <memory>

#include <limits>

#include <Daemon.h>
#include <ReverseTcpShell.h>

std::unique_ptr<Daemon> g_pDaemon;

/*
 *  @brief  The SIGINT (Ctrl+c) signal handler.
 *          Clean up the connections and quit.
 *
 *  @param  signal the signal id.
 */
void SigIntHandler(int signal)
{
    std::cerr << "\nCaught SIGINT, exiting...\n";
    g_pDaemon.reset();
    exit(0);
}

int main(int argc, char** argv)
{
    if ((argc < 2))
    {
        std::cout << " Reverse TCP shell daemon.\n"
                << "    Usage: " << argv[0] << " <port in 1024-65535>\n\n"
                << " Environment variables:\n"
                << "    SHELL - shell executable. Required.\n"
                << "    PATH - path for the shell. Default=" DEFAULT_PATH "\n";
        
        return 1;
    }

    const int portIdPlaceholder = std::stoi(std::string(argv[1]));
    if (std::numeric_limits<uint16_t>::max() < portIdPlaceholder)
    {
        std::cerr << "Port too big, " << portIdPlaceholder << " > "
            << std::numeric_limits<uint16_t>::max() << '\n';
        
        return 1;
    }
    else if (1024 > portIdPlaceholder)
    {
        std::cerr << "Port too small, " << portIdPlaceholder << " < 1024." << '\n';
        
        return 1;
    }

    if (Task::ReverseTcpShell::SHELL.empty())
    {
        std::cerr  << "SHELL is undefined.\n";
        
        return 1;
    }
    
    //! Require root priviledges.
    if (getuid() != 0)
        std::cerr << "WARNING: Root privileges not available.\n";

    std::cout << "SHELL=" << Task::ReverseTcpShell::SHELL << "\n";
    std::cout << "PATH=" << Task::ReverseTcpShell::PATH << "\n";

    const PortId portId = static_cast<PortId>(portIdPlaceholder);

    signal(SIGINT, SigIntHandler);

    try
    {
        g_pDaemon.reset(new Daemon(portId));
        g_pDaemon->Run();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }

    return 0;
}

