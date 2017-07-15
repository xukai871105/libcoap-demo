/* -*- Mode: C; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 * -*- */

/* coap -- simple implementation of the Constrained Application Protocol (CoAP)
 *         as defined in RFC 7252
 *
 * Copyright (C) 2010--2016 Olaf Bergmann <bergmann@tzi.org>
 *
 * This file is part of the CoAP library libcoap. Please see README for terms
 * of use.
 */

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <signal.h>

#include "coap_config.h"
#include <coap/resource.h>
#include <coap/coap.h>
#include <coap/coap_time.h>

#define COAP_RESOURCE_CHECK_TIME 2


/* temporary storage for dynamic resource representations */
static int quit = 0;

/* changeable clock base (see handle_put_time()) */
static time_t clock_offset;

#ifdef __GNUC__
#define UNUSED_PARAM __attribute__ ((unused))
#else /* not a GCC */
#define UNUSED_PARAM
#endif /* GCC */

/* SIGINT handler: set quit to 1 for graceful termination */
static void
handle_sigint(int signum UNUSED_PARAM) {
  quit = 1;
}

#define INDEX "Hello libcoap!\r\n"

static void
hnd_get_index(coap_context_t *ctx UNUSED_PARAM,
              struct coap_resource_t *resource UNUSED_PARAM,
              const coap_endpoint_t *local_interface UNUSED_PARAM,
              coap_address_t *peer UNUSED_PARAM,
              coap_pdu_t *request UNUSED_PARAM,
              str *token UNUSED_PARAM,
              coap_pdu_t *response) {
  unsigned char buf[3];

  response->hdr->code = COAP_RESPONSE_CODE(205);

  coap_add_option(response,
                  COAP_OPTION_CONTENT_TYPE,
                  coap_encode_var_bytes(buf, COAP_MEDIATYPE_TEXT_PLAIN), buf);

  coap_add_option(response,
                  COAP_OPTION_MAXAGE,
                  coap_encode_var_bytes(buf, 0x2ffff), buf);

  coap_add_data(response, strlen(INDEX), (unsigned char *)INDEX);
}

#define HELLO "Hello CoAP!\r\n"
static void
hnd_get_test(coap_context_t  *ctx,
             struct coap_resource_t *resource,
             const coap_endpoint_t *local_interface UNUSED_PARAM,
             coap_address_t *peer,
             coap_pdu_t *request,
             str *token,
             coap_pdu_t *response) {
  unsigned char buf[3];

  response->hdr->code = COAP_RESPONSE_CODE(205);

  coap_add_option(response,
                  COAP_OPTION_CONTENT_TYPE,
                  coap_encode_var_bytes(buf, COAP_MEDIATYPE_TEXT_PLAIN), buf);

  coap_add_data(response, strlen(HELLO), (unsigned char *)HELLO);

}


static void
init_resources(coap_context_t *ctx) 
{
  coap_resource_t *r;

  r = coap_resource_init(NULL, 0, 0);
  coap_register_handler(r, COAP_REQUEST_GET, hnd_get_index);

  coap_add_attr(r, (unsigned char *)"ct", 2, (unsigned char *)"0", 1, 0);
  coap_add_attr(r, (unsigned char *)"title", 5, (unsigned char *)"\"General Info\"", 14, 0);
  coap_add_resource(ctx, r);


  r = coap_resource_init((unsigned char *)"test", 4, COAP_RESOURCE_FLAGS_NOTIFY_CON);
  coap_register_handler(r, COAP_REQUEST_GET, hnd_get_test);

  coap_add_attr(r, (unsigned char *)"ct", 2, (unsigned char *)"0", 1, 0);
  coap_add_attr(r, (unsigned char *)"title", 5, (unsigned char *)"\"General Test\"", 16, 0);

  coap_add_resource(ctx, r);
}

static coap_context_t *
get_context(const char *node, const char *port) {
  coap_context_t *ctx = NULL;
  int s;
  struct addrinfo hints;
  struct addrinfo *result, *rp;

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
  hints.ai_socktype = SOCK_DGRAM; /* Coap uses UDP */
  hints.ai_flags = AI_PASSIVE | AI_NUMERICHOST;

  s = getaddrinfo(node, port, &hints, &result);
  if ( s != 0 ) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
    return NULL;
  }

  /* iterate through results until success */
  for (rp = result; rp != NULL; rp = rp->ai_next) {
    coap_address_t addr;

    if (rp->ai_addrlen <= sizeof(addr.addr)) {
      coap_address_init(&addr);
      addr.size = rp->ai_addrlen;
      memcpy(&addr.addr, rp->ai_addr, rp->ai_addrlen);

      ctx = coap_new_context(&addr);
      if (ctx) {
        /* TODO: output address:port for successful binding */
        goto finish;
      }
    }
  }

  fprintf(stderr, "no context available for interface '%s'\n", node);

  finish:
  freeaddrinfo(result);
  return ctx;
}


int main(int argc, char **argv) 
{
  coap_context_t  *ctx;

  fd_set readfds;
  struct timeval tv, *timeout;
  int result;
  coap_tick_t now;
  coap_queue_t *nextpdu;

  char addr_str[NI_MAXHOST] = "::";
  char port_str[NI_MAXSERV] = "5683";

  clock_offset = time(NULL);

  coap_log_t log_level = LOG_DEBUG;
  coap_set_log_level(log_level);
  // coap_set_log_level(LOG_INFO);

  // strncpy(addr_str, "127.0.0.1", NI_MAXHOST-1);
  // strncpy(port_str, "5683", NI_MAXSERV-1);
  ctx = get_context(addr_str, port_str);
  if (!ctx)
    return -1;

  init_resources(ctx);

  signal(SIGINT, handle_sigint);

  while ( !quit ) {
    FD_ZERO(&readfds);
    FD_SET( ctx->sockfd, &readfds );

    nextpdu = coap_peek_next( ctx );

    coap_ticks(&now);
    while (nextpdu && nextpdu->t <= now - ctx->sendqueue_basetime) {
      coap_retransmit( ctx, coap_pop_next( ctx ) );
      nextpdu = coap_peek_next( ctx );
    }

    if (nextpdu && nextpdu->t <= (COAP_RESOURCE_CHECK_TIME * COAP_TICKS_PER_SECOND)) {
      /* set timeout if there is a pdu to send before our automatic timeout occurs */
      tv.tv_usec = ((nextpdu->t) % COAP_TICKS_PER_SECOND) * 1000000 / COAP_TICKS_PER_SECOND;
      tv.tv_sec = (nextpdu->t) / COAP_TICKS_PER_SECOND;
      timeout = &tv;
    } else {
      tv.tv_usec = 0;
      tv.tv_sec = COAP_RESOURCE_CHECK_TIME;
      timeout = &tv;
    }
    result = select( FD_SETSIZE, &readfds, 0, 0, timeout );

    if ( result < 0 ) {         /* error */
      if (errno != EINTR)
        perror("select");
    } else if ( result > 0 ) {  /* read from socket */
      if ( FD_ISSET( ctx->sockfd, &readfds ) ) {
        coap_read( ctx );       /* read received data */
        /* coap_dispatch( ctx );  /\* and dispatch PDUs from receivequeue *\/ */
      }
    }

  }

  coap_free_context(ctx);

  return 0;
}
