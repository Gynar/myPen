#ifndef _MY_COAPHND
#define _MY_COAPHND

#include <coap/coap.h>
#include <coap/coap_dtls.h>

#define UNUSED_PARAM
#define INDEX "This is the server with libcoap (https://libcoap.net)\n" \
              "Ref. Olaf Bergmann <bergmann@tzi.org>\n\n"

void hnd_get_index
(coap_context_t *ctx, struct coap_resource_t *resource, coap_session_t *session,
	coap_pdu_t *request, str *token, str *query, coap_pdu_t *response);
void hnd_get_time
(coap_context_t *ctx, struct coap_resource_t *resource, coap_session_t *session, 
	coap_pdu_t *request, str *token, str *query, coap_pdu_t *response);
void hnd_put_time
(coap_context_t *ctx, struct coap_resource_t *resource, coap_session_t *session,
	coap_pdu_t *request, str *token, str *query, coap_pdu_t *response);
void hnd_delete_time
(coap_context_t *ctx, struct coap_resource_t *resource, coap_session_t *session,
	coap_pdu_t *request, str *token, str *query, coap_pdu_t *response);
void hnd_put_sensor
(coap_context_t *ctx, struct coap_resource_t *resource, coap_session_t *session,
	coap_pdu_t *request, str *token, str *query, coap_pdu_t *response);
void hnd_get_async
(coap_context_t *ctx, struct coap_resource_t *resource, coap_session_t *session,
	coap_pdu_t *request, str *token, str *query, coap_pdu_t *response);

#endif