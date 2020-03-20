#include <iostream>
#include <vector>

#include <cstring>

#include <poll.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

using std::cout;
using std::endl;
using std::string;

int createServer(in_addr_t addr, in_port_t port, int backlog) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        return fd;
    }

    sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(sockaddr_in));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = addr;
    serverAddr.sin_port = port;

    int callRet;
    callRet = bind(fd, (const sockaddr *) &serverAddr, sizeof(sockaddr_in));
    if (callRet == -1) {
        return callRet;
    }

    callRet = listen(fd, backlog);
    if (callRet == -1) {
        return callRet;
    }

    return fd;
}

int runServer(int serverFd) {
    nfds_t clientMax = 1024;
    pollfd clients[clientMax];
    int maxi = 0;
    int pollReady;

    int bufSize = 2048;
    char buf[bufSize];
    memset(buf, '\0', bufSize);
    int readN, writeN;
    clients[0].fd = serverFd;
    clients[0].events = POLLRDNORM;

    for (int i = 1; i < clientMax; i++) {
        clients[i].fd = -1;
        clients[i].events = 0;
        clients[i].revents = 0;
    }

    while (true) {
        pollReady = poll(clients, maxi + 1, -1);
        if (pollReady == -1) {
            cout << "poll error: " << strerror(errno) << endl;
            break;
        }

        cout << "poll ready: " << pollReady << endl;
        // 首先检测是否有新的连接到来
        if (clients[0].revents & POLLRDNORM) {
            sockaddr_in clientAddr;
            socklen_t clientAddrLen;
            int connFd = accept(serverFd, (sockaddr *) &clientAddr, &clientAddrLen);
            if (connFd == -1) {
                cout << "accept error: " << strerror(errno) << endl;
                continue;
            }
            // 将新的连接加入到client
            int i;
            for (i = 1; i < clientMax; i++) {
                if (clients[i].fd < 0) {
                    clients[i].fd = connFd;
                    clients[i].events = POLLRDNORM;
                    break;
                }
            }

            if (maxi < i) {
                maxi = i;
            }

            if (--pollReady <= 0) {
                continue;
            }
        }

        // 否则处理客户端的写入
        for (int i = 1; i <= maxi; i++) {
            if (clients[i].revents & POLLRDNORM) {
                readN = recv(clients[i].fd, buf, bufSize, 0);
                if (readN < 0) {
                    cout << "recv error: " << strerror(errno) << endl;

                } else if (readN == 0) {
                    maxi--;
                    close(clients[i].fd);
                    clients[i].fd = -1;
                    clients[i].events = 0;
                    clients[i].revents = 0;

                } else {
                    writeN = send(clients[i].fd, buf, readN, 0);
                    if (writeN > 0) {
                        memset(buf, '\0', readN);
                    } else if (writeN < 0) {
                        cout << "send error: " << strerror(errno) << endl;
                    }
                }
            }
        }
    }
}

int main(int argc, const char *argv[]) {
    in_addr_t addr;
    in_port_t port;
    string listenInfo;

    if (argc != 3) {
        addr = INADDR_ANY;
        port = htons(34567);
        listenInfo = "0.0.0.0:34567";
    } else {
        inet_pton(AF_INET, argv[1], &addr);
        port = htons(atoi(argv[2]));
        listenInfo = argv[1];
        listenInfo.append(":");
        listenInfo.append(argv[2]);
    }

    int serverFd = createServer(addr, port, 128);
    if (serverFd == -1) {
        cout << "listen server error: " << strerror(errno) << endl;
        return 0;
    }

    cout << listenInfo << endl;
    runServer(serverFd);
}