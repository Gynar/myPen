// 2017-11-24 created

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <signal.h>

#include <pthread.h>
#include <unistd.h>

#include "my_libcoap.h"

#define SERVICE_UP		1
#define SERVICE_SUSPEND	0
#define SERVICE_DOWN	-1

typedef enum pg_state_t {
	SRCH_PAIR,
	PREP_PANEL,
	WAIT_SIGN,
	ACTIVE_IDLE,
}pg_state_t;

static int quit = 0;

static void handle_sigint(int signum UNUSED_PARAM) {
	quit = 1;
}

// need mutex?
int server_state = SERVICE_DOWN;

void* coap_server_service(void* arg_t) {
	coap_context_t* ctx;
	unsigned wait_ms;

	ctx = libcoap_open();

	wait_ms = COAP_RESOURCE_CHECK_TIME * 1000;

	while (!quit) {
		int result = coap_run_once(ctx, wait_ms);
		if (result < 0) {
			break;
		}
		else if ((unsigned)result < wait_ms) {
			wait_ms -= result;
			server_state = SERVICE_SUSPEND;
		}
		else {
			if (time_resource) {
				coap_resource_set_dirty(time_resource, NULL);
				server_state = SERVICE_UP;
			}
			wait_ms = COAP_RESOURCE_CHECK_TIME * 1000;
		}

		/* check if we have to send asynchronous responses */
		coap_ticks(&now);
		check_async(ctx, now);
		/* check if we have to send observe notifications */
		coap_check_notify(ctx);
	}

	libcoap_close(ctx);
	server_state = SERVICE_DOWN;

	return (void*)0;
}

int main(int argc, char **argv){
	// thread var
	pthread_t coap_server_thr;
	//pthread_attr_t p_attr_detach_mode;
	int thr_id;
	int thr_st;	

	// thread attr set : detach attr
	//pthread_attr_init(&p_attr_detach_mode);
	//pthread_attr_setdetachstate(&p_attr_detach_mode, PTHREAD_CREATE_DETACHED);
	
	// usr program var
	pg_state_t pg_state;

	// init
	pg_state = SRCH_PAIR;

	signal(SIGINT, handle_sigint);

	// my program loop
	while (!quit) {
		// starting service
		thr_id = pthread_create(&coap_server_thr, NULL, //&p_attr_detach_mode,
			coap_server_service, NULL);
		if (thr_id < 0) {
			perror("coap service thread creation fail...");
			exit(1);
		}

		// service state machine
		while (!quit) {
			// connection check
			switch (pg_state) {
			case SRCH_PAIR:
			
				break;
			case PREP_PANEL: // png create
				
				break;
			case WAIT_SIGN:

				break;
			case ACTIVE_IDLE:

				break;
			default:

				break;
			}
		}
		// wait for closing service
		pthread_join(coap_server_thr, (void**)&thr_st);
	}

	return 0;
}
