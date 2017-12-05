#include "my_coaphnd.h"

void
hnd_get_index(coap_context_t *ctx UNUSED_PARAM,
	struct coap_resource_t *resource UNUSED_PARAM,
	coap_session_t *session UNUSED_PARAM,
	coap_pdu_t *request UNUSED_PARAM,
	str *token UNUSED_PARAM,
	str *query UNUSED_PARAM,
	coap_pdu_t *response) {
	unsigned char buf[3];

	response->code = COAP_RESPONSE_CODE(205);

	coap_add_option(response,
		COAP_OPTION_CONTENT_TYPE,
		coap_encode_var_bytes(buf, COAP_MEDIATYPE_TEXT_PLAIN), buf);

	coap_add_option(response,
		COAP_OPTION_MAXAGE,
		coap_encode_var_bytes(buf, 0x2ffff), buf);

	coap_add_data(response, strlen(INDEX), (unsigned char *)INDEX);
}

void
hnd_get_time(coap_context_t  *ctx,
	struct coap_resource_t *resource,
	coap_session_t *session,
	coap_pdu_t *request,
	str *token,
	str *query,
	coap_pdu_t *response) {
	unsigned char buf[40];
	size_t len;
	time_t now;
	coap_tick_t t;
	(void)request;

	/* FIXME: return time, e.g. in human-readable by default and ticks
	* when query ?ticks is given. */

	/* if my_clock_base was deleted, we pretend to have no such resource */
	response->code =
		my_clock_base ? COAP_RESPONSE_CODE(205) : COAP_RESPONSE_CODE(404);

	if (coap_find_observer(resource, session, token)) {
		coap_add_option(response,
			COAP_OPTION_OBSERVE,
			coap_encode_var_bytes(buf, resource->observe), buf);
	}

	if (my_clock_base)
		coap_add_option(response,
			COAP_OPTION_CONTENT_FORMAT,
			coap_encode_var_bytes(buf, COAP_MEDIATYPE_TEXT_PLAIN), buf);

	coap_add_option(response,
		COAP_OPTION_MAXAGE,
		coap_encode_var_bytes(buf, 0x01), buf);

	if (my_clock_base) {

		/* calculate current time */
		coap_ticks(&t);
		now = my_clock_base + (t / COAP_TICKS_PER_SECOND);

		if (query != NULL
			&& memcmp(query->s, "ticks", min(5, query->length)) == 0) {
			/* output ticks */
			len = snprintf((char *)buf, sizeof(buf), "%u", (unsigned int)now);
			coap_add_data(response, len, buf);

		}
		else {      /* output human-readable time */
			struct tm *tmp;
			tmp = gmtime(&now);
			len = strftime((char *)buf, sizeof(buf), "%b %d %H:%M:%S", tmp);
			coap_add_data(response, len, buf);
		}
	}
}

void
hnd_put_time(coap_context_t *ctx UNUSED_PARAM,
	struct coap_resource_t *resource,
	coap_session_t *session UNUSED_PARAM,
	coap_pdu_t *request,
	str *token UNUSED_PARAM,
	str *query UNUSED_PARAM,
	coap_pdu_t *response) {
	coap_tick_t t;
	size_t size;
	unsigned char *data;

	/* FIXME: re-set my_clock_base to clock_offset if my_clock_base == 0
	* and request is empty. When not empty, set to value in request payload
	* (insist on query ?ticks). Return Created or Ok.
	*/

	/* if my_clock_base was deleted, we pretend to have no such resource */
	response->code =
		my_clock_base ? COAP_RESPONSE_CODE(204) : COAP_RESPONSE_CODE(201);

	coap_resource_set_dirty(resource, NULL);

	/* coap_get_data() sets size to 0 on error */
	(void)coap_get_data(request, &size, &data);

	if (size == 0)        /* re-init */
		my_clock_base = clock_offset;
	else {
		my_clock_base = 0;
		coap_ticks(&t);
		while (size--)
			my_clock_base = my_clock_base * 10 + *data++;
		my_clock_base -= t / COAP_TICKS_PER_SECOND;
	}
}

void
hnd_delete_time(coap_context_t *ctx UNUSED_PARAM,
	struct coap_resource_t *resource UNUSED_PARAM,
	coap_session_t *session UNUSED_PARAM,
	coap_pdu_t *request UNUSED_PARAM,
	str *token UNUSED_PARAM,
	str *query UNUSED_PARAM,
	coap_pdu_t *response UNUSED_PARAM) {
	my_clock_base = 0;    /* mark clock as "deleted" */

						  /* type = request->hdr->type == COAP_MESSAGE_CON  */
						  /*   ? COAP_MESSAGE_ACK : COAP_MESSAGE_NON; */
}

void
hnd_put_sensor(coap_context_t *ctx UNUSED_PARAM,
	struct coap_resource_t *resource UNUSED_PARAM,
	coap_session_t *session UNUSED_PARAM,
	coap_pdu_t *request,
	str *token UNUSED_PARAM,
	str *query UNUSED_PARAM,
	coap_pdu_t *response) {

	size_t size;
	unsigned char *data;

	response->code = my_sensor_base ? COAP_RESPONSE_CODE(204) : COAP_RESPONSE_CODE(201);
	coap_resource_set_dirty(resource, NULL);

	(void)coap_get_data(request, &size, &data);

	if (size == 0) {
		response->code = COAP_RESPONSE_CODE(400);
	}
	else {
		my_sensor_base = 1;
		info("recived val : %s \n", data);
	}
}


void
hnd_get_async(coap_context_t *ctx,
	struct coap_resource_t *resource UNUSED_PARAM,
	coap_session_t *session,
	coap_pdu_t *request,
	str *token UNUSED_PARAM,
	str *query UNUSED_PARAM,
	coap_pdu_t *response) {
	unsigned long delay = 5;
	size_t size;

	if (async) {
		if (async->id != request->tid) {
			coap_opt_filter_t f;
			coap_option_filter_clear(f);
			response->code = COAP_RESPONSE_CODE(503);
		}
		return;
	}

	if (query) {
		unsigned char *p = query->s;

		delay = 0;
		for (size = query->length; size; --size, ++p)
			delay = delay * 10 + (*p - '0');
	}

	async = coap_register_async(ctx,
		session,
		request,
		COAP_ASYNC_SEPARATE | COAP_ASYNC_CONFIRM,
		(void *)(COAP_TICKS_PER_SECOND * delay));
}

