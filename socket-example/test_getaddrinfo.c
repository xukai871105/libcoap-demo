#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>

#include <errno.h>
// #include <sys/types.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>


int main(int argc, char **argv)
{
	char *host = "www.baidu.com";
	char *port = "80";
	int socket_id = 0;

	struct addrinfo hint, *addr_list, *cur;
	char ipstr[16];   
	bzero(&hint, sizeof(hint));

	hint.ai_family = AF_INET; 			// AP_UNSPEC
	hint.ai_socktype = SOCK_DGRAM;		// SOCK_DGRAM SOCK_STREAM
	hint.ai_protocol = IPPROTO_UDP;

	int ret = getaddrinfo(host, port, &hint, &addr_list);
	if (ret != 0) {
		fprintf(stderr,"getaddrinfo error \n");
		exit(1);
	}

	for (cur = addr_list; cur != NULL; cur= cur->ai_next) {
		inet_ntop(AF_INET, &(((struct sockaddr_in *)(cur->ai_addr))->sin_addr),
		ipstr, 16);
		printf("%s\n", ipstr);

		socket_id = socket(cur->ai_family, cur->ai_socktype, cur->ai_protocol);
		printf("Socket: %d\n", socket_id);
	}

	freeaddrinfo(addr_list);

	close(socket_id);
	exit(0);
}