cmake_minimum_required(VERSION 3.5)

project(NetworkProgramming LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(01-inet_ntoa-reentrant
    01-inet_ntoa-reentrant.cpp)

add_executable(02-backlog
    02-backlog.cpp)

add_executable(03-msgoob
    03-msgoob.cpp)

add_executable(04-parse-http
    04-parse-http.cpp)

add_executable(05-response-http-use-iovec
    05-response-http-use-iovec.cpp)

add_executable(06-response-http-use-sendfile
    06-response-http-use-sendfile.cpp)

add_executable(07-tee
        07-tee.cpp)

add_subdirectory(07)

add_executable(select_tcp_echo_server
        select_tcp_echo_server.cpp)

add_executable(tcp_echo_client
        tcp_echo_client.cpp)