cmake_minimum_required(VERSION 3.0.0)
project(DamnVulnerableAndroidRootkit VERSION 0.1.0)

# include(CTest)
# enable_testing()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(Common External)

# add_subdirectory(Client)
# add_subdirectory(Daemon)
add_subdirectory(ReverseTcpShell)
