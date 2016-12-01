#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <errno.h>

#define BUFFER_SIZE 50
#define PORT 2147

char TERMINAL_STR[8];
int term_i = 0;
char queue[10];
int queue_i = 0;
int queue_size = 0;

void putc_queue(char c, FILE *fp) {
   queue[queue_i] = c;
   if (queue_i == 9) {
      queue_i = 0;
   } else {
     queue_i++;
   }

   if (queue_size < 10) {
      queue_size++;
   } else {
     fputc(queue[queue_i], fp);
   }
}


//int term(char *str) {
//  return !strncmp(str, TERMINAL_STR, sizeof(TERMINAL_STR));
//}

int term(char c) {
  if (term_i > 2) printf("term%c%d\n", c,term_i);

  if (c == TERMINAL_STR[term_i]) {
    puts("match");
    if (term_i == 6) {
      puts("final match");
      return(1);
    }
    term_i++;
  } else {
    term_i = 0;
  }
  return(0);
}


int soc_to_file(int server, char *filename) {
    int i, done = 0;
    char c;
    char buffer[BUFFER_SIZE];
    FILE *output = fopen( filename, "w+");
    
    
    while (!done) {
      if (read( server, buffer, BUFFER_SIZE) != BUFFER_SIZE ) {  
        return( -1 ); 
      }
      //if (term(buffer)) {
      //  break;
      //} 
      //printf( "Received a value of [%8s]\n", buffer ); 
      
      for (i = 0; i<BUFFER_SIZE; i++){
        c = buffer[i];
        
        if (term(c)) {
          done = 1;
          puts("term found");
          break;
        }
        putc_queue(c, output);
      }
    }
    puts("loop exited");

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
  strcpy(TERMINAL_STR,"cmsc25X");
  TERMINAL_STR[6] = '7';
  puts(TERMINAL_STR);
  if (argc <2) {
    puts("Please type a filename");
    return (0);
  }
  return client_operation(argv[1]); 
}

