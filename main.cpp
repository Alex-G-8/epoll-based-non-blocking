#include <iostream>
#include <unistd.h>         // close(), read(), write()
#include <fcntl.h>          // fcntl()
#include <sys/socket.h>     // socket(), bind(), listen(), accept()
#include <netinet/in.h>     // sockaddr_in
#include <sys/epoll.h>      // epoll functions

#define PORT 8080
#define MAX_EVENTS 10

using namespace std;

// Set a file descriptor to non-blocking mode
int set_non_blocking(int fd) {
    return fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
}

int main() {
    // Create server socket (TCP)
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    // Configure address structure
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    bind(server_fd, (sockaddr*)&addr, sizeof(addr));
    listen(server_fd, SOMAXCONN);

    // Make server non-blocking
    set_non_blocking(server_fd);

    // Create epoll instance
    int epfd = epoll_create1(0);

    epoll_event ev{}, events[MAX_EVENTS];

    // Register server with epoll
    ev.events = EPOLLIN;
    ev.data.fd = server_fd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, server_fd, &ev);

    cout << "Server running on port " << PORT << endl;

    // Event loop
    while (true) {
        // Wait for events
        int n = epoll_wait(epfd, events, MAX_EVENTS, -1);

        for (int i = 0; i < n; i++) {
            int fd = events[i].data.fd;

            // If event is on server, new client connection
            if (fd == server_fd) {
                int client_fd = accept(server_fd, nullptr, nullptr);

                // Make client non-blocking
                set_non_blocking(client_fd);

                // Add client to epoll
                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = client_fd;
                epoll_ctl(epfd, EPOLL_CTL_ADD, client_fd, &ev);

                cout << "New client connected\n";
            } 
            // Data from client
            else {
                char buffer[1024];

                int count = read(fd, buffer, sizeof(buffer));

                // Client disconnected or error
                if (count <= 0) {
                    close(fd);
                    cout << "Client disconnected\n";
                } 
                // Message received
                else {
                    cout << "Message: "
                              << string(buffer, count)
                              << endl;

                    // Echo message back to client
                    write(fd, buffer, count);
                }
            }
        }
    }

    return 0;
}
