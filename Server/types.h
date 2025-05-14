#ifndef TYPES_H
#define TYPES_H

#include <iostream>

enum RequestType : uint8_t {
    LOGIN = 0,
    REGISTER = 1,
    GET_CHATS = 2,
    FIND_USERS = 3,
    SEND_MESSAGE = 4,
    MAX_VALUE = 127
};

enum ResponseType : uint8_t {
    RESPONSE_LOGIN = 128,
    RESPONSE_REGISTER = 129,
    RESPONSE_GET_CHATS = 130,
    RESPONSE_GET_CHATS_EMPTY = 131,
    RESPONSE_FIND_USERS = 132,
    RESPONSE_SEND_MESSAGE = 133,
    RESPONSE_ERROR = 255,
};

#endif // TYPES_H
