
// 2017-11-24 created

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>
#include <math.h>
#include "coap_impl.h"
#include "gpng_impl.h"

#define DEV_KTIMER "/dev/my_ktimer"
#define DELTA 1
#define FR_THRESHOLD 300
#define PI 3.14159265358979323846f

typedef struct{
	int x;
	int y;
	int pushed;
}point_t;

// externs?
recv_dat_t recv_dat;
int con_established = 0;
struct coap_resource_t *time_resource = NULL;
time_t clock_offset;
time_t my_clock_base = 0;
coap_async_state_t *async = NULL;
int my_sensor_base = 0;

// 
int quit = 0;
int quit_server = 0;
int timeout;

void
	quit_signal(int sig);
void*
	coap_server_service(void* arg);

void
quit_signal(int sig){
	quit = 1;
}
void*
coap_server_service(void* arg) {
	printf("thread now on working\n");	
	// optional
		// for loopback test
	char *group = NULL;
	char* addr_arg = "127.0.0.1";
	char* log_arg = "6";

	// required
	char addr_str[NI_MAXHOST] = "::";
	char port_str[NI_MAXSERV] = "5683";
	coap_context_t* ctx;
	coap_tick_t now;
	unsigned wait_ms;

	// erase if you don't want lookback
	//strncpy(addr_str, addr_arg, NI_MAXHOST - 1);
	//addr_str[NI_MAXHOST - 1] = '\0';
	//

	coap_log_t log_level = LOG_WARNING;
	// erase if you want default log level
	log_level = strtol(log_arg, NULL, 10);

	clock_offset = time(NULL);

	coap_startup();
	coap_dtls_set_log_level(log_level);
	coap_set_log_level(log_level);

	ctx = get_context(addr_str, port_str);
	if (!ctx)
		return (void*)-1;

	fill_keystore(ctx);
	init_resources(ctx);

	/* join multicast group if requested at command line */
	if (group)
		join(ctx, group);

	wait_ms = COAP_RESOURCE_CHECK_TIME * 1000;
	printf("coap_server ready to run!\n");
	while (!quit_server) {
		printf(".");
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
	printf("\ncoap server closing!\n");

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
	
	// ktimer
	int fd_ktimer;

	// usr program var
	unsigned int used;
	unsigned int cap;
	point_t* pmem;
	
	//fd_ktimer = open(DEV_KTIMER, O_RDWR);
	//if (fd_timer == NULL) {
	//	printf("timer device open failed!\n");
	//	printf("\t[tip] checkout whether ktimer module is loaded.\n");
	//	return 0;
	//}
	printf("program will be started!\n");
	//signal(SIGINT, quit_signal);
	while (!quit){
		// init mem
		used = 1;
		cap = 512;
		pmem = (point_t*)malloc(sizeof(point_t)*cap);
		printf("pmem allocated!\n");
		// server thread start
		thr_id = pthread_create(&coap_server_thr, NULL, //&p_attr_detach_mode,
			coap_server_service, NULL);
		

		if(thr_id < 0){
			printf("thread creation failed!\n");
			exit(0);
		}
		else
			printf("thread creation succeeded!\n");

		double roll = 0, pitch = 0, yaw = 0;
		double v0 = 0;
		int i_cnt = 500;
		timeout = 512;
		// wait connection
		while(!con_established);
			printf("@");
		// connected, wait data
		while(timeout){ // @@ kernel timer
			if(recv_dat.renewed){
				timeout--;
				if(timeout <= i_cnt){
					printf("timeout count : %d \n", timeout);
					i_cnt -= 100;
				}
				recv_dat.renewed = 0;

				// @@ calc new point
				// nx = ?
				// ny = ?
				int nx = pmem[used].x, ny = pmem[used].y;
				double gyroscope_sensitivity = 250/32768;
				double aroll = 0, apitch = 0, ayaw = 0;
				double forceMagnitudeApprox = abs(recv_dat.Ax) + abs(recv_dat.Ay) + abs(recv_dat.Az);
				double v1;
				double v01;
				double Acc;
				double length;

				pitch += (recv_dat.Gx/gyroscope_sensitivity)*DELTA;
				roll -= (recv_dat.Gy/gyroscope_sensitivity)*DELTA;
				yaw -= (recv_dat.Gz/gyroscope_sensitivity)*DELTA;
				if(forceMagnitudeApprox > 8192 && forceMagnitudeApprox < 32768)
				{
					apitch = atan2(recv_dat.Ay, recv_dat.Az) * 180/PI;
					pitch = pitch * 0.98 + apitch * 0.5 * DELTA;
					aroll = atan2(recv_dat.Ax, recv_dat.Az)* 180/PI;
					roll = roll * 0.98 + aroll * 0.5*DELTA;
					ayaw = atan2(recv_dat.Ax, recv_dat.Ay) * 180/PI;
					yaw = yaw * 0.98 + ayaw * 0.5 * DELTA;
				}
				Acc = sqrt(recv_dat.Ax*recv_dat.Ax + recv_dat.Ay*recv_dat.Ay);
				length = sqrt(nx * nx + ny * ny);
				v1 = v0 + Acc*DELTA;
				v01 = (v0 + v1)/2;

				length = length + v01*(DELTA);
				nx += (int)(length * cos(yaw));
				ny += (int)(length * sin(yaw));

				// add new point

				if(recv_dat.Fr > FR_THRESHOLD) // #deifne THRESHOLD
					pmem[used].pushed = 1;
				else
					pmem[used].pushed = 0;

				pmem[used].x = nx;
				pmem[used++].y = ny;
				
				printf("[received vars]\n");
				printf("\tAx = %3.3f / Ay = %3.3f / Az = %3.3f\n", recv_dat.Ax, recv_dat.Ay, recv_dat.Az);
				printf("\tGx = %3.3f / Gy = %3.3f / Gz = %3.3f\n", recv_dat.Gx, recv_dat.Gy, recv_dat.Gz);
				printf("\tFr = %5d \n", recv_dat.Fr);
				printf("[calculated vars]\n");
				printf("\tforceMagnitudeApprox = %3.3f\n", forceMagnitudeApprox);
				printf("\taroll = %3.3f / apitch = %3.3f / ayaw = %3.3f\n", aroll, apitch, ayaw);
				printf("\troll = %3.3f / pitch = %3.3f / yaw = %3.3f\n", roll, pitch, yaw);
				printf("\tv0 = %3.3f / v01 = %3.3f\n", v0, v01);
				printf("\tAcc = %3.3f / length = %3.3f\n", Acc, length);
				printf("\t(X,Y) : ( %5d, %5d )\n", nx, ny);

				if(used >= cap){ // realloc
					cap *= 2;
					pmem = (point_t*)realloc(pmem, sizeof(point_t)*cap);
				}
				v0 = v1;
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

		// @@@@@@@@@@@@@@@@@@@@@
		quit = 1;
	}


	return 0;
}
