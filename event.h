#ifndef EVENT_H

#define EVENT_H

struct event {
    
    int write;
    int read;
    int accept;

    struct connection *con;

};

#endif