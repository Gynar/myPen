#ifndef _COAP_IMPL_H
#define _COAP_IMPL_H

#include <stdio.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <dirent.h>
#include <coap/coap.h>
#include <coap/coap_dtls.h>

#define COAP_RESOURCE_CHECK_TIME 2
#define PORT 5683
#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif
#define UNUSED_PARAM
#define INDEX "This is the server with libcoap (https://libcoap.net)\n" \
              "Ref. Olaf Bergmann <bergmann@tzi.org>\n\n"

typedef struct {
	int Ax;
	int Ay;
	int Az;
	int Gx;
	int Gy;
	int Gz;
	int Fr;
	int renewed;
}recv_dat_t;

extern recv_dat_t recv_dat;
extern int con_established = 0;
extern struct coap_resource_t *time_resource = NULL;
extern time_t clock_offset; 

void 
	hnd_get_index(coap_context_t *ctx,
		struct coap_resource_t *resource, 
		coap_session_t *session,
		coap_pdu_t *request,
		str *token,
		str *query,
		coap_pdu_t *response);

void 
	hnd_get_time(coap_context_t *ctx,
		struct coap_resource_t *resource,
		coap_session_t *session,
		coap_pdu_t *request,
		str *token,
		str *query,
		coap_pdu_t *response);

void 
	hnd_put_time(coap_context_t *ctx,
		struct coap_resource_t *resource,
		coap_session_t *session,
		coap_pdu_t *request,
		str *token,
		str *query,
		coap_pdu_t *response);

void 
	hnd_delete_time(coap_context_t *ctx,
		struct coap_resource_t *resource,
		coap_session_t *session,
		coap_pdu_t *request,
		str *token,
		str *query,
		coap_pdu_t *response);

void 
	hnd_put_sensor(coap_context_t *ctx,
		struct coap_resource_t *resource,
		coap_session_t *session,
		coap_pdu_t *request,
		str *token,
		str *query,
		coap_pdu_t *response);

void
	hnd_get_async(coap_context_t *ctx,
		struct coap_resource_t *resource,
		coap_session_t *session,
		coap_pdu_t *request,
		str *token,
		str *query,
		coap_pdu_t *response);

void 
	init_resources(coap_context_t *ctx);

void 
	fill_keystore(coap_context_t *ctx);

void 
	usage(const char *program, const char *version);

coap_context_t* 
	get_context(const char *node, const char *port);

int 
	join(coap_context_t *ctx, char *group_name);

void 
	check_async(coap_context_t *ctx, coap_tick_t now);

#endif

