#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <sys/wait.h>

char *prompt = "257sh";

void signal_handler(int no) {
  printf("Signal handler reccieved [%d]\n", no);
}


void execute(int num_tokens, char **tokens) {
  char *command;
  char dir[150];
  int i;

  if (!num_tokens) return;

  command = tokens[0];
  if (strcmp(command, "exit") == 0) {
    exit(EXIT_SUCCESS);
  } else if (strcmp(command, "pid") == 0) {
    //pid_t pid = ;
    printf("%d\n", getpid());
  } else if (strcmp(command, "ppid") == 0) {
    //pid_t ppid = ;
    printf("%d\n", getppid());
  } else if (strcmp(command, "cd") == 0) {
    getcwd(dir, 100);
    if (num_tokens < 2) {
      puts(dir);
    } else {
      if (strncmp(tokens[1], "/", 1) == 0
         || strncmp(tokens[1], "~", 1) == 0) {
        strcpy(tokens[1], dir);
      } else {
        strcat(dir, "/");
        strncat(dir, tokens[1], 50);
      }
      i = chdir(dir);
      if (i == -1) puts("Directory does not exist");
    }
  } else {
    for (i = 0; i < num_tokens; i++) {
      puts(tokens[i]);
    }
    tokens[num_tokens] = NULL;
    int pid = fork();
    if (pid == 0) {
      execvp(tokens[0], tokens);
      exit(0);
    } else {
      wait(NULL);
    }


  }
  
}


int main (int argc, char **argv)
{ 
  char line[50];
  char *tokens[25];
  int num_tokens;
  char *delim = " \t\r\n\v\f";
  
  signal(SIGINT, signal_handler);

    //puts(argv[1]);
    //puts(argv[2]);
    //printf("%d", argc);

  if (argc >= 3 && !strcmp(argv[1], "-p")) {
    prompt = argv[2];
  }

  while(1) {
    printf("%s$ ", prompt);
    
    fgets(line, 50, stdin);
    num_tokens = 0;
    tokens[num_tokens] = strtok(line, delim);
    while (tokens[num_tokens] != NULL) {
      num_tokens++;
      tokens[num_tokens] = strtok(NULL, delim);
    } 

    execute(num_tokens, tokens);

  }
  
}

