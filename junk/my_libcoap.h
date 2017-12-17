#ifndef _MY_LIBCOAP
#define _MY_LIBCOAP

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

void init_resources(coap_context_t *ctx);
void fill_keystore(coap_context_t *ctx);
void usage(const char *program, const char *version);
coap_context_t* get_context(const char *node, const char *port);
int join(coap_context_t *ctx, char *group_name);
void check_async(coap_context_t *ctx, coap_tick_t now);

coap_context_t* libcoap_open(void);
void libcoap_close(coap_context* ctx);


#endif