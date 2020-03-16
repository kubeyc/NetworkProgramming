#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <cstring>

#include <vector>
#include <iostream>

using std::vector;

int main(int argc, const char *argv[]) {
    int servfd = socket(AF_INET, SOCK_STREAM, 0);
    int ret = 0;
    struct sockaddr_in addr;
    bzero(&addr, sizeof(struct sockaddr_in));

    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9090);

    ret = bind(servfd, (const struct sockaddr *) &addr, sizeof(struct sockaddr_in));
    if (ret != 0) {
        std::cout << "bind error" << std::endl;
        return 0;
    }

    ret = listen(servfd, 128);
    if (ret != 0) {
        std::cout << "listen error" << std::endl;
        return 0;
    }

    fd_set rset;
    FD_ZERO(&rset);
    FD_SET(servfd, &rset);
    int maxFd = servfd;
    vector<int> clients(FD_SETSIZE, -1);
    vector<int>::size_type maxClientSize = 0;
    int readyFD;



    char buf[1024];
    memset(buf, '\0', 1024);

    int readn;
    for (;;) {
        readyFD = select(maxFd + 1, &rset, nullptr, nullptr, nullptr);
        if (readyFD == -1) {
            std::cout << "select failure." << std::endl;
            break;
        }

        std::cout << readyFD << std::endl;

        if (FD_ISSET(servfd, &rset)) {
            int connFd;
            struct sockaddr_in connAddr;
            socklen_t connAddrLen = sizeof(struct sockaddr_in);
            connFd = accept(servfd, (struct sockaddr *) &connAddr, &connAddrLen);
            // 设置连接的客户端的fd
            for (vector<int>::size_type i = 0, n = clients.size(); i < n; i++) {
                if (clients[i] == -1) {
                    clients.at(i) = connFd;
                    maxClientSize = i;
                    break;
                }
            }
            FD_SET(connFd, &rset);

            if (maxClientSize == clients.size()) {
                std::cout << "too many connection." << std::endl;
                continue;
            }
        }

        for (vector<int>::size_type i = 0, n = clients.size(); i < n; i++) {
            if (clients[i] < 0) {
                continue;
            }
            if (FD_ISSET(clients[i], &rset)) {
                // 客户端关闭连接
                readn = recv(clients[i], buf, 1024, 0);
                if (readn == 0) {
                    clients.at(i) = -1;
                    FD_CLR(clients[i], &rset);

                } else if (readn > 0) {
                    send(clients[i], buf, readn, 0);
                    memset(buf, '\0', readn);
                }
            }
        }
    }
}