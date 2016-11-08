//#include <sys/times.h>
#include <sys/time.h>
//#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <semaphore.h>

#define DOUBLE_XOR(a, b) (double) ((long long) a ^ (long long) b)

#define DOUBLE_SWAP(a, b)  a = DOUBLE_XOR(a, b); b = DOUBLE_XOR(a, b); a = DOUBLE_XOR(a, b); 

#define MIN(a, b)  (a<b) ? a : b

#define PARTITIONS 8

sem_t *sem;
sem_t* sems[9];
sem_t* sem_arr;

double start, stop, used, mf;

double ftime(void);

double ftime (void)
{

    struct timeval t;
    gettimeofday(&t, NULL);
    return  (double) t.tv_sec + (double)t.tv_usec/1000000.0;
}


void linear_transpose_multiply (double *a, double *b, double *c, int n, int partitions)
{
  int ii, i, j;
  double *ak, *bk, *astop;
  double sum;
  int pid;
  //int partitions = 4;
  int p_size = ((n-1)/partitions)+1;
  //printf("p_size=%d\n", p_size);

  for (i = 0; i<n; i++) {
    for (j = i+1; j<n; j++) {
      DOUBLE_SWAP(b[i*n+j], b[j*n+i]);
    }
  }

  for (ii=0; ii<n; ii+=p_size) {
        
        pid = fork();
        if ( pid < 0 ) {
            fprintf(stderr,"fork failed after %d processes\n", 
              ii);
            exit(1);
        } else if ( pid > 0 ) {
            //printf("parent:  %d\n",i);

        } else {
          usleep ( 1000 );
          //printf("ii=%d / stop = %d\n", ii, MIN(ii+p_size, n));
          int stop = MIN(ii+p_size, n);
          for (i=ii; i<stop; i++) {
            //printf("i=%d\n", i);
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

              c[i*n+j] = sum;
              //printf("cval= %f, sum = %f, i = %d, j = %d", c[i*n+j], sum, i, j);
            }
          }
          //printf("%dX\n\n",ii);
          usleep(10000);
          exit(0);
      }
  }
  for ( i = 0; i < (partitions); i++ ) wait(NULL);
}



void million_sems_block_multiply (double *a, double *b, double *c, 
  int n, int block_size)
{
  usleep ( 10000 );
  int i, j, k, i0, j0, k0;
  int partitions = ((n-1)/block_size)+1;
  int stop_i, stop_j, stop_k;
  //printf("Partitions: %d", partitions);

  int pid;
  double sum;

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
            fprintf(stderr,"fork failed after %d processes\n", 
              i0*partitions*partitions + j0*partitions + k0);
            exit(1);
        } else if ( pid > 0 ) {
            //printf("parent:  %d\n",i);

        } else {
          usleep ( 1000 );
          stop_i = MIN(n, (i0+1)*block_size);
          for (i=i0*block_size; i<stop_i; i++) {
            stop_j = MIN(n, (j0+1)*block_size);
            for (j=j0*block_size; j<stop_j; j++) { 
              stop_k = MIN(n, (k0+1)*block_size);
              sum = 0;
              for (k=k0*block_size; k<stop_k; k++) {
                sum += a[i*n + k] * b[k*n + j];
              }
              sem_wait ( &sem_arr[i*n + j] );
              c[i*n + j] += sum;
              sem_post ( &sem_arr[i*n + j] );
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
  if (n <= 20) {
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
  printf("   Elapsed time: %10.2f", used);
  printf("   DP MFLOPS: %10.2f \n", mf);
  print_linear_matrix(mat, n);
}


int main (int argc, char **argv)
{
  int i, j, n, block_size, shmfd;
  double *mem, *a, *b, *c, *c_check;


  if (argc<2) exit(0);
  n =  atoi(argv[1]);
  block_size = argc > 2 ? atoi(argv[2]) : n/4;
  printf("N: %d, Block_Size: %d\n", n, block_size);


  shmfd = shm_open ( "/pjh_memory", O_RDWR | O_CREAT, 0666 );
  if ( shmfd < 0 ) {
      fprintf(stderr,"Could not create pjh_memory\n");
      exit(1);
  }
  ftruncate ( shmfd, 4*n*n*sizeof(double) );
  mem = (double *) mmap ( NULL, 4*n*n*sizeof(double), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0 );
  if ( mem == NULL ) {
      fprintf(stderr,"Could not map pjh_memory\n");
      exit(1);
  }
  close ( shmfd );
  shm_unlink ( "/pjh_memory" );

   a = mem;
  b = a + (n*n);
  c = b + (n*n);
  c_check = c + (n*n);


  shmfd = shm_open ( "/pjh_sems", O_RDWR | O_CREAT, 0666 );
  if ( shmfd < 0 ) {
      fprintf(stderr,"Could not create pjh_sems\n");
      exit(1);
  }
  ftruncate ( shmfd, n*n*sizeof(sem_t) );
  sem_arr = (sem_t *) mmap ( NULL, n*n*sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0 );
  if ( sem_arr == NULL ) {
      fprintf(stderr,"Could not map pjh_sems\n");
      exit(1);
  }
  close ( shmfd );
  shm_unlink ( "/pjh_sems" );
  
  for (i = 0; i < n*n; i++) {
    j = sem_init( &sem_arr[i], 1, 1 );
    if ( j != 0 ) {
      fprintf(stderr,"Could not init semaphore\n");
        exit(1);
    }
  }


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
  printf("  X\n");
  print_linear_matrix(b, n);
  printf("\n");

  //printf("Million Semaphore Block Multiply: \n");
  //start_time();
  //million_sems_block_multiply(a,b,c, n, block_size);
  //stop_and_analyze(c, n);

  for (i = 8; i <= 8; i *= 2) {
    printf("Fast Transpose Multiply: %d processes\n", i);
    start_time();
    linear_transpose_multiply(a,b,c_check, n, i);
    stop_and_analyze(c_check, n);
  }
  
  j = 0;
  for (i = 0; i < n*n; i++) {
    
    if (c[i] != c_check[i]){
      //printf("Check failed at i = %d\n", i);
      j++;
    }
  }
  printf("(%d) checks failed.\n", j);


  for (i = 0; i < n*n; i++) {
    j = sem_destroy( &sem_arr[i]);
    if ( j != 0 ) {
      fprintf(stderr,"Could not destroy semaphore\n");
        exit(1);
    }
  }

  return (0);
}

