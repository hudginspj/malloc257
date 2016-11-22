#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main (int argc, char **argv) { 
  
  if (argc == 2) {
    FILE * input = fopen( argv[1], "r");
    
    int line_counter = 1, byte_counter = 0;
    char c = '\0';
    while (c != EOF) {
      if (c == '\n') line_counter++;
      byte_counter++;
      c = fgetc(input);
    }
    
    printf("File size %d bytes\n", byte_counter);
    printf("Contains %d lines\n\n", line_counter);

    fclose(input);
  } else {
    puts("This assignment is all in one file.");
    puts("Please provide a file as an argument.\n");
  }
  
  int c_temp, limit, step;
  printf("Specify a lowest temperature, in Celsius: ");
  scanf("%d", &c_temp);
  printf("Specify an upper limit: ");
  scanf("%d", &limit);
  printf("Specify in an interval: ");
  scanf("%d", &step);
  
  puts("Celsius    Farenheit");
  float f_temp;
  for (; c_temp <= limit; c_temp += step) {
    f_temp = ((float) c_temp * 9 / 5) + 32;
    printf("%7.3f    %7.3f\n", (float) c_temp, f_temp);
  }
  return (0);
}

