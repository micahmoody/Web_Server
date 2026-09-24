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

//write buffer
#define INITIAL_WRITE_BUFFER_SIZE 4096
#define WRITE_BUFFER_RESIZE_FACTOR 2

//http
#define MAX_HEADER_COUNT 12

//www
#define DOCUMENT_ROOT "./www"
#define DEFAULT_PATH "./www/index.html"

//file serving
#define STREAM_CHUNK_SIZE 1024
#define FALLBACK_MIME_TYPE "application/octet-stream"

//404
#define HTTP_404_FILE_PATH "./www/notfound.html" //this path is not relative to document_root

#endif