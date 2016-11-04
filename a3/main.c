#include <sys/times.h>
#include<stdlib.h>
#include<stdio.h>

#define DOUBLE_XOR(a, b) (double) ((long long) a ^ (long long) b)

#define DOUBLE_SWAP(a, b)  a = DOUBLE_XOR(a, b); b = DOUBLE_XOR(a, b); a = DOUBLE_XOR(a, b); 

#define SWAP(a, b, t) t = a; a = b; b = t;

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


void transpose(double **mat, int n) {
  int i,j;
  for (i = 0; i<n; i++) {
    for (j = i+1; j<n; j++) {
      DOUBLE_SWAP(mat[i][j], mat[j][i]);
    }
  }

}

void transpose_multiply (double **a, double **b, double **c, int n)
{
  int i, j, k;
  //double temp;

  for (i = 0; i<n; i++) {
    for (j = i+1; j<n; j++) {
      DOUBLE_SWAP(b[i][j], b[j][i]);
    }
  }

  for (i=0; i<n; i++) {
    for (j=0; j<n; j++) { 
      c[i][j] = 0;
      for (k=0; k<n; k++) {
        c[i][j]= c[i][j] + a[i][k] * b[j][k];
      }
    }
  }
}

void fast_transpose_multiply (double **a, double **b, double **c, int n)
{
  int i, j;
  double *ak, *bk, *astop;
  double sum;

  for (i = 0; i<n; i++) {
    for (j = i+1; j<n; j++) {
      DOUBLE_SWAP(b[i][j], b[j][i]);
    }
  }

  for (i=0; i<n; i++) {
    for (j=0; j<n; j++) { 
      sum = 0;
      ak = a[i];
      bk = b[j];
      astop = ak + n;
      while (ak<astop) {
        sum += *ak * *bk;
        ak++;
        bk++;
      }
      c[i][j] = sum;
    }
  }
}


void partition_multiply (double **a, double **b, double **c, int n, int partitions)
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

void block_multiply (double **a, double **b, double **c, int n, int block_size)
{
  int i, j, k, i0, j0, k0;
  int stop_i, stop_j, stop_k;


  for (i=0; i<n; i++) {
    for (j=0; j<n; j++) { 
      c[i][j] = 0;
    }
  }

  for (i0=0; i0<n; i0 += block_size) {
    for (j0=0; j0<n; j0 += block_size) { 
      for (k0=0; k0<n; k0 += block_size) {

        stop_i = MIN(n, i0 + block_size);
        for (i=i0; i<stop_i; i++) {
          stop_j = MIN(n, j0 + block_size);
          for (j=j0; j<stop_j; j++) { 
            stop_k = MIN(n, k0 + block_size);
            for (k=k0; k<stop_k; k++) {
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
  if (n <= 10) {
    for (i=0; i<n; i++) {
      for (j=0; j<n; j++) {
        printf("%.1f ", mat[i][j]);
      }
      printf("\n");
    }
  }

}


void start_time() {
  start = ftime();
}

void stop_and_analyze(double **mat, int n) {
  stop = ftime();
  used = stop - start;
  mf = ((n*n)  / 500000.0) * n  / used;
  //printf ("\n");
  printf("   Elapsed time: %10.2f", used);
  printf("   DP MFLOPS: %10.2f \n", mf);
  print_matrix(mat, n);
}



int main (void)
{
  int i, j, n, block_size;
  double **a, **b, **c;


  printf ( "Enter the value of n: ");
  scanf ( "%d", &n);

  //print_matrix(m,2);

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
      b[i][j]= i - (j/2) ;
    }
  }

  print_matrix(a, n);
  print_matrix(b, n);

  printf("  Normal: ");
  if (n < 1001) {
    start_time();
    multiply (a,b,c,n);
    stop_and_analyze(c, n);
  } else {
    printf("Skipping.\n");
  }
  

  

  

  printf("  Transpose Multiply: ");
  start_time();
  transpose_multiply(a,b,c,n);
  stop_and_analyze(c, n);

  transpose(b,n);

  for (block_size = 32; block_size <= 32; block_size *= 2) {
    printf("  Block (%d): ", block_size);
    start_time();
    block_multiply(a,b,c,n, block_size);
    stop_and_analyze(c, n);
  }

  printf("  Fast Transpose: ");
  start_time();
  fast_transpose_multiply(a,b,c,n);
  stop_and_analyze(c, n);


  return (0);
}

