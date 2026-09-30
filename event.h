#ifndef EVENT_H

#define EVENT_H

enum event_owner_type {
    EVENT_SERVER,
    EVENT_CONNECTION
};

struct event {
   
    enum event_owner_type type;

    void *data;

};

#endif