#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <errno.h>

#define BUFFER_SIZE 50
#define PORT 2147
#define TERMINAL_STR "cmsc257"

int term(char *str) {
  return !strncmp(str, TERMINAL_STR, sizeof(TERMINAL_STR));
}


int soc_to_file(int server, char *filename) {
    int i;
    char c;
    char buffer[BUFFER_SIZE];
    FILE *output = fopen( filename, "w+");
    
    
    while (1) {
      if (read( server, buffer, BUFFER_SIZE) != BUFFER_SIZE ) {  
        return( -1 ); 
      }
      if (term(buffer)) {
        break;
      } 
      //printf( "Received a value of [%8s]\n", buffer ); 
      
      for (i = 0; i<BUFFER_SIZE; i++){
        c = buffer[i];
        fputc(c, output);
        if (c == EOF) {
          //done = 1;
          break;
        }
      }
    }

    fclose(output);
    return(0);
}

int client_operation( char *filename ) {

  int socket_fd;
  //uint32_t value;
  //char *value = "Hi!";
  char buffer[BUFFER_SIZE];
  struct sockaddr_in caddr;
  char *ip = "127.0.0.1";

    caddr.sin_family = AF_INET; 
    caddr.sin_port = htons(PORT); 
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
    //write( socket_fd, value, BUFFER_SIZE);
    strcpy(buffer, filename);
    if (write( socket_fd, buffer, BUFFER_SIZE) != BUFFER_SIZE ) {  
    //if (errno){
        printf( "Error writing network data [%s]\n", strerror(errno) ); 
        return( -1 ); 
    }    
    printf( "Sent a value of [%8s]\n", buffer ); 
    //read( socket_fd, response, BUFFER_SIZE);
    /*if (read( socket_fd, buffer, BUFFER_SIZE) != BUFFER_SIZE ) {  
    //if (errno) {
        printf( "Error reading network data [%s]\n", strerror(errno) ); 
        return( -1 ); 
    }    
    //value = ntohl(value); 
    printf( "Receivd a value of [%8s]\n", buffer ); */
    
    strcpy(buffer, filename);
    strncpy(buffer, "X", 1);
    if (soc_to_file(socket_fd, buffer)) {  
        printf( "Error reading network data [%s]\n", strerror(errno) ); 
        return( -1 ); 
    }    


    close(socket_fd); // Close the socket 
    return( 0 ); 

}



int main (int argc, char **argv)
{
  if (argc <2) {
    puts("Please type a filename");
    return (0);
  }
  return client_operation(argv[1]); 
}

