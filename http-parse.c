#include <stdio.h>
#include <string.h>

#include "http-parse.h"
#include "buffer.h"
#include "server.h"

int is_tchar(char c) {
    return (c >= 'A' && c <= 'Z') ||
           (c >= 'a' && c <= 'z') ||
           (c >= '0' && c <= '9') ||
           c == '!' || c == '#' || c == '$' ||
           c == '%' || c == '&' || c == '\'' ||
           c == '*' || c == '+' || c == '-' ||
           c == '.' || c == '^' || c == '_' ||
           c == '`' || c == '|' || c == '~';
}

enum http_parse_request_line_result http_parse_request_line(struct http_parser *parser, struct buffer *read_buf) {

    for (; parser->pos < read_buf->data_end; parser->pos += 1) {
        switch (parser->request_line_state) {

            case HTTP_PARSE_REQUEST_LINE_STATE_METHOD_START:
                
                switch (read_buf->data[parser->pos]) {

                    case ' ':
                        return HTTP_PARSE_REQUEST_LINE_RESULT_NO_METHOD;

                    default:
                        if (!is_tchar(read_buf->data[parser->pos])) {
                            return HTTP_PARSE_REQUEST_LINE_RESULT_MALFORMED;
                        }
                        parser->request_line_state = HTTP_PARSE_REQUEST_LINE_STATE_METHOD;
                        parser->http_method.start = parser->pos;
                        break;

                }
                break;

            case HTTP_PARSE_REQUEST_LINE_STATE_METHOD:

                switch (read_buf->data[parser->pos]) {

                    case ' ':
                        parser->http_method.len = parser->pos - parser->http_method.start;
                        parser->request_line_state = HTTP_PARSE_REQUEST_LINE_STATE_TARGET_START;
                        continue;

                    default:
                        if (!is_tchar(read_buf->data[parser->pos])) {
                            return HTTP_PARSE_REQUEST_LINE_RESULT_MALFORMED;
                        }
                        break;

                }

                break;

            case HTTP_PARSE_REQUEST_LINE_STATE_TARGET_START:

                switch (read_buf->data[parser->pos]) {

                    case ' ':
                        return HTTP_PARSE_REQUEST_LINE_RESULT_NO_TARGET;

                    default:
                        parser->http_target.start = parser->pos;
                        parser->request_line_state = HTTP_PARSE_REQUEST_LINE_STATE_TARGET;
                        break;

                }
                break;

            case HTTP_PARSE_REQUEST_LINE_STATE_TARGET:

                switch (read_buf->data[parser->pos]) {

                    case ' ':
                        parser->http_target.len = parser->pos - parser->http_target.start;
                        parser->request_line_state = HTTP_PARSE_REQUEST_LINE_STATE_VERSION_START;
                        continue;

                    default:
                        break;

                }
                break;

            case HTTP_PARSE_REQUEST_LINE_STATE_VERSION_START:

                switch (read_buf->data[parser->pos]) {

                    case '\r':
                        return HTTP_PARSE_REQUEST_LINE_RESULT_NO_VERSION;

                    default:
                        parser->http_version.start = parser->pos;
                        parser->request_line_state = HTTP_PARSE_REQUEST_LINE_STATE_VERSION;
                    
                }
                break;

            case HTTP_PARSE_REQUEST_LINE_STATE_VERSION:

                switch (read_buf->data[parser->pos]) {

                    case '\r':
                        if (parser->pos + 1 >= read_buf->data_end) {
                            return HTTP_PARSE_REQUEST_LINE_RESULT_INCOMPLETE;
                        }
                        if (read_buf->data[parser->pos + 1] == '\n') {
                            parser->http_version.len = parser->pos - parser->http_version.start;

                            parser->pos += 2;

                            printf("\nfinished reading request line");
                            fflush(stdout);

                            return HTTP_PARSE_REQUEST_LINE_RESULT_SUCCESS;
                        } else {
                            return HTTP_PARSE_REQUEST_LINE_RESULT_MALFORMED;
                        }

                    default:
                        break;

                }
                break;

        }
    }

    return HTTP_PARSE_REQUEST_LINE_RESULT_INCOMPLETE;
    
}

enum http_parse_result http_parse(struct http_parser *parser, struct buffer *read_buf) {

    enum http_parse_request_line_result request_line_result;

    while (1)

        switch (parser->state) {
            
            case HTTP_PARSE_REQUEST_LINE:

                request_line_result = http_parse_request_line(parser, read_buf);

                switch (request_line_result) {
                    case HTTP_PARSE_REQUEST_LINE_RESULT_SUCCESS:
                        parser->state = HTTP_PARSE_HEADERS;
                        continue;
                    case HTTP_PARSE_REQUEST_LINE_RESULT_INCOMPLETE:
                        return HTTP_PARSE_RESULT_INCOMPLETE;
                    default:
                        return HTTP_PARSE_RESULT_ERR;
                }
                break;

            case HTTP_PARSE_HEADERS:

                break;

        return HTTP_PARSE_RESULT_SUCCESS;

    }

}

int http_parser_init(struct http_parser *parser) {

    memset(parser, 0, sizeof(*parser));
    parser->state = HTTP_PARSE_REQUEST_LINE;

    return 0;

}