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

int main(int argc, char **argv)
{
	int fd;
	struct sockaddr_in servaddr, cliaddr;
	uint8_t buf[4096];
	uint8_t scratch_raw[4096];
	coap_rw_buffer_t scratch_buf = {scratch_raw, sizeof(scratch_raw)};
	// create socket
	fd = socket(AF_INET,SOCK_DGRAM,0);
	
	bzero(&servaddr,sizeof(servaddr));
	
	// socket config
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(PORT);
	
	// socket binding
	bind(fd,(struct sockaddr *)&servaddr, sizeof(servaddr));
	
	endpoint_setup();

	while(1)
	{
		int n, rc;
		socklen_t len = sizeof(cliaddr);
		coap_packet_t pkt;

		n = recvfrom(fd, buf, sizeof(buf), 0, (struct sockaddr *)&cliaddr, &len);
	
		printf("Received: ");
		coap_dump(buf, n, true);
		printf("\n");

		if (0 != (rc = coap_parse(&pkt, buf, n)))
			printf("Bad packet rc=%d\n", rc);
	        else
        	{
			size_t rsplen = sizeof(buf);
			coap_packet_t rsppkt;

            coap_dumpPacket(&pkt);

            coap_handle_req(&scratch_buf, &pkt, &rsppkt);

            if (0 != (rc = coap_build(buf, &rsplen, &rsppkt)))
                printf("coap_build failed rc=%d\n", rc);
            else
            {

                printf("Sending: ");
                coap_dump(buf, rsplen, true);
                printf("\n");

                coap_dumpPacket(&rsppkt);

                sendto(fd, buf, rsplen, 0, (struct sockaddr *)&cliaddr, sizeof(cliaddr));
            }
        }
    }
}

