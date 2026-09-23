
#include <cerrno>
#include <cstdlib>
#include <cassert>
#include <sys/socket.h>
#include <netdb.h>
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <thread>
#include <mutex>
#include <atomic>

constexpr std::string_view OE_PORT = "3490";
constexpr std::string_view MD_PORT = "3590";

constexpr int MAXDATASIZE = 100;
constexpr int BACKLOG = 100;
std::mutex book_mutex;

[[noreturn]] void fail(std::string_view what)
{
    std::cerr << "server " << what << ": " << std::strerror(errno) << '\n';
    std::exit(EXIT_FAILURE);
}

int make_listener(std::string_view port)
{
    // listen on sock_fd, send through the two client fds
    int sock_fd;

    addrinfo hints{};
    addrinfo *servinfo = nullptr; // points to results

    // Step 1: Get the address info
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int status = getaddrinfo(nullptr, port.data(), &hints, &servinfo);
    if (status != 0) std::cerr << "server: gai:" << gai_strerror(status);

    assert(servinfo->ai_next == nullptr);

    if ((sock_fd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) == -1)
        fail("socket");

    // set socket options so lets bind despite timeout
    int yes = 1;
    if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1)
        fail("setsockopt");

    if (bind(sock_fd, servinfo->ai_addr, servinfo->ai_addrlen) == -1)
        fail("bind");

    // all done with servinfo
    freeaddrinfo(servinfo);

    // listens to incoming connections on the socket bound to the specific port, and accepts
    if (listen(sock_fd, BACKLOG) == -1)
        fail("listen");

    return sock_fd;
}

void handle_connection(int fd, int other, std::string_view name)
{
    char buf[MAXDATASIZE];
    while (true) {
        ssize_t n = recv(fd, buf, sizeof buf, 0);
        if (n == -1 && errno == EINTR) continue;
        if (n <= 0) break;

        std::lock_guard lock(book_mutex);

        std::cout << name << ": " << buf << '\n';
    }
    std::cout << name << " closed\n";
    shutdown(other, SHUT_RDWR);
}

int main(int argc, char* argv[])
{

    int oe_sock = make_listener(OE_PORT);
    int md_sock = make_listener(MD_PORT);

    // two accepts, one per client
    sockaddr_storage their_addr{};
    socklen_t addr_size = sizeof(their_addr);

    int oe_fd = accept(oe_sock, reinterpret_cast<sockaddr*>(&their_addr), &addr_size);
    if (oe_fd == -1) fail("accept");

    addr_size = sizeof(their_addr);
    int md_fd = accept(md_sock, reinterpret_cast<sockaddr*>(&their_addr), &addr_size);
    if (md_fd == -1) fail("accept");

    // read with two threads
    std::thread md_thread(handle_connection, md_fd, oe_fd, "market data");
    std::thread oe_thread(handle_connection, oe_fd, md_fd, "order entry");

    md_thread.join();
    oe_thread.join();

    // epilogue
    close(oe_sock);
    close(md_sock);
    close(oe_fd);
    close(md_fd);
}