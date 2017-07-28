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