# HTTP Web Server

This is a practice project for continued experience in network and concurrent programming in C. It is not complete.

The server uses epoll() to select ready fds, which are set to nonblocking. Currently epoll_wait is edge-triggered but this will change to level-triggered after the rest of the server is passable.

Notes:

- the server will only compile on linux, as it uses unistd and defines _GNU_SOURCE
- it must be compiled with connection.c and http-parse.c