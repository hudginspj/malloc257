#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <errno.h>

#define BUFFER_SIZE 50
#define PORT 2147



int shutdown_requested = 0;
//int in_progress = 0;
int children = 0;
int server;
int pid =1;

char TERMINAL_STR[8];
int term_i = 0;
char queue[9];
int queue_i = 0;
int queue_size = 0;

void putc_queue(char c, FILE *fp) {
   queue[queue_i] = c;
   if (queue_i == 8) {
      queue_i = 0;
   } else {
     queue_i++;
   }

   if (queue_size < 9) {
      queue_size++;
   } else {
     fputc(queue[queue_i], fp);
   }
}



int term(char c) {
  if (c == TERMINAL_STR[term_i]) {
    if (term_i == 7) return 1;
    term_i++;
  } else {
    term_i = 0;
  }
  return 0;
}


void signal_handler(int no) {
  //puts("signal caught");
  //printf("Pid %d\n", pid);
  if (pid > 0) {
    shutdown_requested = 1;
    printf("\nShutting down\n");
    for (;children >0; children--) {
      //printf("Waiting for child %d\n", children);
      wait(NULL);
    }
    //puts("Children dead");
    close(server);
    exit(0);
  }
}


int file_to_soc(int client, char *filename) {
    int i, reached_eof = 0;
    char buffer[BUFFER_SIZE];
    FILE *input = fopen( filename, "r");
    

    //Stop after eof is reached
    while (!reached_eof) {
      //fill buffer
      //usleep(80000);
      for (i = 0; i<BUFFER_SIZE; i++){
        
        if (EOF == (buffer[i] = fgetc(input))) {
          reached_eof = 1;
          break;
        }
      }
      //write buffer to socket
      if (write( client, buffer, BUFFER_SIZE) != BUFFER_SIZE) {
            return( errno );
      }
      //printf( "Sent a value of [%8s]\n", buffer );
    }



    strcpy(buffer, TERMINAL_STR);
    if (write( client, buffer, BUFFER_SIZE) != BUFFER_SIZE) {
        return( errno );
        //raise(SIGINT);
    }


    fclose(input);
    return(0);
}


int server_operation( void ) {  
  int server, client; 
  
  uint32_t inet_len; 
  //char value[16];
  //char *response = "Bugger off!";
  char buffer[BUFFER_SIZE];
  struct sockaddr_in saddr, caddr; 
    saddr.sin_family = AF_INET; 
    saddr.sin_port = htons(PORT); 
    saddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    server = socket(PF_INET, SOCK_STREAM, 0);  

    signal(SIGINT, signal_handler);
    
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

    while ( !shutdown_requested ) {
      inet_len = sizeof(caddr);
      if ( (client = accept( server, (struct sockaddr *)&caddr, &inet_len )) == -1 ) {
          printf( "Error on client accept [%s]\n", strerror(errno) );
          close(server);
          return( -1 );
      }  
      children++;
      //pid = fork();
      //if (pid == 0) {
      if (!fork()) {
        printf( "Server new client connection [%s/%d]", inet_ntoa(caddr.sin_addr), caddr.sin_port );
        //read( client, value, BUFFER_SIZE);
        if (read( client, buffer, BUFFER_SIZE) != BUFFER_SIZE ) {
        //if (errno) {
            printf( "Error writing network data [%s]\n", strerror(errno) );
            close(server);
            return( -1 );
        }
        //value = ntohl(value);
        printf( "Received a value of [%8s]\n", buffer );

        buffer[BUFFER_SIZE-1] = '\0';
        if (file_to_soc(client, buffer)) {
            printf( "Error writing file to network [%s]\n", strerror(errno) );
            close(server);
            return( -1 );
        }

        close(client); // Close the socket
        exit(0);
        //in_progress = 0;
      } else if (pid < 0) {
        shutdown_requested = 1;
      }
    }


    for (;children >0; children--) wait(NULL);
    close(server);
    return ( 0 );
}

int main (int argc, char **argv)
{
  strcpy(TERMINAL_STR,"cmsc25X");
  TERMINAL_STR[6] = '7';
  return server_operation();
}

