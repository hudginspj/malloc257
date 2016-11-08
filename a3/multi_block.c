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
  usleep ( 10000 );
  int i, j, k, i0, j0, k0;
  int block_size = ((n-1)/partitions)+1;
  int stop_i, stop_j, stop_k;

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
            fprintf(stderr,"fork failed at %d\n",i);
            exit(1);
        } else if ( pid > 0 ) {
            //printf("parent: new child is %d\n",pid);

        } else {
          usleep ( 1000 );
          printf("child %d/%d/%d, parent is %d\n",i0,j0,k0, getppid());
          //child_transpose_multiply(a, b, c, offset, partition_size, n);
          stop_i = MIN(n, (i0+1)*block_size);
          for (i=i0*block_size; i<stop_i; i++) {
            stop_j = MIN(n, (j0+1)*block_size);
            for (j=j0*block_size; j<stop_j; j++) { 
              stop_k = MIN(n, (k0+1)*block_size);
              sum = 0;
              for (k=k0*block_size; k<stop_k; k++) {
                sum += a[i*n + k] * b[k*n + j];
              }
              sem_wait ( sem );
              c[i*n + j] += sum;
              sem_post ( sem );
            }
          }
          exit(0);
        }

        
      }
    }
  }

  for ( i = 0; i < (partitions * partitions * partitions); i++ ) wait(NULL);

}

void sem_arr_part_multiply (double *a, double *b, double *c, 
  int n, int partitions)
{
  usleep ( 10000 );
  int i, j, k, i0, j0, k0;
  int block_size = ((n-1)/partitions)+1;
  int stop_i, stop_j, stop_k;

  int pid;
  double sum;

  for (i=0; i<n; i++) {
    for (j=0; j<n; j++) { 
      c[i*n + j] = 0;
    }
  }


  for (i0=0; i0<partitions; i0++) {
    for (j0=0; j0<partitions; j0++) { 
      sem_t *output_block_sem = sems[i0 * partitions + j0];

      for (k0=0; k0<partitions; k0++) {
        pid = fork();
        if ( pid < 0 ) {
            fprintf(stderr,"fork failed at %d\n",i);
            exit(1);
        } else if ( pid > 0 ) {
            //printf("parent: new child is %d\n",pid);

        } else {
          usleep ( 1000 );
          printf("child %d/%d/%d, parent is %d\n",i0,j0,k0, getppid());
          //child_transpose_multiply(a, b, c, offset, partition_size, n);
          stop_i = MIN(n, (i0+1)*block_size);
          for (i=i0*block_size; i<stop_i; i++) {
            stop_j = MIN(n, (j0+1)*block_size);
            for (j=j0*block_size; j<stop_j; j++) { 
              stop_k = MIN(n, (k0+1)*block_size);
              sum = 0;
              for (k=k0*block_size; k<stop_k; k++) {
                sum += a[i*n + k] * b[k*n + j];
              }
              sem_wait ( output_block_sem );
              //printf("Accessed sem %d\n", i0*3 + j0);
              c[i*n + j] += sum;
              sem_post ( output_block_sem );
            }
          }
          exit(0);
        }

        
      }
    }
  }

  for ( i = 0; i < (partitions * partitions * partitions); i++ ) wait(NULL);

}


void million_sems_multiply (double *a, double *b, double *c, 
  int n, int partitions)
{
  usleep ( 10000 );
  int i, j, k, i0, j0, k0;
  int block_size = ((n-1)/partitions)+1;
  int stop_i, stop_j, stop_k;

  int pid;
  double sum;

  for (i=0; i<n; i++) {
    for (j=0; j<n; j++) { 
      c[i*n + j] = 0;
    }
  }


  for (i0=0; i0<partitions; i0++) {
    for (j0=0; j0<partitions; j0++) { 
      //sem_t *output_block_sem = sems[i0 * partitions + j0];

      for (k0=0; k0<partitions; k0++) {
        pid = fork();
        if ( pid < 0 ) {
            fprintf(stderr,"fork failed at %d\n",i);
            exit(1);
        } else if ( pid > 0 ) {
            //printf("parent: new child is %d\n",pid);

        } else {
          usleep ( 1000 );
          //printf("child %d/%d/%d, parent is %d\n",i0,j0,k0, getppid());
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
      //sem_t *output_block_sem = sems[i0 * partitions + j0];

      for (k0=0; k0<partitions; k0++) {
        pid = fork();
        if ( pid < 0 ) {
            fprintf(stderr,"fork failed at %d\n", 
              i0*partitions*partitions + j0*partitions + k0);
            exit(1);
        } else if ( pid > 0 ) {
            //printf("parent:  %d\n",i);

        } else {
          usleep ( 1000 );
          //printf("child %d/%d/%d, parent is %d\n",i0,j0,k0, getppid());
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
  //printf(start);
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


int main (int argc, char **argv)
{
  int i, j, n, block_size, shmfd;
  double *mem, *a, *b, *c, *c_check;


  //printf ( "Enter the value of n: ");
  //scanf ( "%d", &n);
  if (argc<2) exit(0);
  n =  atoi(argv[1]);
  block_size = argc > 2 ? atoi(argv[2]) : n/4;
  printf("N: %d, Block_Size: %d, %s", n, block_size, argv[1]);

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




  shmfd = shm_open ( "/pjh_sems", O_RDWR | O_CREAT, 0666 );
  if ( shmfd < 0 ) {
      fprintf(stderr,"Could not create pjh_sems\n");
      exit(1);
  }
  ftruncate ( shmfd, n*n*sizeof(sem_t) );
  sem_arr = (sem_t *) mmap ( NULL, n*n*sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0 );
  if ( mem == NULL ) {
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

  
  char sem_name[10] = "pjh_sem_0";
  //for (counter = '0'; counter<'9'; counter++) {
  for (i = 0; i < PARTITIONS*PARTITIONS; i++) {
    sem_name[8] = sem_name[8] + 1;
    sems[i] = sem_open ( sem_name, O_CREAT, 0666, 1 );
    if ( sems[i] == NULL ) {
      fprintf(stderr,"Could not create %s\n", sem_name);
      exit(1);
    }
    sem_unlink ( sem_name );
    //printf("Created %s\n", sem_name);
  }



  sem = sem_open ( "pjh_sem", O_CREAT, 0666, 1 );
  if ( sem == NULL ) {
    fprintf(stderr,"Could not create pjh semaphore\n");
      exit(1);
  }
  sem_unlink ( "pjh_sem" );


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

  //printf("  Fork Partition Multiply: ");
  //start_time();
  //fork_part_multiply(a,b,c, n, PARTITIONS);
  //stop_and_analyze(c, n);

  //printf("  Semaphore Array Multiply: ");
  //start_time();
  //sem_arr_part_multiply(a,b,c, n, PARTITIONS);
  //stop_and_analyze(c, n);

  printf(" Million Semaphore Block Multiply: \n");
  start_time();
  million_sems_block_multiply(a,b,c, n, block_size);
  stop_and_analyze(c, n);

  
  printf("  Fast Transpose Multiply: \n");
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


  for (i = 0; i < n*n; i++) {
    j = sem_destroy( &sem_arr[i]);
    if ( j != 0 ) {
      fprintf(stderr,"Could not destroy semaphore\n");
        exit(1);
    }
  }

  return (0);
}

