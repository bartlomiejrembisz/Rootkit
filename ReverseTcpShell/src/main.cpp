#include <iostream>

#include <signal.h>
#include <memory>

#include "Daemon.h"

std::unique_ptr<Daemon> g_pDaemon;

void SigIntHandler(int signal)
{
    std::cerr << "\nCaught SIGINT, exiting...\n";
    g_pDaemon.reset();
    exit(0);
}

int main(int argc, char** argv)
{
    if (getuid() != 0)
    {
        std::cerr << "Daemon requires root privilege\n";
        exit(1);
    }

    signal(SIGINT, SigIntHandler);

    try
    {
        g_pDaemon.reset(new Daemon(9999));
        g_pDaemon->Run();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }

    return 0;
}

