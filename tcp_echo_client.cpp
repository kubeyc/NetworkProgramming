#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <cstring>

#include <vector>
#include <iostream>
#include <string>

int main(int argc, const char *argv[]) {
    int serverFd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in serverAddr;
    bzero(&serverAddr, sizeof(struct sockaddr_in));

    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(9090);

    connect(serverFd, (const struct sockaddr *) &serverAddr, sizeof(struct sockaddr_in));
    char buf[1024];
    memset(buf, '\0', 1024);
    while (true) {
        std::cin >> buf;
        int sendn = send(serverFd, buf, sizeof(buf), 0);
        memset(buf, '\0', sendn);
        int readn = recv(serverFd, buf, 1024, 0);
        std::cout << "recv: " << buf << std::endl;
        memset(buf, '\0', readn);
    }
}