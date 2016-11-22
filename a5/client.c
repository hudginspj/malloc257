#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <errno.h>



int client_operation( void ) {

  int socket_fd;
  //uint32_t value;
  char *value = "Hi!";
  char response[16];
  struct sockaddr_in caddr;
  char *ip = "127.0.0.1";

    caddr.sin_family = AF_INET; 
    caddr.sin_port = htons(16453); 
    if ( inet_aton(ip, &caddr.sin_addr) == 0 ) {  
      return( -1 ); 
    }    
    socket_fd = socket(PF_INET, SOCK_STREAM, 0);  
    if (socket_fd == -1) { 
        printf( "Error on socket creation [%s]\n", strerror(errno) ); 
    return( -1 ); 
    }    
    if ( connect(socket_fd, (const struct sockaddr *)&caddr, sizeof(caddr)) == -1 ) {  
        printf( "Error on socket connect [%s]\n", strerror(errno) ); 
    return( -1 ); 
    }    
    //value = htonl( 1 ); 
    write( socket_fd, value, 16);
    //if (0&& write( socket_fd, value, 2) != sizeof(value) ) {  
    if (errno){
        printf( "Error writing network data [%s]\n", strerror(errno) ); 
        return( -1 ); 
    }    
    printf( "Sent a value of [%s]\n", value ); 
    read( socket_fd, response, 16);
    //if (0&& read( socket_fd, response, 16) != sizeof(response) ) {  
    if (errno) {
        printf( "Error reading network data [%s]\n", strerror(errno) ); 
        return( -1 ); 
    }    
    //value = ntohl(value); 
    printf( "Receivd a value of [%s]\n", response ); 
    close(socket_fd); // Close the socket 
    return( 0 ); 

}



int main (int argc, char **argv)
{
  return client_operation(); 
}

