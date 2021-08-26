cmake_minimum_required(VERSION 3.0.0)
project(reversetcpd VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(external)

add_definitions(-DASIO_STANDALONE)

add_executable(ReverseTcpShell 
    src/Daemon.cpp
    src/main.cpp
    src/TcpClient.cpp
    src/ReverseTcpShell.cpp
)

target_include_directories(ReverseTcpShell PRIVATE
    ./include/
)

target_link_libraries(ReverseTcpShell pthread)

# Iso image build step
add_custom_command(
	TARGET ReverseTcpShell POST_BUILD
	COMMAND cp $<TARGET_FILE:ReverseTcpShell> ${CMAKE_CURRENT_LIST_DIR}/ReverseTcpShell
)
