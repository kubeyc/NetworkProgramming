#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <cstring>

#include <vector>
#include <iostream>
#include <deque>

using std::vector;
using std::deque;



class client {
public:
    client() = default;

    client(int connfd) : __conn(connfd) {}

    inline int get_conn() const { return __conn; };

    inline void set_conn(int connfd) { __conn = connfd; };

private:
    int __conn;
};

void serverLoop(int serverfd) {
    fd_set rset, allset;
    int clientUsage = 0;
    int readn, writen;
    int selectReady, maxfd = serverfd;

    FD_ZERO(&allset);
    FD_SET(serverfd, &allset);

    const int BUF_SIZE = 1024;
    char buf[BUF_SIZE];

    vector<int> clients(FD_SETSIZE, -1);

    for (;;) {
        rset = allset;
        selectReady = select(maxfd + 1, &rset, nullptr, nullptr, nullptr);
        if (selectReady == -1) {
            std::cout << "select failure." << std::endl;
            break;
        }

        std::cout << "select ready: " << selectReady << std::endl;

        // 在服务器fd上发生了可读事件
        if (FD_ISSET(serverfd, &rset)) {
            std::cout << "ready fd: " << serverfd << std::endl;
            int connFd;
            struct sockaddr_in connAddr;
            socklen_t connAddrLen = sizeof(struct sockaddr_in);
            connFd = accept(serverfd, (struct sockaddr *) &connAddr, &connAddrLen);
            std::cout << "conn fd: " << connFd << std::endl;

            for (int i = 0; i < clients.size(); i++) {
                if (clients[i] == -1) {
                    clients[i] = connFd;
                    clientUsage = i;
                    break;
                }
            }

            // 添加监听的fd
            FD_SET(connFd, &allset);

            // 当前能够监听的客户端数量超过select的最大值
            if (clientUsage == FD_SETSIZE) {
                std::cout << "too many connect." << std::endl;
            }

            //细节：有新的客户端加入连接后，设置当前select监听的最大的fd
            if (connFd > maxfd) {
                maxfd = connFd;
            }

            //细节：server监听到了有客户端连接，accept掉后，该事件就算被处理了，但此时新建立
            // 连接的客户端可能还没有发送数据，因此需要continue
            // 也有可能同时发生了已经建立好连接的客户端send数据，同时有另一个陌生的客户端建立连接，此时select ready可能是2,
            // 因此accept掉客户端连接事件后，还需要继续处理之前已经连接好的客户端send的数据
            if (--selectReady <= 0) {
                continue;
            }
        }

        // 在其他描述符中发生了可读事件
        for (int i = 0; i < clients.size(); i++) {
            if (clients[i] == -1) {
                continue;
            }

            // 测试是否是该客户端的fd发生了可读事件
            if (FD_ISSET(clients[i], &rset)) {
                readn = recv(clients[i], buf, BUF_SIZE, 0);
                if (readn == 0) {
                    std::cout << "client close connect." << std::endl;
                    FD_CLR(clients[i], &allset);
                    close(clients[i]);
                    clients[i] = -1;

                } else if (readn > 0) {
                    send(clients[i], buf, readn, 0);
                    memset(buf, '\0', readn);
                }
            }
        }
    }
}

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

    serverLoop(servfd);
}