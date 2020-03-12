#include <iostream>
#include <cstring>
#include <cctype>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

enum PARSE_STATE {REQ_LINE = 0, REQ_LINE_OK, REQ_LINE_BAD, REQ_HEADER, REQ_HEADER_OK, REQ_HEADER_BAD, REQ_OK, REQ_BAD};

enum LINE_STATUS {LINE_OK = 0, LINE_BAD, LINE_OPEN };

enum HTTP_STATE {PARSE_REQUEST_RECV = 0, PARSE_REQUEST_OK, PARSE_REQUEST_BAD};

class HttpParser
{

public:
    HttpParser();
    ~HttpParser() = default;
    enum HTTP_STATE parse(char* buffer, const size_t& bufferLength);
private:
    size_t __checkIndex;
    size_t __readLineIndex;
    const char* readHttpLine(char* buffer, const size_t& bufferLength, enum LINE_STATUS& state);
    void parseRequestLine(const char* line, const size_t& lineLength);
    void parseRequestHeader(const char* line, const size_t& lineLength);
    enum PARSE_STATE __parseState;
};

HttpParser::HttpParser() {
    __checkIndex = 0;
    __readLineIndex = 0;
    __parseState = PARSE_STATE::REQ_LINE;
}


const char* HttpParser::readHttpLine(char* buffer, const size_t& bufferLength, enum LINE_STATUS& state) {
    for (;__checkIndex < bufferLength; __checkIndex++) {
        if (buffer[__checkIndex] == '\r') {
                // 表示读取完本次的缓冲区, 但没有读到一个完整到http line报文
                if (__checkIndex + 1 == bufferLength) {
                    state = LINE_STATUS::LINE_OPEN;
                    return nullptr;

                     // \r\n, 完整到读取到了一个http line 报文
                }else if (buffer[__checkIndex+1] == '\n') {
                    buffer[__checkIndex++] = '\0';
                    buffer[__checkIndex++] = '\0';
                    state = LINE_STATUS::LINE_OK;
                    // 根据当前读取到的偏移值获取line
                    const char* line = buffer + __readLineIndex;
                    // 更新偏移
                    __readLineIndex = __checkIndex;
                    return line;
                }

                state = LINE_STATUS::LINE_BAD;
                return nullptr;

        } else if(buffer[__checkIndex] == '\n') {
            if (__checkIndex > 1 && buffer[__checkIndex-1] == '\r') {
                    buffer[__checkIndex++] = '\0';
                    buffer[__checkIndex++] = '\0';
                    state = LINE_STATUS::LINE_OK;
                    // 根据当前读取到的偏移值获取line
                    const char* line = buffer + __readLineIndex;
                    // 更新偏移
                    __readLineIndex = __checkIndex;
                    return line;
                }

                state = LINE_STATUS::LINE_BAD;
                return nullptr;
        }

    }

    state = LINE_STATUS::LINE_OPEN;

    return nullptr;
}

void HttpParser::parseRequestLine(const char* line, const size_t& lineLength) {
    char method[255];
    size_t i = 0, j = 0;
    while (!isspace(line[i]) && i < lineLength) {
        method[j++] = line[i++];
    }

    method[j] = '\0';

    while(isspace(line[i]) && i < lineLength) {
        i++;
    }


    char path[255];
    j = 0;
    while(!isspace(line[i]) && i < lineLength) {
        path[j++] = line[i++];
    }
    path[j] = '\0';

    while(isspace(line[i]) && i < lineLength) {
        i++;
    }

    char version[255];
    j = 0;
    while(i < lineLength) {
        version[j++] = line[i++];
    }
    version[j] = '\0';
    std::cout << "method: " << method << std::endl;
    std::cout << "path: " << path<< std::endl;
    std::cout << "version: " << version << std::endl;

    __parseState = PARSE_STATE::REQ_LINE_OK;
}


