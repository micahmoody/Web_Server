# HTTP Web Server

This is a practice project for continued experience in network and concurrent programming in C. 

As of Sep 26 2026, it works (accepts clients, reads and parses HTTP requests, constructs and sends back an HTTP response and file), but it is not complete.

## How it works

The server uses edge-triggered epoll() and nonblocking sockets to create an event-driven loop, which handles the listening socket, readable client FDs and writable client FDS.

Requests from the client are parsed without copying any strings. Instead, pointers to the version, target path, headers, etc are stored in a connection-specific struct.

Once the full request by the client has been parsed, the server registers interest in writability for the client FD with the epoll FD set and constructs the HTTP response. Once the socket is writable, it sends the response followed by the file (using sendfile(), which copies bytes directly between sockets, rather than first storing it in a buffer in the server, which would be slower).

The server then disconnects the client. 

## Vulnerabilities & bugs

- resolve_target() will resolve the path to a file even if it is outside of the document root. This means a client could access potentially vulnerable files (e.g. http://server-ip:8080/../../../../etc/passwd).

- The server will respond with status code 200 even when sending a 404 page.

- Though unlikely, a particular connection could be closed even after triggering an event and before that event is handled. The server would then try to handle that event, causing use-after-free and bad FD issues.

- While headers extraction is fairly secure and detects malformed request, the request parsing function just assumes it's handed a line with two spaces. This would be very easy to exploit.

## Other intended additions

- Implement keep-alive. Currently the server disconnects after one successful read/write exchange.