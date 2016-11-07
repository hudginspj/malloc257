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

//#define SWAP(a, b, t) t = a; a = b; b = t;

#define MIN(a, b)  (a<b) ? a : b

#define PARTITIONS 3

sem_t *sem;
sem_t* sems[PARTITIONS*PARTITIONS];

double start, stop, used, mf;

double ftime(void);

double ftime (void)
{
    //struct tms t;
    //times ( &t );
    //return (t.tms_utime + t.tms_stime +t.tms_cutime + t.tms_cstime) / 100.0;

    struct timeval t;
    gettimeofday(&t, NULL);
    //printf("secs: %ld, usecs: %ld, %f\n", t.tv_sec, t.tv_usec, (double)t.tv_usec/1000000.0);
    return  (double) t.tv_sec + (double)t.tv_usec/1000000.0;
  //clock_t t;
  //t = clock();
  //printf("Time: %f\n", (float)t/CLOCKS_PER_SEC);
  //return( (float)t/ CLOCKS_PER_SEC) ;
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
  for (i = 0; i<n; i++) {
    for (j = i+1; j<n; j++) {
      DOUBLE_SWAP(b[i*n+j], b[j*n+i]);
    }
  }
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


void block_transpose(double *mat, int i0, int j0, int n, int block_size) {
  int i,j;
  for (i = 0; i<block_size; i++) {
    for (j = i+1; j<block_size; j++) {
      DOUBLE_SWAP(mat[(i0+i)*n+(j0+j)], mat[(i0+j)*n+(j0+i)]);
    }
  }
}

void multiply_block(double *a, double *b, double *c,
  int i0, int j0, int k0,
  int n, int partitions, int block_size,
  sem_t *output_block_sem) {
  int i, j, k;
  double sum;

  for (i=0; i<block_size; i++) {
      for (j=0; j<block_size; j++) { 
              sum = 0;
              for (k=0; k<block_size; k++) {
                //printf("a[%d] * b[%d]\n",
                 // ((i0*block_size) + i)*n + (k0*block_size + k),
                 // ((k0*block_size) + j)*n + (j0*block_size + k));
                sum += a[((i0*block_size) + i)*n + (k0*block_size + k)] 
                     * b[((k0*block_size) + j)*n + (j0*block_size + k)];
              }
              sem_wait ( output_block_sem );
              c[((i0*block_size) + i)*n + ((j0*block_size) + j)] += sum;
              sem_post ( output_block_sem );
            }
          }

}


void block_transpose_multiply (double *a, double *b, double *c, 
  int n, int partitions)
{
  usleep ( 10000 );
  //int i, j, k;
  int i0, j0, k0;
  int block_size = ((n-1)/partitions)+1;
  //int stop_k; //stop_i, stop_j,

  int pid;
  //double sum;

  for (i0=0; i0<n; i0++) {
    for (j0=0; j0<n; j0++) { 
      c[i0*n + j0] = 0;
    }
  }
  for (i0=0; i0<partitions; i0++) {
    for (j0=0; j0<partitions; j0++) { 
      block_transpose(b, i0*block_size, j0*block_size, n, block_size);
    }
  }
  
  for (i0=0; i0<partitions; i0++) {
    for (j0=0; j0<partitions; j0++) { 
      sem_t *output_block_sem = sems[i0 * partitions + j0];

      //block_transpose(i0, j0, MIN(block_size, (n-i0)), MIN(block_size, (n-j0)));
      //block_transpose(b, i0*block_size, j0*block_size, n, block_size);


      for (k0=0; k0<partitions; k0++) {
        pid = fork();
        if ( pid < 0 ) {
            fprintf(stderr,"fork failed at %d\n",i0);
            exit(1);
        } else if ( pid > 0 ) {
            //printf("parent: new child is %d\n",pid);

        } else {
          usleep ( 1000 );
          //printf("child %d/%d/%d, parent is %d\n",i0,j0,k0, getppid());
          multiply_block(a, b, c,
              i0, j0, k0,
              n,  partitions, block_size,
              output_block_sem);
          /*//child_transpose_multiply(a, b, c, offset, partition_size, n);
          //stop_i = MIN(n, (i0+1)*block_size);
          //stop_i = MIN(block_size, (n-i0));
          for (i=0; i<block_size; i++) {
            //stop_j = MIN(n, (j0+1)*block_size);
            //stop_j = MIN(block_size, (n-j0));
            for (j=0; j<block_size; j++) { 
              //stop_k = MIN(n, (k0+1)*block_size);
              sum = 0;
              //for (k=k0*block_size; k<stop_k; k++) {
              for (k=0; k<block_size; k++) {

                sum += a[((i0*block_size) + i)*n + (k0*block_size + k)] 
                     * b[((k0*block_size) + j)*n + (j0*block_size + k)];
              }
              sem_wait ( output_block_sem );
              c[((i0*block_size) + i)*n + ((j0*block_size) + j)] += sum;
              sem_post ( output_block_sem );
            }
          }*/
          exit(0);
        }
        
      }
    }
  }
  for ( i0 = 0; i0 < (partitions * partitions * partitions); i0++ ) wait(NULL);
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


int main (void)
{
  int i, j, n, shmfd;
  double *mem, *a, *b, *c, *c_check;


  printf ( "Enter the value of n: ");
  scanf ( "%d", &n);
  n = n - (n%PARTITIONS); //TODO pad matrix

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
  

  
  char sem_name[10] = "pjh_sem_0";
  for (i = 0; i < PARTITIONS*PARTITIONS; i++) {
    sem_name[8] = sem_name[8] + 1;
    sems[i] = sem_open ( sem_name, O_CREAT, 0666, 1 );
    if ( sems[i] == NULL ) {
      fprintf(stderr,"Could not create %s\n", sem_name);
      exit(1);
    }
    sem_unlink ( sem_name );
    printf("Created %s\n", sem_name);
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

  for (i=0; i<n; i++) {
    for (j=0; j<n; j++) { 
      c[i*n + j] = 0;  //TODO don't need this
    }
  }

  print_linear_matrix(a, n);
  printf(" X \n");
  print_linear_matrix(b, n);

  printf("  Fast Transpose Multiply: ");
  start_time();
  linear_transpose_multiply(a,b,c_check, n);
  stop_and_analyze(c_check, n); 

/*8

  block_transpose(b, 0, 0, n, 2);
  block_transpose(b, 0, 2, n, 2);
  block_transpose(b, 0, 4, n, 2);
  block_transpose(b, 2, 0, n, 2);
  block_transpose(b, 2, 2, n, 2);
  block_transpose(b, 2, 4, n, 2);
  block_transpose(b, 4, 0, n, 2);
  block_transpose(b, 4, 2, n, 2);
  block_transpose(b, 4, 4, n, 2);

  printf("  MATRIX B: \n");
  print_linear_matrix(b, n);

  multiply_block(a,b,c,  0,2,0, n, PARTITIONS, 2, sem);
  multiply_block(a,b,c,  0,2,1, n, PARTITIONS, 2, sem);
  multiply_block(a,b,c,  0,2,2, n, PARTITIONS, 2, sem);
  //printf("Next block\n");
  //multiply_block(a,b,c,  1,1,0, n, PARTITIONS, 2, sem);


  printf("  MATRIX C: \n");
  print_linear_matrix(c, n);*/

  /*int i0, k0, j0, block_size=2;
  for (i0=0; i0<PARTITIONS; i0++) {
    for (j0=0; j0<PARTITIONS; j0++) { 
      block_transpose(b, i0*block_size, j0*block_size, n, block_size);
    }
  }

  //int i0, k0, j0, block_size=2;
  for (i0=0; i0<PARTITIONS; i0++) {
    for (j0=0; j0<PARTITIONS; j0++) { 
      //block_transpose(b, i0*block_size, j0*block_size, n, block_size);
      for (k0=0; k0<PARTITIONS; k0++) {
          multiply_block(a, b, c,
              i0, j0, k0,
              n,  PARTITIONS, block_size,
              sem);
      }
    }
  }

  printf("  MATRIX C: \n");
  print_linear_matrix(c, n);*/


  printf("  Block Transpose Multiply3: \n");
  start_time();
  block_transpose_multiply(a,b,c, n, PARTITIONS);
  stop_and_analyze(c, n);

  printf("  MATRIX B: \n");
  print_linear_matrix(b, n);
  

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

