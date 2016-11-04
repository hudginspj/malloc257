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



void transpose(double **mat, int n) {
  int i,j;
  for (i = 0; i<n; i++) {
    for (j = i+1; j<n; j++) {
      DOUBLE_SWAP(mat[i][j], mat[j][i]);
    }
  }

}

void linear_transpose(double *mat, int n) {
  int i,j;
  for (i = 0; i<n; i++) {
    for (j = i+1; j<n; j++) {
      DOUBLE_SWAP(mat[i*n+j], mat[j*n+i]);
    }
  }

}

void fast_transpose_multiply (double **a, double **b, double *c, int n)
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
      *c = sum;
      c++;
    }
  }
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


/*
0.0 1.0 2.0 3.0 4.0
1.0 2.0 3.0 4.0 5.0
2.0 3.0 4.0 5.0 6.0
3.0 4.0 5.0 6.0 7.0
4.0 5.0 6.0 7.0 8.0
0.0 0.0 -1.0 -1.0 -2.0
1.0 1.0 0.0 0.0 -1.0
2.0 2.0 1.0 1.0 0.0
3.0 3.0 2.0 2.0 1.0
4.0 4.0 3.0 3.0 2.0
  Linear Transpose Multiply:    Elapsed time:       0.00   DP MFLOPS:        inf
30.0 30.0 20.0 20.0 10.0
40.0 40.0 25.0 25.0 10.0
50.0 50.0 30.0 30.0 10.0
60.0 60.0 35.0 35.0 10.0
70.0 70.0 40.0 40.0 10.0
*/

void child_transpose_multiply (double *a, double *b, double *c, 
      int offset, int m, int n)
{
  int i, j;
  double *ak, *bk, *astop;
  double sum;
  int counter =0;//TODO debugging

  a += offset;
  b += offset;
  c += offset;

  for (i=0; i<m; i++) {
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
      counter++;
    }
  }
  printf("Set C %d times for offset  %d.\n", counter, offset);
}

void fork_transpose_multiply (double *a, double *b, double *c, int n)
{
  int i, j;
  
  int partitions = 3;
  int partition_size = ((n-1)/partitions)+1;

  int offset; //TODO debugging

  printf("Partition size: %d\n", partition_size);

  int pid;

  for (i = 0; i<n; i++) {
    for (j = i+1; j<n; j++) {
      DOUBLE_SWAP(b[i*n+j], b[j*n+i]);
    }
  }

  offset = 0;
  for ( i = 0; i < partitions-1; i++ ) {
        pid = fork();
        if ( pid < 0 ) {
            fprintf(stderr,"fork failed at %d\n",i);
            exit(1);
        } else if ( pid > 0 ) {
            printf("parent: new child is %d\n",pid);

        } else {
            usleep ( 1000 );
            printf("child %d, parent is %d\n",i, getppid());
            child_transpose_multiply(a, b, c, offset, partition_size, n);
            exit(0);
        }
        offset += n*partition_size;
        //a += partition_size*n;
        //b += partition_size*n;
        //c += partition_size*n;
  } 
  child_transpose_multiply(a, b, c, offset,
   (n-(partition_size * (partitions-1))), n);

  for ( i = 0; i < partitions-1; i++ ) wait(NULL);

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
  double *mem, *a, *b, *c;


  printf ( "Enter the value of n: ");
  scanf ( "%d", &n);

  //kids = argc > 1 ? atoi(argv[1]) : 5;
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



  //Populate arrays....
  //a= (double**)malloc(n*sizeof(double));
  //b= (double**)malloc(n*sizeof(double));
  //c= (double*)malloc(n*n*sizeof(double));

  for (i=0; i<n; i++)
  {
    //a[i]= malloc(sizeof(double)*n);
    //b[i]= malloc(sizeof(double)*n);
    //c[i]= malloc(sizeof(double)*n);
  }

  for (i=0; i<n; i++) {
    for (j=0; j<n; j++) {
      //a[i*n + j]= i + j;
      a[i*n + j]= 1;
    }
  }

  for (i=0; i<n; i++) {
    for (j=0; j<n; j++) {
      //b[i*n + j]= i - (j/2) ;
      b[i*n + j]= 2;
    }
  }

  print_linear_matrix(a, n);
  print_linear_matrix(b, n);

  printf("  Linear Transpose Multiply: ");
  start_time();
  linear_transpose_multiply(a,b,c,n);
  stop_and_analyze(c, n);

  printf("  Just Transpose");
  linear_transpose(b,n);
  print_linear_matrix(b, n);

  printf("  Fork Transpose Multiply: \n");
  usleep ( 10000 );
  start_time();
  fork_transpose_multiply(a,b,c,n);
  stop_and_analyze(c, n);

  return (0);
}

