////////////////////////////////////////////////////////////////////////////////
//
//  File           : TempAnalysis.c
//  Description    : Analyzes temperature data as specified for project 3.
//                   Month names must begin with an upper case letter.  
//
//   Author        : Paul Hudgins
//   Last Modified : 11/23/16

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void printheading() {
  puts("Paul Hudgins\nCMSC 205 - Project 3\n11/23/16");
}

int numDaysInMonth(char *month) {
   if (!strncmp(month, "Feb", 3)) {
     return(28);
   } else if (!strncmp(month, "Sep", 3)) {
     return(30);
   } else if (!strncmp(month, "Apr", 3)) {
     return(30);
   } else if (!strncmp(month, "Jun", 3)) {
     return(30);
   } else if (!strncmp(month, "Nov", 3)) {
     return(30);
   } 

   return(31);
}

void averageTemp(float *avg, int *arr, int len) {
  int i;
   *avg = 0;
   for (i = 0; i < len; i++) {
     *avg += (float) arr[i];
   }
   *avg = *avg / (float) len;
}


int main (int argc, char **argv) { 
  int scan_return, i, mon_len, wave;
  float avg;
  char month_name[20];
  int temps[31];
  
  printheading();

  if (argc != 2) {
     puts("Enter one file name.");
  }
  FILE *input = fopen( argv[1], "r");
  
  month_name[19] = '\0';
  fscanf(input, "%s", month_name);
  mon_len = numDaysInMonth(month_name);
  i = 0;
  while (i < mon_len) {
    scan_return = fscanf(input, "%d", &temps[i]);
    if (scan_return == EOF) {
      puts("Not enough data points for month");
      fclose(input);
      return(0);
    }
    i++;
  }
  fclose(input);
  
  averageTemp(&avg, temps, mon_len);
  printf("The average temperature for %s was %.2f\n", month_name, avg);
  wave = 0;
  for (i = 0; i < mon_len; i++) {
    if ((mon_len-i)>=3
        && (float)temps[i] > avg
        && (float)temps[i+1] > avg
        && (float)temps[i+2] > avg) {
      wave = 1;
    }
    if ((float)temps[i] <= avg) {
      wave = 0;
    }
    printf("%2d  %3d %s\n",
      i + 1,
      temps[i],
      (wave) ? "+" : "");
  }

  return (0);
}

