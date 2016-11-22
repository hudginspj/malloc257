#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <errno.h>



int server_operation( void ) {  
  int server, client; 
  uint32_t inet_len; 
  char value[16];
  char *response = "Bugger off!";
  struct sockaddr_in saddr, caddr; 
    saddr.sin_family = AF_INET; 
    saddr.sin_port = htons(16453); 
    saddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    server = socket(PF_INET, SOCK_STREAM, 0);  
    
    if (setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) < 0)
      puts("setsockopt(SO_REUSEADDR) failed");

    if (server == -1) { 
        printf( "Error on socket creation [%s]\n", strerror(errno) ); 
        return( -1 ); 
    } 
    if ( bind(server, (struct sockaddr *)&saddr, sizeof(saddr)) == -1 ) { 
        printf( "Error on socket bind [%s]\n", strerror(errno) ); 
        return( -1 ); 
    } 
    if ( listen( server, 5 ) == -1 ) { 
        printf( "Error on socket listen [%s]\n", strerror(errno) ); 
        return( -1 ); 
    } 

    while ( 1 ) {
        inet_len = sizeof(caddr);
        if ( (client = accept( server, (struct sockaddr *)&caddr, &inet_len )) == -1 ) {
            printf( "Error on client accept [%s]\n", strerror(errno) );
            close(server);
            return( -1 );
        }
        printf( "Server new client connection [%s/%d]", inet_ntoa(caddr.sin_addr), caddr.sin_port );
        read( client, value, 16);
        //if (0&& read( client, value, 2) != sizeof(value) ) {
        if (errno) {
            printf( "Error writing network data [%s]\n", strerror(errno) );
            close(server);
            return( -1 );
        }
        //value = ntohl(value);
        printf( "Receivd a value of [%s]\n", value );
        //value++;
        //value = htonl(value);
        write( client, response, 16);
        //if (write( client, response, sizeof(response)) != sizeof(response) ) {
        if (errno) {
            printf( "Error writing network data [%s]\n", strerror(errno) );
            close(server);
            return( -1 );
        }
        printf( "Sent a value of [%s]\n", response );
        close(client); // Close the socket
    }
    return ( 0 );
}

int main (int argc, char **argv)
{
  return server_operation();
}

