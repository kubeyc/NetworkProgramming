#include <iostream>
#include <fstream>

#include <cstring>
#include <cstdio>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/uio.h> // macos
#include <unistd.h>


int main() {
    int servfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(struct sockaddr_in));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(8090);

    bind(servfd, (struct sockaddr*)&servaddr, sizeof(struct sockaddr_in));
    listen(servfd, 128);

    struct sockaddr_in cliaddr;
    socklen_t clilen;
    int clifd = accept(servfd, (struct sockaddr*)&cliaddr, &clilen);

    struct stat filestat;
    const char* filename = "/Users/xyc/workspace/code/cpp/NetworkProgramming/05-response-http-use-iovec.cpp";


    // c
    if (stat(filename, &filestat) < 0) {
        std::cout << "file not found" << std::endl;
        return 0;
    }

    int fd = open(filename, O_RDONLY);

    int bufsize = 1024;
    char buf[bufsize];
    int sendlen = 0;
    int ret = 0;
    memset(buf, '\0', bufsize);
    ret = sprintf(buf, "HTTP/1.1 200 OK\r\n");
    send(clifd, buf, strlen(buf), 0);
    sendlen += ret;

    ret = sprintf(buf + sendlen, "Content-Length: %lld\r\n", filestat.st_size);
    send(clifd, buf+sendlen, strlen(buf+sendlen), 0);
    sendlen += ret;


    ret = sprintf(buf + sendlen, "\r\n");
    send(clifd, buf + sendlen, strlen(buf+sendlen), 0);
    sendlen += ret;

    sendfile(fd, clifd, 0, &filestat.st_size, nullptr, 0); // macos

}
