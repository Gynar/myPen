// 2017-11-24 created

#include <pthread.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdbool.h>
#include <strings.h>

#include "coap.h"

#define PORT 5683

typedef enum tag_comtype_t{
	SEARCH_PAIR,
	READY_SIGN,
	RECV_DATA
}comtype_t;

typedef struct tag_argSock_t{
	int m_fd;
	comtype_t m_cmd;
}argSock_t;

// thread global bar

// function for thead
void* c_thread(void* arg){
	int n, rc, fd;
	comtype_t cmd;

	struct sockaddr_in cliaddr;

	uint8_t buf[4096];
	uint8_t scratch_raw[4096];
	coap_rw_buffer_t scratch_buf = {scratch_raw, sizeof(scratch_raw)};
	socklen_t len;
	coap_packet_t pkt;
	
	fd = ((argSock_t*)arg)->m_fd;
	cmd = ((argSock_t*)arg)->m_cmd;

	len = sizeof(cliaddr);
	n = recvform(fd, buf, sizeof(buf), 0, (struct sockaddr*)&cliaddr, &len);
	rc = coap_parse(&pkt, buf, n);

	if(!rc)
		printf("Bad packet rc=%d\n", rc);
	else{
		size_t rsplen = sizeof(buf);
		coap_packet_t rsppkt;

		coap_handle_req(&scratch_buf, &pkt, &rsppkt);

		rc = coap_build(buf, &rsplen, &rsppkt);
		if(!rc)
			printf("coap_build failed rc=%d\n", rc);
		else{
			sendto(fd, buf, rsplen, 0, (struct sockaddr*)&cliaddr, sizeof(cliaddr));
		}
	}
}

int main(int argc, char **argv){
	// socket var
	int fd;
	struct sockaddr_in servaddr;
	// thread var
		




	// socket establishing
		// create socket
	fd = socket(AF_INET,SOCK_DGRAM,0);
	bzero(&servaddr,sizeof(servaddr));
		// socket config
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(PORT);
		// socket binding
	bind(fd,(struct sockaddr *)&servaddr, sizeof(servaddr));
		// endpoint setup
	endpoint_setup();
		
	// my program loop
	for(;;){
		
		// connection check
		while(1){
		}
		// create png file
		while(1){
		}
		// send communication ready sign
		while(1){
		}
		// active communication loop
		while(1){
		}
	}
}
