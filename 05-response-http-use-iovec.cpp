#include <iostream>

#include <cstring>
#include <cstdio>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/uio.h>
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
    if (stat(filename, &filestat) < 0) {
        std::cout << "file not found" << std::endl;
        return 0;
    }

    char filebuf[filestat.st_size + 1];
    memset(filebuf, '\0', filestat.st_size + 1);

    int fd = open(filename, O_RDONLY);
    read(fd, filebuf, filestat.st_size);

    int bufsize = 1024;
    int writelen = 0;
    char headerbuf[bufsize];
    writelen += snprintf(headerbuf, bufsize -1, "HTTP/1.1 200 OK\r\n");
    writelen += snprintf(headerbuf + writelen, bufsize-writelen-1, "Content-Length: %lld\r\n", filestat.st_size);
    writelen += snprintf(headerbuf+writelen,bufsize-writelen-1, "\r\n");
    struct iovec iov[2];

    iov[0].iov_base = headerbuf;
    iov[0].iov_len = writelen;
    iov[1].iov_base = filebuf;
    iov[1].iov_len = filestat.st_size;
    writev(clifd, iov, 2);

    return 0;
}
