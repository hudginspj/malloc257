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

int shutdown_requested = 0;
int in_progress = 0;
int server;

void signal_handler(int no) {
  if (no == SIGINT) {
    shutdown_requested = 1;
    printf("\nShutting down\n");
    if (!in_progress) {
      close(server);
      //return(0);
      exit(0);
    }
  }
}


int term(char *str) {
  return !strncmp(str, TERMINAL_STR, sizeof(TERMINAL_STR));
}

int write_term(){
  char c; 
  if (term_i < 8) {
     c = TERMINAL_STR[term_i];
     term_i++;
     printf("write_term%c", c);
     return c;
  } else{
    return 0;
  }
}

int file_to_soc(int client, char *filename) {
    int i, reached_eof = 0, done = 0;
    char c;
    char buffer[BUFFER_SIZE];
    FILE *input = fopen( filename, "r");
    
    term_i = 0;
    
    while (!done) {
      
      for (i = 0; i<BUFFER_SIZE; i++){
        if (reached_eof) {
          c = write_term();
          if (c == 0) {
            done = 1;
            break;
          } else {
            buffer[i] = c;
          }
        } else {
          c = fgetc(input);
          
          if (c == EOF) {
            reached_eof = 1;
          } else {
            buffer[i] = c;
          }
        }
      }
      if (write( client, buffer, BUFFER_SIZE) != BUFFER_SIZE) {
            return( errno );
      }
      printf( "Sent a value of [%8s]\n", buffer );
    }
    //strcpy(buffer, TERMINAL_STR);
    //if (write( client, buffer, BUFFER_SIZE) != BUFFER_SIZE) {
    //        return( errno );
    //}


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
        in_progress = 1;
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
        //value++;
        //value = htonl(value);
        //write( client, response, BUFFER_SIZE);

        /*if (write( client, buffer, BUFFER_SIZE) != BUFFER_SIZE) {
        //if (errno) {
            printf( "Error writing network data [%s]\n", strerror(errno) );
            close(server);
            return( -1 );
        }
        printf( "Sent a value of [%8s]\n", buffer );*/
        buffer[BUFFER_SIZE-1] = '\0';
        if (file_to_soc(client, buffer)) {
            printf( "Error writing file to network [%s]\n", strerror(errno) );
            close(server);
            return( -1 );
        }

        close(client); // Close the socket
        in_progress = 0;
    }
    close(server);
    return ( 0 );
}

int main (int argc, char **argv)
{
  strcpy(TERMINAL_STR,"Xmsc257");
  TERMINAL_STR[0] = 'c';
  puts(TERMINAL_STR);
  return server_operation();
}

