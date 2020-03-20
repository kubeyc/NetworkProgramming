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
}

bool checkAccept(pollfd *ev, int evSize, int &pos) {
    if (ev[0].revents & POLLRDNORM) {
        sockaddr_in clientAddr;
        socklen_t clientAddrLen;
        int clientFd = accept(ev[0].fd, (sockaddr *) &clientAddr, &clientAddrLen);
        if (clientFd == -1) {
            cout << "accept error: " << strerror(errno) << endl;
            return false;
        }

        int i;
        for (i = 1; i < evSize; i++) {
            if (ev[i].fd < 0) {
                ev[i].fd = clientFd;
                ev[i].events = POLLRDNORM;
            }
        }

        if (i == evSize) {
            cout << "too many connect" << endl;
            return false;
        }

        if (i > pos) {
            pos = i;
        }
    }

    return true;
}

void echo(pollfd *ev, int i) {
    if (ev[i].revents & POLLRDNORM) {

    }
}

int runServer(int serverFd) {
    nfds_t evSize = 1024;
    pollfd ev[evSize];
    int maxi = 0;
    int pollReady;

    ev[0].fd = serverFd;
    ev[0].events = POLLRDNORM;

    for (int i = 0; i < evSize; i++) {
        ev[i].fd = -1;
        ev[i].events = 0;
        ev[i].revents = 0;
    }

    while (true) {
        pollReady = poll(ev, maxi + 1, -1);
        if (pollReady == -1) {
            cout << "poll error: " << strerror(errno) << endl;
            break;
        }

        cout << "poll ready: " << pollReady << endl;

        if (--pollReady <= 0) {
            continue;
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