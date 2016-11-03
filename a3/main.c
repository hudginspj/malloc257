#include <sys/times.h>
#include<stdlib.h>
#include<stdio.h>

#define DOUBLE_XOR(a, b) (double) ((long long) a ^ (long long) b)

#define DOUBLE_SWAP(a, b)  a = DOUBLE_XOR(a, b); b = DOUBLE_XOR(a, b); a = DOUBLE_XOR(a, b); 

#define MIN(a, b)  (a<b) ? a : b

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


void transpose_multiply (double **a, double **b, double **c, int n)
{
  int i, j, k;

  for (i = 0; i<n; i++) {
    for (j = i+1; j<n; j++) {
      DOUBLE_SWAP(b[i][j], b[j][i]);
    }
  }

  for (i=0; i<n; i++) {
    for (j=0; j<n; j++) { 
      c[i][j] = 0;
      for (k=0; k<n; k++) {
        c[i][j]= c[i][j] + a[i][k] * b[k][j];
      }
    }
  }
}

void block_multiply (double **a, double **b, double **c, int n, int partitions)
{
  int i, j, k, i0, j0, k0;
  int block_size = ((n-1)/partitions)+1;
  int stop_i, stop_j, stop_k;


  for (i=0; i<n; i++) {
    for (j=0; j<n; j++) { 
      c[i][j] = 0;
    }
  }

  for (i0=0; i0<partitions; i0++) {
    for (j0=0; j0<partitions; j0++) { 
      for (k0=0; k0<partitions; k0++) {

        stop_i = MIN(n, (i0+1)*block_size);
        for (i=i0*block_size; i<stop_i; i++) {
          stop_j = MIN(n, (j0+1)*block_size);
          for (j=j0*block_size; j<stop_j; j++) { 
            stop_k = MIN(n, (k0+1)*block_size);
            for (k=k0*block_size; k<stop_k; k++) {
              c[i][j]= c[i][j] + a[i][k] * b[k][j];
            }
          }
        }
      }
    }
  }
}

void print_matrix(double **mat, int n) {
  int i,j;
  if (n < 10) {
    for (i=0; i<n; i++) {
      for (j=0; j<n; j++) {
        printf("%.1f ", mat[i][j]);
      }
      printf("\n");
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

  for (i=0; i<n; i++) {
    for (j=0; j<n; j++) {
      a[i][j]= i + j;
    }
  }

  for (i=0; i<n; i++) {
    for (j=0; j<n; j++) {
      b[i][j]= i - j;
    }
  }

  print_matrix(a, n);
  print_matrix(b, n);

  printf("Normal: ");
  start = ftime();
  multiply (a,b,c,n);
  stop = ftime();
  used = stop - start;
  mf = (n*n*n *2.0) / used / 1000000.0;
  printf ("\n");
  printf ( "Elapsed time:   %10.2f \n", used);
  printf ( "DP MFLOPS:       %10.2f \n", mf);
  print_matrix(c, n);

  printf("Block: ");
  start = ftime();
  block_multiply(a,b,c,n, 3);
  stop = ftime();
  used = stop - start;
  mf = (n*n*n *2.0) / used / 1000000.0;
  printf ("\n");
  printf ( "Elapsed time:   %10.2f \n", used);
  printf ( "DP MFLOPS:       %10.2f \n", mf);
  print_matrix(c, n);

  printf("Transpose: ");
  start = ftime();
  transpose_multiply(a,b,c,n);
  stop = ftime();
  used = stop - start;
  mf = (n*n*n *2.0) / used / 1000000.0;
  printf ("\n");
  printf ( "Elapsed time:   %10.2f \n", used);
  printf ( "DP MFLOPS:       %10.2f \n", mf);
  print_matrix(c, n);


  return (0);
}

