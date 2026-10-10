#ifndef HTTP_PARSE_H

#define HTTP_PARSE_H

#include "buffer.h"

enum http_parser_state {
    HTTP_PARSE_REQUEST_LINE,
    HTTP_PARSE_HEADERS
};

enum http_parse_result {
    HTTP_PARSE_RESULT_INCOMPLETE,
    HTTP_PARSE_RESULT_SUCCESS,
    HTTP_PARSE_RESULT_ERR
};

enum http_parse_request_line_result {
    HTTP_PARSE_REQUEST_LINE_RESULT_SUCCESS,
    HTTP_PARSE_REQUEST_LINE_RESULT_NO_METHOD,
    HTTP_PARSE_REQUEST_LINE_RESULT_NO_TARGET,
    HTTP_PARSE_REQUEST_LINE_RESULT_NO_VERSION,
    HTTP_PARSE_REQUEST_LINE_RESULT_INCOMPLETE,
    HTTP_PARSE_REQUEST_LINE_RESULT_MALFORMED
};

enum http_parser_request_line_state {
    HTTP_PARSE_REQUEST_LINE_STATE_METHOD,
    HTTP_PARSE_REQUEST_LINE_STATE_METHOD_START,
    HTTP_PARSE_REQUEST_LINE_STATE_TARGET,
    HTTP_PARSE_REQUEST_LINE_STATE_TARGET_START,
    HTTP_PARSE_REQUEST_LINE_STATE_VERSION,
    HTTP_PARSE_REQUEST_LINE_STATE_VERSION_START
};

enum http_parser_headers_state {
    HTTP_PARSE_HEADERS_STATE_START,
};

struct http_parser {

    enum http_parser_state state;
    
    enum http_parser_request_line_state request_line_state;
    enum http_parser_headers_state headers_state;

    size_t pos;
    
    struct buffer_range http_method;
    struct buffer_range http_target;
    struct buffer_range http_version;

};

int http_parser_init(struct http_parser *parser);

// http_parse always leaves parser pos pointing at next unprocessed byte
enum http_parse_result http_parse(struct http_parser *parser, struct buffer *read_buf);

#endif