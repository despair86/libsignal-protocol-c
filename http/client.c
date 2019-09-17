/*
 * Copyright (C) 2019 Rick V. All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 * HTTP client for loki-msgr
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>
#ifdef _WIN32
#include <windows.h>
#include <wincrypt.h>
#ifdef _MSC_VER
#include <malloc.h>
char *
strtok_r(char * __restrict s, const char * __restrict delim, char * * __restrict last);
#endif
#else
#include <sys/utsname.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif
#include "http.h"

/* PolarSSL */
#include <mbedtls/ssl.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/net_sockets.h>
#include <mbedtls/error.h>
#include <mbedtls/certs.h>
#include <mbedtls/base64.h>

/* PolarSSL internal state */
mbedtls_net_context server_fd;
mbedtls_entropy_context entropy;
mbedtls_ctr_drbg_context ctr_drbg;
mbedtls_ssl_context ssl;
mbedtls_ssl_config conf;
mbedtls_x509_crt cacert;
unsigned char* ca_certs;

/* imageboard ref just because */
static char userAgent[] = "Loki_Pager/0.1 PolarSSL/2.16.2; U; ";

typedef struct url_parser_url
{
    char *protocol;
    char *host;
    int port;
    char *path;
    char *query_string;
    int host_exists;
    char *host_ip;
} url_parser_url_t;

static const char* seed = "Loki Pager HTTPS client";

/* If this fails, do NOT use the HTTP client */
bool http_client_init()
{
#ifdef _WIN32
    DWORD version, major, minor, build;
    char* arch;
#endif
    int r;
    char str[512];
    char* ua;
    size_t s;
    FILE* certs;

    mbedtls_net_init(&server_fd);
    mbedtls_ssl_init(&ssl);
    mbedtls_ssl_config_init(&conf);
    /* Everything below this comment is persistent throughout the app's 
     * lifetime */
    mbedtls_x509_crt_init(&cacert);
    mbedtls_entropy_init(&entropy);
    mbedtls_ctr_drbg_init(&ctr_drbg);

    certs = fopen("rootcerts.pem", "rb");
    if (!certs)
    {
        fprintf(stderr, "root certs not found, aborting\n");
        return false;
    }
    ca_certs = malloc(524288);
    assert(ca_certs);
    s = fread(ca_certs, 1, 524288, certs);
    ca_certs[s] = 0;
    r = mbedtls_x509_crt_parse(&cacert, ca_certs, s + 1);
    if (r < 0)
    {
        mbedtls_strerror(r, str, 512);
        printf("parse ca cert store failed\n  !  mbedtls_x509_crt_parse returned: %s\n\n", str);
        return false;
    }
    if (mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, (unsigned char*) seed, strlen(seed)) != 0)
        return false;
    /* fill in user-agent string */
#ifdef _WIN32
    version = GetVersion();
    major = (DWORD) (LOBYTE(LOWORD(version)));
    minor = (DWORD) (HIBYTE(LOWORD(version)));
    if (version < 0x80000000)
        build = (DWORD) (HIWORD(version));
    ua = malloc(512);
    arch = getenv("PROCESSOR_ARCHITECTURE");
    snprintf(ua, 512, "%sWindows NT %u.%u; %s", userAgent, major, minor, arch);
    client_ua = ua;
#else
    ua = malloc(512);
    assert(ua);
    struct utsname sys_name;
    uname(&sys_name);
#if (__x86_64__ || __amd64__) && defined(__sun)
    snprintf(sys_name.machine, _SYS_NMLN, "x86_64");
#endif
    snprintf(ua, 512, "%s%s %s; %s", userAgent, sys_name.sysname, sys_name.release, sys_name.machine);
    client_ua = ua;
#endif
    fclose(certs);
    return true;
}

static bool initTLS()
{
    /* Clear only previous connection state */
    mbedtls_net_init(&server_fd);
    mbedtls_ssl_init(&ssl);
    mbedtls_ssl_config_init(&conf);
    return true;
}

static void free_parsed_url(url_parsed)
url_parser_url_t *url_parsed;
{
    if (url_parsed->protocol)
        free(url_parsed->protocol);
    if (url_parsed->host)
        free(url_parsed->host);
    if (url_parsed->path)
        free(url_parsed->path);
    if (url_parsed->query_string)
        free(url_parsed->query_string);

    free(url_parsed);
}

