
#include <cerrno>
#include <cstdlib>
#include <cassert>
#include <sys/socket.h>
#include <netdb.h>
#include <iostream>
#include <cstring>
#include <unistd.h>

[[noreturn]] void fail(std::string_view what)
{
    std::cerr << "oe client " << what << ": " << std::strerror(errno) << '\n';
    std::exit(EXIT_FAILURE);
}

int main()
{
    constexpr std::string_view OE_PORT = "3490";

    // sends on sock_fd
    int sock_fd;

    addrinfo hints{};
    addrinfo *servinfo = nullptr; // points to results

    // Step 1: Get the address info
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    int status = getaddrinfo(nullptr, OE_PORT.data(), &hints, &servinfo);
    if (status != 0) std::cerr << "server: gai:" << gai_strerror(status);

    // Step 2: Make a socket and bind it
    assert(servinfo->ai_next == nullptr);

    if ((sock_fd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) == -1)
        fail("socket");

    // set socket options so lets bind despite timeout
    int yes = 1;
    if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1)
        fail("setsockopt");

    // connect to the socket
    if (connect(sock_fd, servinfo->ai_addr, servinfo->ai_addrlen) == -1) fail("connect");

    // all done with servinfo
    freeaddrinfo(servinfo);

    if (send(sock_fd, "Hello, world!", 13, 0) == -1)
        fail("send");

    close(sock_fd);
}