enum HTTP_STATE HttpParser::parse(char* buffer, const size_t& bufferLength) {
    enum LINE_STATUS lineStatus;
    const char* line;
    for(;;) {
        line = readHttpLine(buffer, bufferLength, lineStatus);
        if (lineStatus != LINE_STATUS::LINE_OK) {
            break;
        }

        switch (__parseState) {
        case PARSE_STATE::REQ_LINE:
            parseRequestLine(line, strlen(line));
            break;

        case PARSE_STATE::REQ_LINE_OK: // 解析请求行ok
            // 转变状态, 解析header
            __parseState = REQ_HEADER;
            break;

        case PARSE_STATE::REQ_LINE_BAD: // 解析请起行失败
            __parseState = REQ_BAD;
            break;

        case PARSE_STATE::REQ_HEADER:
            parseRequestHeader(line, strlen(line));
            break;

        case PARSE_STATE::REQ_HEADER_OK: // 解析请求头ok
            __parseState = REQ_OK;
            break;

        case PARSE_STATE::REQ_HEADER_BAD: // 解析请求头失败
            __parseState = REQ_BAD;
            break;

        case PARSE_STATE::REQ_OK:
            return HTTP_STATE::PARSE_REQUEST_OK;

        case PARSE_STATE::REQ_BAD:
            return HTTP_STATE::PARSE_REQUEST_BAD;
        }
    }

    switch (lineStatus) {
    case LINE_STATUS::LINE_OPEN:
        return HTTP_STATE::PARSE_REQUEST_RECV;
    case LINE_STATUS::LINE_BAD:
        return HTTP_STATE::PARSE_REQUEST_BAD;
    default:
        return HTTP_STATE::PARSE_REQUEST_OK;
    }
}


void HttpParser::parseRequestHeader(const char* line, const size_t& lineLength) {
    std::cout << line << std::endl;
    __parseState = PARSE_STATE::REQ_HEADER_OK;
}

int main()
{
    int serverfd = socket(AF_INET, SOCK_STREAM, 0);
    int reuse = 1;
    setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    struct sockaddr_in addr;
    bzero(&addr, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9090);
    addr.sin_addr.s_addr = INADDR_ANY;

    int ret = bind(serverfd, (struct sockaddr*)&addr, sizeof(struct sockaddr_in));
    if (ret == -1) {
        std::cout << "bind error" << std::endl;
        close(serverfd);
        return 0;
    }

    ret = listen(serverfd, 128);
    if (ret == -1) {
        std::cout << "listen error" << std::endl;
        close(serverfd);
        return 0;
    }

    struct sockaddr_in cliAddr;
    socklen_t cliAddrLen = sizeof(struct sockaddr_in);


    int connfd = accept(serverfd, (struct sockaddr*)&cliAddr, &cliAddrLen);
    if (connfd == -1) {
        std::cout << "listen error" << std::endl;
        close(serverfd);
        return 0;
    }

    HttpParser parser;
    size_t readIndex = 0;
    size_t bufsize = 4;
    char* buf = new char[bufsize];
    for(;;) {
//       std::cout << "read buf size: " << bufsize - readIndex << std::endl;
       ret = recv(connfd, buf + readIndex, bufsize - readIndex, 0);
//       std::cout << "read ret size: " << ret << std::endl;
       if (ret == 0) {
           std::cout << "peer closed" << std::endl;
           return 0;
       } else if(ret == -1) {
           std::cout << "recv error" << std::endl;
           return 0;
       }
       readIndex += ret;
       enum HTTP_STATE state = parser.parse(buf, readIndex);
       if (state == HTTP_STATE::PARSE_REQUEST_RECV) {
           bufsize = readIndex * 2;
           char* newbuf = new char[bufsize + 1];
           memcpy(newbuf, buf, readIndex);
           delete []buf;
           buf = newbuf;
           continue;
       } else if (state == HTTP_STATE::PARSE_REQUEST_OK){
           std::cout << "parse ok" << std::endl;
           break;
       } else if (state == HTTP_STATE::PARSE_REQUEST_BAD) {
           std::cout << "parse bad" << std::endl;
           break;
       }
    }
}