static parse_url(url, verify_host, parsed_url)
char *url;
bool verify_host;
url_parser_url_t *parsed_url;
{
    char *local_url;
    char *token;
    char *token_host;
    char *host_port;
    char *host_ip;

    char *token_ptr;
    char *host_token_ptr;

    char *path = NULL;

    /* Copy our string */
    local_url = strdup(url);

    token = strtok_r(local_url, ":", &token_ptr);
    parsed_url->protocol = strdup(token);

    /* Host:Port */
    token = strtok_r(NULL, "/", &token_ptr);
    if (token)
        host_port = strdup(token);
    else
        host_port = (char *) calloc(1, sizeof (char));

    token_host = strtok_r(host_port, ":", &host_token_ptr);
    parsed_url->host_ip = NULL;
    if (token_host)
    {
        parsed_url->host = strdup(token_host);

        if (verify_host)
        {
            struct hostent *host;
            host = gethostbyname(parsed_url->host);
            if (host != NULL)
            {
                parsed_url->host_ip = inet_ntoa(* (struct in_addr *) host->h_addr);
                parsed_url->host_exists = 1;
            }
            else
            {
                parsed_url->host_exists = 0;
            }
        }
        else
        {
            parsed_url->host_exists = -1;
        }
    }
    else
    {
        parsed_url->host_exists = -1;
        parsed_url->host = NULL;
    }

    /* Port */
    token_host = strtok_r(NULL, ":", &host_token_ptr);
    if (token_host)
        parsed_url->port = atoi(token_host);
    else
        parsed_url->port = 0;

    token_host = strtok_r(NULL, ":", &host_token_ptr);
    assert(token_host == NULL);

    token = strtok_r(NULL, "?", &token_ptr);
    parsed_url->path = NULL;
    if (token)
    {
        path = (char *) realloc(path, sizeof (char) * (strlen(token) + 2));
        memset(path, 0, sizeof (char) * (strlen(token) + 2));
        strcpy(path, "/");
        strcat(path, token);

        parsed_url->path = strdup(path);

        free(path);
    }
    else
    {
        parsed_url->path = (char *) malloc(sizeof (char) * 2);
        strcpy(parsed_url->path, "/");
    }

    token = strtok_r(NULL, "?", &token_ptr);
    if (token)
    {
        parsed_url->query_string = (char *) malloc(sizeof (char) * (strlen(token) + 1));
        strncpy(parsed_url->query_string, token, strlen(token));
    }
    else
    {
        parsed_url->query_string = NULL;
    }

    token = strtok_r(NULL, "?", &token_ptr);
    assert(token == NULL);

    free(local_url);
    free(host_port);
    return 0;
}

static void *memncat(a, an, b, bn, s)
const void *a, *b;
size_t an, bn, s;
{
    char *p = malloc(s * (an + bn));
    memset(p, '\0', s * (an + bn));
    memcpy(p, a, an * s);
    memcpy(p + an*s, b, bn * s);
    return p;
}

static bool open_tls_sock(host, port)
char* host, *port;
{
    int r;
    unsigned int flags;

    r = mbedtls_net_connect(&server_fd, host, port, MBEDTLS_NET_PROTO_TCP);
    if (r)
    {
        printf("error - failed to connect to server: %d\n", r);
        return false;
    }

    r = mbedtls_ssl_config_defaults(&conf, MBEDTLS_SSL_IS_CLIENT, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT);
    if (r)
    {
        printf("error - failed to set TLS options: %d\n", r);
        return false;
    }

    mbedtls_ssl_conf_authmode(&conf, MBEDTLS_SSL_VERIFY_REQUIRED);
    mbedtls_ssl_conf_ca_chain(&conf, &cacert, NULL);
    mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &ctr_drbg);

    r = mbedtls_ssl_setup(&ssl, &conf);
    if (r)
    {
        printf("error - failed to setup TLS session: %d\n", r);
        return false;
    }

    r = mbedtls_ssl_set_hostname(&ssl, host);

    if (r)
    {
        printf("error - failed to perform SNI: %d\n", r);
        return false;
    }

    mbedtls_ssl_set_bio(&ssl, &server_fd, mbedtls_net_send, mbedtls_net_recv, NULL);

    while ((r = mbedtls_ssl_handshake(&ssl)) != 0)
    {
        if (r != MBEDTLS_ERR_SSL_WANT_READ && r != MBEDTLS_ERR_SSL_WANT_WRITE)
        {
            printf(" failed\n  ! mbedtls_ssl_handshake returned -0x%x\n\n", -r);
            return false;
        }
    }
    if ((flags = mbedtls_ssl_get_verify_result(&ssl)) != 0)
    {
        char vrfy_buf[512];
        printf(" failed\n");
        mbedtls_x509_crt_verify_info(vrfy_buf, sizeof (vrfy_buf), "  ! ", flags);
        printf("%s\n", vrfy_buf);
        return false;
    }
    return true;
}

/* Response data/funcs */
struct HttpResponse
{
    char* body;
    int code;
    size_t size;
};

static void* response_realloc(opaque, ptr, size)
void* opaque, *ptr;
{
    return realloc(ptr, size);
}

static void response_body(opaque, data, size)
void* opaque;
const char* data;
{
    struct HttpResponse* response = (struct HttpResponse*) opaque;
    response->body = memncat(response->body, response->size, data, size, sizeof (char));
    response->size += size;
}

static void response_header(opaque, ckey, nkey, cvalue, nvalue)
void* opaque;
const char* ckey, *cvalue;
{
#if 0
    printf("%s, %d, %s, %d\n", ckey, nkey, cvalue, nvalue);
#endif
}

