#include <pthread.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "barrier.h" 
#define TAMANHOMAX 100
/* Bits of value to sort on. */
#define BITS 29

/* Synchronization tools. */
#define BARRIER_COUNT 1000
pthread_barrier_t barrier;

/* Thread arguments for radix sort. */
struct rs_args {
  int id;         /* thread index. */
  unsigned *val;  /* array. */
  unsigned *tmp;  /* temporary array. */
  int n;          /* size of array. */
  int *nzeros;    /* array of zero counters. */
  int *nones;     /* array of one counters. */
  int t;          /* number of threads. */
};

/* Global variables and utilities. */
struct rs_args *args;


/****************************************************************************\
 * Array utilities.
\****************************************************************************/

/* Copy array. */
void copy_array (unsigned *dest, unsigned *src, int n)
{
  for ( ; n > 0; n-- )
    *dest++ = *src++;
}

/* Print array. */
void print_array (unsigned *val, int n)
{
  int i;
  for ( i = 0; i < n; i++ )
    printf ("%d \n", val[i]);
  printf ("\n");
}

/* Fill array with random values. */
void random_array (unsigned *val, int n)
{
  int i;
  for ( i = 0; i < n; i++ ) {
  	val[i] = (unsigned)lrand48() & (unsigned)((1 << BITS) - 1);  	
  }
}

/* Check if array is sorted. */
int array_is_sorted (unsigned *val, int n)
{
  int i;
  for ( i = 1; i < n; i++ )
    if ( val[i-1] > val[i] )
      return 0;
  return 1;
}


/****************************************************************************\
 * Thread part of radix sort.
\****************************************************************************/

/* Individual thread part of radix sort. */
void radix_sort_thread (unsigned *val, /* Array of values. */
			unsigned *tmp,           /* Temp array. */
			int start, int n,        /* Portion of array. */
			int *nzeros, int *nones, /* Counters. */
			int thread_index,        /* My thread index. */
			int t)                   /* Number of theads. */
{
  /* THIS ROUTINE WILL REQUIRE SOME SYNCHRONIZATION. */
  /* MAYBE A CALL TO barrier() or TWO. */

  unsigned *src, *dest;
  int bit_pos;
  int index0, index1;
  int i;
  printf("###### Got in main function, thread %d\n", thread_index);

  /* Initialize source and destination. */
  src = val;
  dest = tmp;

  /* For each bit... */
  for ( bit_pos = 0; bit_pos < BITS; bit_pos++ ) {

    /* Count elements with 0 in bit_pos. */
    nzeros[thread_index] = 0;
    for ( i = start; i < start + n; i++ ) {
      if ( ((src[i] >> bit_pos) & 1) == 0 ) {
	  	nzeros[thread_index]++;      	
      }	
    }
    nones[thread_index] = n - nzeros[thread_index];
	
    /* Ensure all threads have reached this point, and then let continue */
    pthread_barrier_wait(&barrier);

    /* Get starting indices. */
    index0 = 0;
    index1 = 0;
    for ( i = 0; i < thread_index; i++ ) {
      index0 += nzeros[i];
      index1 += nones[i];
    }
    index1 += index0;
    for ( ; i < t; i++ ) {
      index1 += nzeros[i];
	}
	
    /* Ensure all threads have reached this point, and then let continue */
    pthread_barrier_wait(&barrier);

    /* Move values to correct position. */
    for ( i = start; i < start + n; i++ ) {
      if ( ((src[i] >> bit_pos) & 1) == 0 ) {
	  	dest[index0++] = src[i];      	
      } else {
	  	dest[index1++] = src[i];      	
      }
    }
	
    /* Ensure all threads have reached this point, and then let continue */
    pthread_barrier_wait(&barrier);
	
    /* Swap arrays. */
    tmp = src;
    src = dest;
    dest = tmp;
  }
  printf ("\n====== Printing nzeros array of thread %d\n\n", thread_index);
  print_array (nzeros, n);
  printf ("\n====== Printing nones array of thread %d\n\n", thread_index);
  print_array (nones, n);
  // printf ("\n====== Printing val array of thread %d\n\n", thread_index);
  // print_array (val, n);
  // printf ("\n====== Printing temp array of thread %d\n\n", thread_index);
  // print_array (dest, n);
}

// /* Thread main routine. */
// void thread_main (struct rs_args *args)
// {
//   int start;
//   int n;
// 
//   /* Get portion of array to process. */
//   n = args->n / args->t; /* Number of elements this thread is in charge of */
//   start = args->id * n; /* Thread is in charge of [start, start+n] elements */
// 
//   /* Perform radix sort. */
//   radix_sort_thread (args->val, args->tmp, start, n,
// 		     args->nzeros, args->nones, args->id, args->t);
// }



