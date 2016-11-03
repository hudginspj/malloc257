#include <sys/times.h>
#include<stdlib.h>
#include<stdio.h>

double start, stop, used, mf;

double ftime(void);
void multiply (double **a, double **b, double **c, int n);

double ftime (void)
{
    struct tms t;
    
    times ( &t );
 
    return (t.tms_utime + t.tms_stime) / 100.0;
}

void multiply (double **a, double **b, double **c, int n)
{
   int i, j, k;

   for (i=0; i<n; i++)
   {
     for (j=0; j<n; j++)
       
         c[i][j] = 0;
    }

    for (i=0; i<n; i++)
    {
       for (j=0; j<n; j++)
       {
         for (k=0; k<n; k++)
           c[i][j]= c[i][j] + a[i][k] * b[k][j];
        }
     }
  }

int main (void)
{
   int i, j, n;
   double **a, **b, **c;

    printf ( "Enter the value of n: ");
    scanf ( "%d", &n);

   //Populate arrays....
     a= (double**)malloc(n*sizeof(double));
     b= (double**)malloc(n*sizeof(double));
     c= (double**)malloc(n*sizeof(double));

     for (i=0; i<n; i++)
     {
       a[i]= malloc(sizeof(double)*n);
       b[i]= malloc(sizeof(double)*n);
       c[i]= malloc(sizeof(double)*n);
      }

     for (i=0; i<n; i++)
     {
        for (j=0; j<n; j++)
         a[i][j]=8;
      }

     for (i=0; i<n; i++)
     {
        for (j=0; j<n; j++)
          b[i][j]=7;
      }

      start = ftime();
      multiply (a,b,c,n);
      stop = ftime();
      used = stop - start;
      mf = (n*n*n *2.0) / used / 1000000.0;
      printf ("\n");
      printf ( "Elapsed time:   %10.2f \n", used);
      printf ( "DP MFLOPS:       %10.2f \n", mf);
      return (0);
}