static void response_code(opaque, code)
void* opaque;
{
    struct HttpResponse* response = (struct HttpResponse*) opaque;
    response->code = code;
}

static const struct http_funcs callbacks = {
    response_realloc,
    response_body,
    response_header,
    response_code,
};

/* A one-shot HTTP client. */
/* In: URI (string), data (string, may be null) + size, extra headers (also optional), request type */
/* Out: Response, size of response */

/* Returns: HTTP status code in [ER]AX */
http_request(uri, headers, data, type, post_type, size, out, osize, debug)
char *uri, *headers;
unsigned char *data, *out;
http_verb type;
http_content_type post_type;
size_t size, *osize;
bool debug;
{
    int r, s, len;
#ifdef _MSC_VER
    char buf[1024], port[8], *rq;
#else
    char buf[1024], port[8], rq[size + 8192];
#endif
    char *rq_type = 0, *rq_headers = 0;
    url_parser_url_t *parsed_uri;
    struct HttpResponse rsp;
    struct http_roundtripper rt;

#ifdef _MSC_VER
    rq = alloca(size + 8192);
#endif
    http_init(&rt, callbacks, &rsp);
    rsp.size = 0;
    rsp.body = malloc(0); /* Need a valid pointer, but we can embiggen as we go */
    rsp.code = 0;

    if (!headers)
        headers = "";

    parsed_uri = malloc(sizeof (url_parser_url_t));
    assert(parsed_uri);
    memset(parsed_uri, 0, sizeof (url_parser_url_t));
    r = parse_url(uri, false, parsed_uri);

    if (r)
    {
        printf("Invalid URI pathspec\n");
        return -1;
    }

    initTLS();

    /* get host name, set port if blank */
    if (!strcmp("https", parsed_uri->protocol) && !parsed_uri->port)
        parsed_uri->port = 443;

    /*printf("connecting to %s on port %d...", parsed_uri->host, parsed_uri->port);*/

    snprintf(port, 8, "%d", parsed_uri->port);

    if (!open_tls_sock(parsed_uri->host, port))
    {
        fprintf(stderr, "Failed to connect to %s\n", parsed_uri->host);
        goto exit;
    }

    switch (type)
    {
    case GET:
        rq_type = "GET";
        break;
    case POST:
        rq_type = "POST";
        break;
    default:
        break;
    }

    switch (post_type)
    {
    case HTTP_ENCODED:
        snprintf(buf, 1024, "Content-Type: application/x-www-form-urlencoded\r\nContent-Length: %zu", size);
        rq_headers = strdup(buf);
        break;
    case HTTP_FORM_DATA:
        rq_headers = "Content-Type: multipart/form-data;boundary=\"LOKI_POST_DATA\"\r\n";
        break;
    case HTTP_JSON_DATA:
        snprintf(buf, 1024, "Content-Type: application/json\r\nContent-Length: %zu", size);
        rq_headers = strdup(buf);
    default:
        rq_headers = "";
        break;
    }

    snprintf(rq, 8192, "%s %s HTTP/1.0\r\nHost: %s\r\nUser-Agent: %s\r\n%s%s\r\n\r\n", rq_type, parsed_uri->path, parsed_uri->host, client_ua, headers, rq_headers);
    if (debug)
        printf("Request headers:\n--->%s<---\n", rq);

    s = strlen(rq);
    if (data && size)
    {
        memcpy(rq + s, data, size);
        s += size;
    }

    while (r = mbedtls_ssl_write(&ssl, (unsigned char*) rq, s) <= 0)
    {
        if (r != MBEDTLS_ERR_SSL_WANT_READ && r != MBEDTLS_ERR_SSL_WANT_WRITE)
        {
            printf("failed! error %d\n\n", r);
            goto exit;
        }
    }

    len = 0;
    s = 0;
    do
    {
        r = mbedtls_ssl_read(&ssl, (unsigned char*) buf, 1024);
        if (r <= 0)
            break;
        else
        {
            s = http_data(&rt, buf, r, &len);
        }
    }
    while (r && s);

    mbedtls_ssl_close_notify(&ssl);

    if (rsp.code != 200)
    {
        printf("An error occurred.\n");
        printf("Server response:\n%s", rsp.body);
        goto exit;
    }

    if (out)
        memmove(out, rsp.body, rsp.size);

    *osize = rsp.size;

exit:
    free_parsed_url(parsed_uri);
    r = rsp.code;
    http_free(&rt);
    if (post_type == HTTP_ENCODED || post_type == HTTP_JSON_DATA)
        free(rq_headers);
    return r;
}

void http_client_cleanup()
{
    mbedtls_x509_crt_free(&cacert);
    mbedtls_ctr_drbg_free(&ctr_drbg);
    mbedtls_entropy_free(&entropy);
    mbedtls_net_free(&server_fd);
    mbedtls_ssl_free(&ssl);
    mbedtls_ssl_config_free(&conf);
    free(ca_certs);
    free(client_ua);
}
