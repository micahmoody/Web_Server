#ifndef CONSTANTS_H
#define CONSTANTS_H

//socket
#define PORT 8080
#define BACKLOG 10

//epoll
#define EPOLL_READY_SIZE 64

//read buffer
#define INITIAL_READ_BUFFER_SIZE 4096
#define MAX_READ_BUFFER_SIZE 65536
#define READ_BUFFER_RESIZE_FACTOR 2

//http
#define MAX_HEADER_COUNT 12

#endif