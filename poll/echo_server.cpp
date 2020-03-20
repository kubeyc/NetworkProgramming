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
    // 细节：nfds_t 很难衡量，一般调用sysconf(_SC_OPEN_MAX)来获取当前系统允许的最大可打开的fd的数量
    long openMax = sysconf(_SC_OPEN_MAX);
    if (openMax == -1) {
        cout << "get sys open max error: " << strerror(errno) << endl;
        return -1;
    }
    nfds_t clientMax = openMax;
    cout << "set client max(_SC_OPEN_MAX) = " << openMax << endl;

    // 细节：poll需要分配一个pollfd结构的数组来维护客户信息
    pollfd clients[clientMax];

    int maxi = 0;
    int pollReady;

    int bufSize = 2048;
    char buf[bufSize];
    memset(buf, '\0', bufSize);
    int readN, writeN;

    // 细节：client数组的第一项分配给监听套接字，同时设置POLLRDNORM，让内核有新的连接时通过revents通知
    clients[0].fd = serverFd;
    clients[0].events = POLLRDNORM;

    // 细节：client数组其余各项的fd用-1表示所在的client未使用
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
            /*
             * 检查POLLERR 的原因在于：有些实现在一个连接上接收到RST时返回的是POLLERR 事件，
             * 而其他实现返回的只是POLLRDNORM 事件。不论哪种情形，我们都调用read ，
             * 当有错误发生时，read 将返回这个错误。当一个现有连接由它的客户终止时，我们就把它的fd成员置为-1
             * */
            if (clients[i].revents & (POLLRDNORM | POLLERR)) {
                readN = recv(clients[i].fd, buf, bufSize, 0);
                if (readN < 0) {
                    cout << "recv error: " << strerror(errno) << endl;

                } else if (readN == 0) {
                    // 细节：当一个客户端关闭连接后，需要设置client数组对应位置的fd为-1，以便后续其他连接使用
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