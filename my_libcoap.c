#include "my_libcoap.h"
#include "my_coaphnd.h"

extern time_t clock_offset;
extern time_t my_clock_base = 0;
extern int my_sensor_base = 0;
extern struct coap_resource_t *time_resource = NULL;
extern coap_async_state_t *async = NULL;

void
init_resources(coap_context_t *ctx) {
	coap_resource_t *r;

	r = coap_resource_init(NULL, 0, 0);
	coap_register_handler(r, COAP_REQUEST_GET, hnd_get_index);

	coap_add_attr(r, (unsigned char *)"ct", 2, (unsigned char *)"0", 1, 0);
	coap_add_attr(r, (unsigned char *)"title", 5, (unsigned char *)"\"General Info\"", 14, 0);
	coap_add_resource(ctx, r);

	/* store clock base to use in /time */
	my_clock_base = clock_offset;

	r = coap_resource_init((unsigned char *)"time", 4, COAP_RESOURCE_FLAGS_NOTIFY_CON);
	coap_register_handler(r, COAP_REQUEST_GET, hnd_get_time);
	coap_register_handler(r, COAP_REQUEST_PUT, hnd_put_time);
	coap_register_handler(r, COAP_REQUEST_DELETE, hnd_delete_time);

	coap_add_attr(r, (unsigned char *)"ct", 2, (unsigned char *)"0", 1, 0);
	coap_add_attr(r, (unsigned char *)"title", 5, (unsigned char *)"\"Internal Clock\"", 16, 0);
	coap_add_attr(r, (unsigned char *)"rt", 2, (unsigned char *)"\"Ticks\"", 7, 0);
	r->observable = 1;
	coap_add_attr(r, (unsigned char *)"if", 2, (unsigned char *)"\"clock\"", 7, 0);

	coap_add_resource(ctx, r);
	time_resource = r;

	r = coap_resource_init((unsigned char *)"sensor", strlen("sensor"), COAP_RESOURCE_FLAGS_NOTIFY_CON);
	coap_register_handler(r, COAP_REQUEST_PUT, hnd_put_sensor);

	coap_add_attr(r, (unsigned char *)"ct", 2, (unsigned char *)"0", 1, 0);
	coap_add_attr(r, (unsigned char *)"title", 5, (unsigned char *)"\"General Info\"", 14, 0);
	coap_add_resource(ctx, r);

	r = coap_resource_init((unsigned char *)"async", 5, 0);
	coap_register_handler(r, COAP_REQUEST_GET, hnd_get_async);

	coap_add_attr(r, (unsigned char *)"ct", 2, (unsigned char *)"0", 1, 0);
	coap_add_resource(ctx, r);
}

void
fill_keystore(coap_context_t *ctx) {
	static uint8_t key[] = "secretPSK";
	size_t key_len = sizeof(key) - 1;
	coap_context_set_psk(ctx, "CoAP", key, key_len);
}

void
usage(const char *program, const char *version) {
	const char *p;

	p = strrchr(program, '/');
	if (p)
		program = ++p;

	fprintf(stderr, "%s v%s -- a small CoAP implementation\n"
		"(c) 2010,2011,2015 Olaf Bergmann <bergmann@tzi.org>\n\n"
		"usage: %s [-A address] [-p port]\n\n"
		"\t-A address\tinterface address to bind to\n"
		"\t-g group\tjoin the given multicast group\n"
		"\t-p port\t\tlisten on specified port\n"
		"\t-v num\t\tverbosity level (default: 3)\n"
		"\t-l list\t\tFail to send some datagram specified by a comma separated list of number or number intervals(for debugging only)\n"
		"\t-l loss%%\t\tRandmoly fail to send datagrams with the specified probability(for debugging only)\n",
		program, version, program);
}

