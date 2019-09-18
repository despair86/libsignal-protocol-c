/*-
 * Copyright 2012 Matthew Endsley
 * All rights reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted providing that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef HTTP_HTTP_H
#define HTTP_HTTP_H

#if defined(__cplusplus)
extern "C" {
#endif
#if __STDC_VERSION__ >= 199901L
#include <stdbool.h>
#else
#define bool int
#define true 1
#define false 0
#endif
#include <stddef.h>

    typedef enum {
        GET,
        POST
    } http_verb;

    typedef enum {
        HTTP_NONE,
        HTTP_ENCODED,
        HTTP_FORM_DATA,
        HTTP_JSON_DATA
    } http_content_type;

    char* client_ua;

    /**
     * Callbacks for handling response data.
     *  realloc_scratch - reallocate memory, cannot fail. There will only
     *                    be one scratch buffer. Implemnentation may take
     *                    advantage of this fact.
     *  body - handle HTTP response body data
     *  header - handle an HTTP header key/value pair
     *  code - handle the HTTP status code for the response
     */
    struct http_funcs {
        void* (*realloc_scratch)(void* opaque, void* ptr, int size);
        void (*body)(void* opaque, const char* data, int size);
        void (*header)(void* opaque, const char* key, int nkey, const char* value, int nvalue);
        void (*code)(void* opqaue, int code);
    };

    struct http_roundtripper {
        struct http_funcs funcs;
        void *opaque;
        char *scratch;
        int code;
        int parsestate;
        int contentlength;
        int state;
        int nscratch;
        int nkey;
        int nvalue;
        int chunked;
    };

    /**
     * Initializes a roundtripper with the specified response functions. This must
     * be called before the rt object is used.
     */
    void http_init(struct http_roundtripper* rt, struct http_funcs, void* opaque);

    /**
     * Frees any scratch memory allocated during parsing.
     */
    void http_free(struct http_roundtripper* rt);

    /**
     * Parses a block of HTTP response data. Returns zero if the parser reached the
     * end of the response, or an error was encountered. Use http_iserror to check
     * for the presence of an error. Returns non-zero if more data is required for
     * the response.
     */
    int http_data(struct http_roundtripper* rt, const char* data, int size, int* read);

    /**
     * Returns non-zero if a completed parser encountered an error. If http_data did
     * not return non-zero, the results of this function are undefined.
     */
    int http_iserror(struct http_roundtripper* rt);

    /* Brings HTTP client to a working state. If this returns FALSE, you are on your own. */
    bool http_client_init();

    /* Cleans up any persistent data used by the web client. */
    void http_client_cleanup();

    /* A oneshot HTTP client. Probably even reentrant, in case of redirection. */
    /* IN: uri, headers, data, verb, content-type, request size, output buffer size */
    /* OUT: response, response size */
    /* RETURN: HTTP status code in [ER]AX (Or whatever the machine ABI designates return values in.) 
     * Writes at most size-1 bytes to user provided buffer. User can check the resulting value of osize,
     * and reallocate+reissue the request to get all the data. */
    /* If out and osize are NULL, all you will be able to get is the HTTP status code. 
     * If *osize is 0, you get no response data, regardless of whether out is a valid pointer or not.
     */
    int http_request(char *uri, char *headers, unsigned char *data, http_verb verb, http_content_type post_type, size_t size, unsigned char *out, size_t *osize, bool debug);

#if defined(__cplusplus)
}
#endif

#endif
