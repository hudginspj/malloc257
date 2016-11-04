#include <sys/times.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

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

void linear_transpose_multiply (double *a, double *b, double *c, int n)
{
  int i, j;
  double *ak, *bk, *astop;
  double sum;

  for (i = 0; i<n; i++) {
    for (j = i+1; j<n; j++) {
      DOUBLE_SWAP(b[i*n+j], b[j*n+i]);
    }
  }

  for (i=0; i<n; i++) {
    for (j=0; j<n; j++) { 
      sum = 0;
      ak = a + (i*n);
      bk = b + (j*n);
      astop = ak + n;
      while (ak<astop) {
        sum += *ak * *bk;
        ak++;
        bk++;
      }
      *c = sum;
      c++;
    }
  }
}

void lin_partition_multiply (double *a, double *b, double *c, 
    int n, int partitions)
{
  int i, j, k, i0, j0, k0;
  int block_size = ((n-1)/partitions)+1;
  int stop_i, stop_j, stop_k;


  for (i=0; i<n; i++) {
    for (j=0; j<n; j++) { 
      c[i*n + j] = 0;
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
              c[i*n + j]= c[i*n + j] + a[i*n + k] * b[k*n + j];
            }
          }
        }
      }
    }
  }
}





void fork_part_multiply (double *a, double *b, double *c, 
  int n, int partitions)
{
  int i, j, k, i0, j0, k0;
  int block_size = ((n-1)/partitions)+1;
  int stop_i, stop_j, stop_k;
  int pid;

  for (i=0; i<n; i++) {
    for (j=0; j<n; j++) { 
      c[i*n + j] = 0;
    }
  }


  for (i0=0; i0<partitions; i0++) {
    for (j0=0; j0<partitions; j0++) { 
      for (k0=0; k0<partitions; k0++) {
        pid = fork();
        if ( pid < 0 ) {
            fprintf(stderr,"fork failed at %d\n",i);
            exit(1);
        } else if ( pid > 0 ) {
            printf("parent: new child is %d\n",pid);

        } else {
          usleep ( 1000 );
          printf("child %d/%d/%d, parent is %d\n",i0,j0,k0, getppid());
          //child_transpose_multiply(a, b, c, offset, partition_size, n);
          stop_i = MIN(n, (i0+1)*block_size);
          for (i=i0*block_size; i<stop_i; i++) {
            stop_j = MIN(n, (j0+1)*block_size);
            for (j=j0*block_size; j<stop_j; j++) { 
              stop_k = MIN(n, (k0+1)*block_size);
              for (k=k0*block_size; k<stop_k; k++) {
                c[i*n + j]= c[i*n + j] + a[i*n + k] * b[k*n + j];
              }
            }
          }
          exit(0);
        }

        
      }
    }
  }

  for ( i = 0; i < (partitions * partitions * partitions); i++ ) wait(NULL);

}


void print_linear_matrix(double *mat, int n) {
  int i,j;
  if (n <= 10) {
    for (i=0; i<n; i++) {
      for (j=0; j<n; j++) {
        printf("%.1f ", *mat);
        mat++;
      }
      printf("\n");
    }
  }
}


void start_time() {
  start = ftime();
}

void stop_and_analyze(double *mat, int n) {
  stop = ftime();
  used = stop - start;
  mf = ((n*n)  / 500000.0) * n  / used;
  //printf ("\n");
  printf("   Elapsed time: %10.2f", used);
  printf("   DP MFLOPS: %10.2f \n", mf);
  print_linear_matrix(mat, n);
}


int main (void)
{
  int i, j, n, shmfd;
  double *mem, *a, *b, *c, *c_check;


  printf ( "Enter the value of n: ");
  scanf ( "%d", &n);

  c_check = (double *)malloc(n*n*sizeof(double));

  shmfd = shm_open ( "/pjh_memory", O_RDWR | O_CREAT, 0666 );
  if ( shmfd < 0 ) {
      fprintf(stderr,"Could not create pjh_memory\n");
      exit(1);
  }
  ftruncate ( shmfd, 3*n*n*sizeof(double) );
  mem = (double *) mmap ( NULL, 3*n*n*sizeof(double), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0 );
  if ( mem == NULL ) {
      fprintf(stderr,"Could not map pjh_memory\n");
      exit(1);
  }
  
  close ( shmfd );
  shm_unlink ( "/pjh_memory" );

  a = mem;
  b = a + (n*n);
  c = b + (n*n);

  for (i=0; i<n; i++) {
    for (j=0; j<n; j++) {
      a[i*n + j]= i + j;
      //a[i*n + j]= 1;
    }
  }

  for (i=0; i<n; i++) {
    for (j=0; j<n; j++) {
      b[i*n + j]= i - (j/2) ;
      //b[i*n + j]= 2;
    }
  }

  print_linear_matrix(a, n);
  print_linear_matrix(b, n);

  //printf("  Linear Partition Multiply: ");
  //start_time();
  //lin_partition_multiply(a,b,c, n, 3);
  //stop_and_analyze(c, n);
  //for (i=0; i<n*n; i++) {
  //  c[i] = 0;
  //}

  printf("  Fork Partition Multiply: ");
  start_time();
  fork_part_multiply(a,b,c, n, 3);
  stop_and_analyze(c, n);

  
  printf("  Fast Transpose Multiply: ");
  start_time();
  linear_transpose_multiply(a,b,c_check, n);
  stop_and_analyze(c_check, n);

  for (i = 0; i < n*n; i++) {
    j = 0;
    if (c[i] != c_check[i]){
      printf("Check failed at i = %d\n", i);
      j++;
    }
  }
  printf("(%d) checks failed.\n", j);

  return (0);
}

