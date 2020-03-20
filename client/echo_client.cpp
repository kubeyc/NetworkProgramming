#include <iostream>
#include <cstring>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using std::cout;
using std::endl;
using std::cin;

int connectServer(in_addr_t inAddr, in_port_t inPort) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        return -1;
    }

    sockaddr_in addr;
    bzero(&addr, sizeof(sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = inPort;
    addr.sin_addr.s_addr = inAddr;

    int callRet = connect(fd, (const sockaddr*)&addr, sizeof(sockaddr));
    if (callRet == -1) {
        return -1;
    }

    return fd;
}

int main(int argc, const char* argv[]) {
    if (argc != 3) {
        cout << "Usage: server_addr server_port" << endl;
        return 0;
    }

    in_port_t inPort = htons(atoi(argv[2]));
    in_addr_t inAddr;
    inet_pton(AF_INET, argv[1], &inAddr);

    cout << "connect to " << argv[1] << ":" << argv[2] << endl;

    int serverFd = connectServer(inAddr, inPort);
    if (serverFd == -1) {
        cout << "connect server error: " << strerror(errno) << endl;
        return 0;
    }

    int writeN, readN;
    int bufSize = 2048;
    char buf[bufSize];
    memset(buf, '\0', bufSize);
    while (true) {
        cin >> buf;
        writeN = send(serverFd, buf, strlen(buf), 0);
        if (writeN < 0) {
            cout << "send error: " << strerror(errno) << endl;
            break;
        }
        memset(buf, '\0', strlen(buf));

        readN = recv(serverFd, buf, bufSize, 0);
        if (readN == 0) {
            close(serverFd);
            break;
        }
        cout << "recv: " << buf << endl;
        memset(buf, '\0', readN);
    }
}