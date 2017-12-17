
// 2017-11-24 created

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>

#include "coap_impl.h"
#include "gpng_impl.h"

#define FR_THRESHOLD 300

typedef struct{
	int x;
	int y;
	int pushed;
}point_t;

int quit;
int quit_server;
int timeout;

void*
	coap_server_service(void* arg);

void*
coap_server_service(void* arg) {
	
	// optional
		// for loopback test
	char *group = NULL;
	char* addr_arg = "127.0.0.1";
	char* port_arg = "6";

	// required
	coap_context_t* ctx;
	coap_tick_t now;
	unsigned wait_ms;

	coap_log_t log_level = LOG_WARNING;
	clock_offset = time(NULL):

	ctx = libcoap_open();

	wait_ms = COAP_RESOURCE_CHECK_TIME * 1000;

	while (!quit_server) {
		int result = coap_run_once(ctx, wait_ms);
		if (result < 0) {
			break;
		}
		else if ((unsigned)result < wait_ms) {
			wait_ms -= result;
		}
		else {
			if (time_resource) {
				coap_resource_set_dirty(time_resource, NULL);
			}
			wait_ms = COAP_RESOURCE_CHECK_TIME * 1000;
		}

		/* check if we have to send asynchronous responses */
		coap_ticks(&now);
		check_async(ctx, now);
		/* check if we have to send observe notifications */
		coap_check_notify(ctx);
	}

	coap_free_context(ctx);
	coap_cleanup();

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
	unsigned int used;
	unsigned int cap;
	point_t* pmem;

	while (!quit){
		// init mem
		used = 1;
		cap = 512;
		pmem = (point_t*)malloc(sizeof(point_t)*cap);

		// server thread start
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

				if(recv_dat.Fr > FR_THRESHOLD) // #deifne THRESHOLD
					pmem[used].pushed = 1;
				else
					pmem[used].pushed = 0;

				pmem[used].x = nx;
				pmem[used++].y = ny;

				if(used >= cap){ // realloc
					cap *= 2;
					pmem = (point_t*)realloc(pmem, sizeof(point_t)*cap);
				}
			}
		}

		// close server by quit_server variable
		printf("timeout! server will be closed.\n");

		quit_server = 1;

		printf("waiting for returning sub thread... ");

		pthread_join(coap_server_thr, (void**)&thr_st);
		
		printf("thread terminated!\n");

		// file config
		printf("preparing png file generation...\n");
		printf("\tpmem used : %6d\n", used);

		tpImage my_surf;
		point_t minPt = {0, 0, 0};
		point_t maxPt = {0, 0, 0};;
		point_t *ptar_Pt, *ptar_PtN;
		unsigned int n;
			// find minPt, maxPt
		ptar_Pt = pmem;
		for(n = 0; n < used; n++){
			if(ptar_Pt->x < minPt.x)
				minPt.x = ptar_Pt->x;
			else if(ptar_Pt->x > maxPt.x)
				maxPt.x = ptar_Pt->x;

			if(ptar_Pt->y < minPt.y)
				minPt.y = ptar_Pt->y;
			else if(ptar_Pt->y > maxPt.y)
				maxPt.y = ptar_Pt->y;

			ptar_Pt++;
		}
		
		printf("[pmem XY Info] max & min\n");
		printf("\tmax X / Y = %5d / %5d\n", maxPt.x, maxPt.y);
		printf("\tmin X / Y = %5d / %5d\n", minPt.x, minPt.y);

			// calc deltaX, deltaY for image Height and Width
		int panelX_len;
		int panelY_len;
		
		panelX_len = maxPt.x - minPt.x;
		panelY_len = maxPt.y - minPt.y;
		
		printf("[pmem XY Info] calculated XY len\n");
		printf("\tX_len( width  ) = %6d\n", panelX_len);
		printf("\tY_len( height ) = %6d\n", panelY_len);

			// if it is too large?

			// config Height and Width, create plane
		my_surf.w = panelX_len;
		my_surf.h = panelY_len;
		my_surf.pix = (tpPixel*)malloc(sizeof(tpPixel)*(my_surf.w)*(my_surf.h));

		printf("...\n");
		printf("Plane memory allocated!\n");

		// draw by pmem
			// offset to always have possitive x, y
		printf("drawing process started...\n");

		if(minPt.x < 0){
			printf("\tapplying x offset, offset_x = +%5d...", -minPt.x);
			ptar_Pt = pmem;
			for(n = 0; n < used; n++){
				ptar_Pt->x = ptar_Pt->x - minPt.x;
				ptar_Pt++;
			}
			printf("applied\n");
		}
		if(minPt.y < 0){
			printf("\tapplying y offset, offset_y = +%5d...", -minPt.y);
			ptar_Pt = pmem;
			for(n = 0; n < used; n++){
				ptar_Pt->y = ptar_Pt->y - minPt.y;
				ptar_Pt++;
			}
			printf("applied\n");
		}
			
			// draw line if pushed
		printf("\tscanning pmem & connecting dots... ");
		printf("  0%%");

		int j; float per; float f_cmp = 2.0f;

		ptar_Pt = pmem;
		ptar_PtN = pmem+1;
		for(n = 1; n < used; n++){
			if(ptar_Pt->pushed){
				tXY tA = {ptar_Pt->x, ptar_Pt->y};
				tXY tB = {ptar_PtN->x, ptar_PtN->y};
				drawLine(&my_surf, tA, tB);
			}
			ptar_Pt = ptar_PtN++;
			
				// displaying processing percentage
			per = n / used * 100;
			if(per >= f_cmp){
				for(j = 0; j < 4; j++)
					printf("\b");
				printf("%3d%%", (int)per);

				f_cmp += 2.0f;
			}
		}
		
		printf(" done!\n");

		// file creation
		printf("Save as png...");
			// get file name
		char* my_instance;
		char my_fname[512] = {0};
		my_instance = find_newName();
		if(my_instance == NULL){
			printf("Warning: Naming Failed... we forced to name \"%s\"\n", FILE_NAME);
			my_instance = (char*)malloc(sizeof(char)*DEFAULT_LEN);
			memset(my_instance, 0, DEFAULT_LEN);
			snprintf(my_instance, DEFAULT_LEN, "%s", FILE_NAME);
		}

		snprintf(my_fname, 512, "%s/%s", STORAGE_DIR, my_instance);
		
		if(tpSavePNG(my_fname, &my_surf) < 0)
			printf("Critical Error Occurred!, failed to Save as\n\t%s\n", my_fname);
		else
			printf("Success to save file.\n");

		// freeing dynamic memories
		printf("freeing pmem...");
		free(pmem);
		printf("done\nfreeing plane mem...");
		free(my_surf.pix);
		printf("done\n");

		// cloud upload
		printf("preparing upload to google cloud... \n");

	}


	return 0;
}