coap_context_t*
get_context(const char *node, const char *port) {
	coap_context_t *ctx = NULL;
	int s;
	struct addrinfo hints;
	struct addrinfo *result, *rp;

	ctx = coap_new_context(NULL);
	if (!ctx) {
		return NULL;
	}

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
	hints.ai_socktype = SOCK_DGRAM; /* Coap uses UDP */
	hints.ai_flags = AI_PASSIVE | AI_NUMERICHOST;

	s = getaddrinfo(node, port, &hints, &result);
	if (s != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
		coap_free_context(ctx);
		return NULL;
	}

	/* iterate through results until success */
	for (rp = result; rp != NULL; rp = rp->ai_next) {
		coap_address_t addr, addrs;
		coap_endpoint_t *ep_udp = NULL, *ep_dtls = NULL, *ep_tcp = NULL, *ep_tls = NULL;

		if (rp->ai_addrlen <= sizeof(addr.addr)) {
			coap_address_init(&addr);
			addr.size = rp->ai_addrlen;
			memcpy(&addr.addr, rp->ai_addr, rp->ai_addrlen);
			addrs = addr;
			if (addr.addr.sa.sa_family == AF_INET) {
				addrs.addr.sin.sin_port = htons(ntohs(addr.addr.sin.sin_port) + 1);
			}
			else if (addr.addr.sa.sa_family == AF_INET6) {
				addrs.addr.sin6.sin6_port = htons(ntohs(addr.addr.sin6.sin6_port) + 1);
			}
			else {
				goto finish;
			}

			ep_udp = coap_new_endpoint(ctx, &addr, COAP_PROTO_UDP);
			if (ep_udp) {
				if (coap_dtls_is_supported()) {
					ep_dtls = coap_new_endpoint(ctx, &addrs, COAP_PROTO_DTLS);
					if (!ep_dtls)
						coap_log(LOG_CRIT, "cannot create DTLS endpoint\n");
				}
			}
			else {
				coap_log(LOG_CRIT, "cannot create UDP endpoint\n");
				continue;
			}
			ep_tcp = coap_new_endpoint(ctx, &addr, COAP_PROTO_TCP);
			if (ep_tcp) {
				if (coap_tls_is_supported()) {
					ep_tls = coap_new_endpoint(ctx, &addrs, COAP_PROTO_TLS);
					if (!ep_tls)
						coap_log(LOG_CRIT, "cannot create TLS endpoint\n");
				}
			}
			else {
				coap_log(LOG_CRIT, "cannot create TCP endpoint\n");
			}
			if (ep_udp)
				goto finish;
		}
	}

	fprintf(stderr, "no context available for interface '%s'\n", node);

finish:
	freeaddrinfo(result);
	return ctx;
}

int
join(coap_context_t *ctx, char *group_name) {
	struct ipv6_mreq mreq;
	struct addrinfo   *reslocal = NULL, *resmulti = NULL, hints, *ainfo;
	int result = -1;

	/* we have to resolve the link-local interface to get the interface id */
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET6;
	hints.ai_socktype = SOCK_DGRAM;

	result = getaddrinfo("::", NULL, &hints, &reslocal);
	if (result != 0) {
		fprintf(stderr, "join: cannot resolve link-local interface: %s\n",
			gai_strerror(result));
		goto finish;
	}

	/* get the first suitable interface identifier */
	for (ainfo = reslocal; ainfo != NULL; ainfo = ainfo->ai_next) {
		if (ainfo->ai_family == AF_INET6) {
			mreq.ipv6mr_interface =
				((struct sockaddr_in6 *)ainfo->ai_addr)->sin6_scope_id;
			break;
		}
	}

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET6;
	hints.ai_socktype = SOCK_DGRAM;

	/* resolve the multicast group address */
	result = getaddrinfo(group_name, NULL, &hints, &resmulti);

	if (result != 0) {
		fprintf(stderr, "join: cannot resolve multicast address: %s\n",
			gai_strerror(result));
		goto finish;
	}

	for (ainfo = resmulti; ainfo != NULL; ainfo = ainfo->ai_next) {
		if (ainfo->ai_family == AF_INET6) {
			mreq.ipv6mr_multiaddr =
				((struct sockaddr_in6 *)ainfo->ai_addr)->sin6_addr;
			break;
		}
	}

	if (ctx->endpoint) {
		result = setsockopt(ctx->endpoint->sock.fd, IPPROTO_IPV6, IPV6_JOIN_GROUP, (char *)&mreq, sizeof(mreq));
		if (result == COAP_SOCKET_ERROR) {
			fprintf(stderr, "join: setsockopt: %s\n", coap_socket_strerror());
		}
	}
	else {
		result = -1;
	}

finish:
	freeaddrinfo(resmulti);
	freeaddrinfo(reslocal);

	return result;
}

void
check_async(coap_context_t *ctx,
	coap_tick_t now) {
	coap_pdu_t *response;
	coap_async_state_t *tmp;

	size_t size = 13;

	if (!async || now < async->created + (unsigned long)async->appdata)
		return;

	response = coap_pdu_init(async->flags & COAP_ASYNC_CONFIRM
		? COAP_MESSAGE_CON
		: COAP_MESSAGE_NON,
		COAP_RESPONSE_CODE(205), 0, size);
	if (!response) {
		debug("check_async: insufficient memory, we'll try later\n");
		async->appdata =
			(void *)((unsigned long)async->appdata + 15 * COAP_TICKS_PER_SECOND);
		return;
	}

	response->tid = coap_new_message_id(async->session);

	if (async->tokenlen)
		coap_add_token(response, async->tokenlen, async->token);

	coap_add_data(response, 4, (unsigned char *)"done");

	if (coap_send(async->session, response) == COAP_INVALID_TID) {
		debug("check_async: cannot send response for message\n");
	}
	coap_remove_async(ctx, async->session, async->id, &tmp);
	coap_free_async(async);
	async = NULL;
}

void
libcoap_setup(void) {

}
