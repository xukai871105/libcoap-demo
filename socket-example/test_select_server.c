#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>

#define BUFFER_SIZE 128

int main()
{
    int server_socket_fd = 0;
    struct sockaddr_in server_addr;
    int ret = 0;


    struct timeval tv;
    int maxfd = 0;
    fd_set read_fds;

    server_socket_fd = socket(AF_INET, SOCK_DGRAM, 0);

    memset(&server_addr,0,sizeof(struct sockaddr_in));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(38000);


    ret = bind(server_socket_fd,(struct sockaddr*)&server_addr, sizeof(server_addr));
    if(-1 == ret)
    {
        perror("bind failed :");
        close(server_socket_fd);
        return -1;
    }

    FD_ZERO(&read_fds);
    FD_SET(server_socket_fd,&read_fds);
    tv.tv_sec = 10;
    tv.tv_usec = 0;
    maxfd = server_socket_fd;

    while(1)
    {
        FD_ZERO(&read_fds);
        FD_SET(server_socket_fd,&read_fds);
        tv.tv_sec = 10;
        tv.tv_usec = 0;
        // maxfd = server_socket_fd;

        ret = select(maxfd+1, &read_fds, NULL, NULL, &tv);

        if (ret < 0) 
        {
            perror("select");
        }

        if (ret == 0)
        {
            continue;
        }       

        if (FD_ISSET(server_socket_fd, &read_fds))
        {
            struct sockaddr_in client_addr; 
            socklen_t client_addr_length = sizeof(client_addr); 

            char buffer[BUFFER_SIZE]; 
            bzero(buffer, BUFFER_SIZE); 

            if (recvfrom(server_socket_fd, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&client_addr, &client_addr_length) == -1) 
            { 
                perror("Receive Data Failed:"); 
                exit(1); 
            }

            printf("(%s , %d) said : ", inet_ntoa(client_addr.sin_addr),
               ntohs(client_addr.sin_port));

            printf("%s\n", buffer); 

            sendto(server_socket_fd, buffer, strlen(buffer), 0,
                    (struct sockaddr *)&client_addr, sizeof(struct sockaddr));
        }
    }
    close(server_socket_fd);

    return 0;
}