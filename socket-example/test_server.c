#include <sys/types.h> 
#include <sys/socket.h> 
#include <unistd.h> 
#include <netinet/in.h> 
#include <arpa/inet.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <errno.h> 
#include <netdb.h> 
#include <stdarg.h> 
#include <string.h> 

#define BUFFER_SIZE 128

int main() 
{
    char *bind_ip = NULL;
    char *port = "38000";
    char ipstr[32];
    // int n, ret;
    struct addrinfo hints, *addr_list, *cur;
    int socket_fd;

    memset( &hints, 0, sizeof( hints ) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;
    if (bind_ip == NULL)
        hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(bind_ip, port, &hints, &addr_list ) != 0 ) {
        // return( MBEDTLS_ERR_NET_UNKNOWN_HOST );
        fprintf(stderr, "getaddrinfo error \n");
        exit(1);
    }

    for (cur = addr_list; cur != NULL; cur = cur->ai_next)
    {
        socket_fd = socket(cur->ai_family, cur->ai_socktype, cur->ai_protocol);
        if (socket_fd  < 0)
        {
            fprintf(stderr, "socket error \n");
            continue;
        }

        printf("Socket :%d\n", socket_fd);
        inet_ntop(AF_INET, &(((struct sockaddr_in *)(cur->ai_addr))->sin_addr),
            ipstr, 16);

        printf("IP Address: %s\n", ipstr);
        if (bind(socket_fd, cur->ai_addr, cur->ai_addrlen) != 0)
        {
            close(socket_fd);
            fprintf(stderr, "bind error \n");
            exit(1);;
        }
        break;
    }

    freeaddrinfo(addr_list);

    printf("Server Listen： %d\n", atoi(port));
    while(1) 
    {  
        struct sockaddr_in client_addr; 
        socklen_t client_addr_length = sizeof(client_addr); 

        char buffer[BUFFER_SIZE]; 
        memset(buffer, 0x00, BUFFER_SIZE); 

        if (recvfrom(socket_fd, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&client_addr, &client_addr_length) == -1) 
        { 
            perror("Receive Data Failed！"); 
            exit(1); 
        } 

        printf("(%s, %d) Said: ", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        sendto(socket_fd, buffer, strlen(buffer), 0,
                (struct sockaddr *)&client_addr, sizeof(struct sockaddr));

        printf("%s\n", buffer); 
    }

    return 0;
}

#if 0
  
#define SERVER_PORT             38000 
#define BUFFER_SIZE             1024 
  
int main() 
{ 

    struct sockaddr_in server_addr; 
    bzero(&server_addr, sizeof(server_addr)); 
    server_addr.sin_family = AF_INET; 
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
    server_addr.sin_port = htons(SERVER_PORT); 

    int server_socket_fd = socket(AF_INET, SOCK_DGRAM, 0); 
    if (server_socket_fd == -1) 
    { 
        perror("Create Socket Failed！"); 
        exit(1); 
    } 

    if (-1 == (bind(server_socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)))) 
    { 
        perror("Server Bind Failed！"); 
        exit(1); 
    } 

    printf("Server Listen： %d\n", SERVER_PORT);
    while(1) 
    {  
        struct sockaddr_in client_addr; 
        socklen_t client_addr_length = sizeof(client_addr); 

        char buffer[BUFFER_SIZE]; 
        bzero(buffer, BUFFER_SIZE); 

        if (recvfrom(server_socket_fd, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&client_addr, &client_addr_length) == -1) 
        { 
            perror("Receive Data Failed！"); 
            exit(1); 
        } 

        printf("(%s , %d) Said : ", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        printf("%s\n", buffer); 
    } 

    close(server_socket_fd); 
    return 0; 
} 

#endif