/* Thread main routine. */
void thread_work (int rank)
{
  int start, count, n;
  int index = rank;
  printf("\n####### Thread_work: THREAD %d = %d \n\n", rank, args[index].id);
  /* Ensure all threads have reached this point, and then let continue. */
  // pthread_barrier_wait(&barrier);
  
  /* Get portion of array to process. */
  n = args[index].n / args[index].t; /* Number of elements this thread is in charge of */
  start = args[index].id * n; /* Thread is in charge of [start, start+n] elements */

  /* Perform radix sort. */
  radix_sort_thread (args[index].val, args[index].tmp, start, n,
  		     args[index].nzeros, args[index].nones, args[index].id, args[index].t);
}



/****************************************************************************\
 * Main part of radix sort.
\****************************************************************************/

/* Radix sort array. */
void radix_sort (unsigned *val, int n, int t)
{
  unsigned *tmp;
  int *nzeros, *nones;
  int r, i;
  
  /* Thread-related variables. */
  long thread;
  pthread_t* thread_handles;

  /* Allocate temporary array. */
  tmp = (unsigned *) malloc (n * sizeof(unsigned));
  if (!tmp) { fprintf (stderr, "Malloc failed.\n"); exit(1); }

  /* Allocate counter arrays. */
  nzeros = (int *) malloc (t * sizeof(int));
  if (!nzeros) { fprintf (stderr, "Malloc failed.\n"); exit(1); }
  nones = (int *) malloc (t * sizeof(int));
  if (!nones) { fprintf (stderr, "Malloc failed.\n"); exit(1); }

  /* Initialize thread handles and barrier. */
  thread_handles = malloc (t * sizeof(pthread_t));
  pthread_barrier_init (&barrier, NULL, t);
  
  /* Initialize thread arguments. */
  for ( i = 0; i < t; i++ ) {
    args[i].id = i;
    args[i].val = val;
    args[i].tmp = tmp;
    args[i].n = n;
    args[i].nzeros = nzeros;
    args[i].nones = nones;
    args[i].t = t;
	
	/* Create a thread. */
	printf ("####### CREATING THREAD id = %d\n", args[i].id);
    pthread_create (&thread_handles[i], NULL, thread_work, i);
  }
  
  printf ("####### THREADS SHOULD BE WORKING NOW \n");
  
  /* Wait for threads to join and terminate. */
  for ( i = 0; i < t; i++ ) {
    pthread_join (thread_handles[i], NULL);
    printf ("####### THREAD %d SHOULD BE FINISHED \n", i);
  }

  /* Free thread arguments. */
  pthread_barrier_destroy(&barrier);
  free (thread_handles);
  free (args);

  printf ("\n====== Before return to main: val array ======\n");
  print_array (val, n);
  printf ("\n====== Before return to main: tmp array ======\n");
  print_array (tmp, n);

  /* Copy array if necessary. */
  if ( BITS % 2 == 1 ) {
    copy_array (val, tmp, n);  	
  }

  /* Free temporary array and couter arrays. */
  free (nzeros);
  free (nones);
  free (tmp);
}

int main(int argc, char *argv[])
{
  FILE *fs;
  unsigned *num;
  int tamanhoVet=1;
  int novoTamanhoVetor;
  int c,c1,c2,t;
  float parte=0.0;
  register int i=0;
  int numThreads;
  numThreads=atoi(argv[1]);
  pthread_t thread[numThreads];
  args = (struct rs_args *) malloc (numThreads * sizeof(struct rs_args));
  if (argc<4)
  { 
    fprintf(stderr,"uso:\n\t %s num de threads arquivos de entrada arquivo de saida\n",argv[0]);
    exit(0);
  }
  

  num = calloc(TAMANHOMAX,sizeof(unsigned));
  for(c=2;c<=(argc-2);c++){
  fs = fopen(argv[c], "r");


  while (!feof(fs))
  {
    if (i<TAMANHOMAX*tamanhoVet)
    {
        fscanf(fs,"%i", &num[i]);
        i++;
    }
    else
    {
       tamanhoVet++;
       num = realloc(num, TAMANHOMAX*tamanhoVet*sizeof(unsigned));
       fscanf(fs,"%i", &num[i]);
       i++;
    }
  }
  
}
  num = realloc(num, (i-1)*sizeof(unsigned));
  novoTamanhoVetor = i-1;
/*    if (numThreads==2){
     parte=novoTamanhoVetor/2;
     ct vs[parte];
      for (c=0;c<2;c++){
      pthread_t tr[2];
      pthread_create(tr[c],NULL,radixsort,vs[c])
      radixsort(vs[c].valores[])
      }
   }else if(numThreads==4){
     parte=novoTamanhoVetor/4;     
      for (c=0;c<4;c++){
       //insira criação de threads aqui?
      }  
   }else if(numThreads==8){
     parte=novoTamanhoVetor/8

   }else if(numThreads==16){
    parte=novoTamanhoVetor/16

   }*/
  radix_sort (num,novoTamanhoVetor,numThreads);
  printf("Tamanho vetor: %d\n", novoTamanhoVetor);
  
  fs = fopen(argv[argc-1], "w");
  for (i=0; i<novoTamanhoVetor; i++){
      fprintf(fs, "%i\n",num[i]);
    }
   
  
  pthread_exit(NULL);
  return 0;
}
