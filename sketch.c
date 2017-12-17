
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

typedef struct{
	int x;
	int y;
	bool pushed;
}point_t;

typedef struct{
	int Ax;
	int Ay;
	int Az;
	int Gx;
	int Gy;
	int Gz;
	int Fr;
	bool renewed;
}recv_dat_t;

int quit;
int quit_server;
int timeout;
bool con_established = 0;
recv_dat_t recv_dat;

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
	int used;
	int cap;
	point_t* pmem;
	// init
	used = 1;
	cap = 512;
	pmem = (point_t*)malloc(sizeof(point_t)*cap);
	memset(pmem, 0, sizeof(point_t)*cap);

	pmem[0].x = 0; pmem[0].y = 0;, pmem[0].pushed = 0;


	while (!quit){
		thr_id = pthread_create(&coap_server_thr, NULL, //&p_attr_detach_mode,
			coap_server_service, NULL);

		// wait connection
		while(!con_established);
		// connected, wait data
		while(!timeout){ // @@ kernel timer
			if(recv_dat.renewed){
				int nx, ny;
				// @@ calc new point
				// nx = ?
				// ny = ?

				// add new point

				if(recv_dat.Fr > THRESHOLD) // #deifne THRESHOLD
					pmiem[used].pushed = 1;
				else
					pmem[used].pushed = 0;

				pmem[used].x = nx;
				pmem[used++].y = ny;

				if(used >= cap)
					// realloc
			}
		}

		// close server by quit_server variable
		quit_server = 1;
		pthread_join(coap_server_thr, (void**)&thr_st);
		
		// create file

		// draw by pmem

		// cloud upload

	}

	return 0;
}
