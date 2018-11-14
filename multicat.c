#include <pthread.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "barrier.h" 
#define TAMANHOMAX 100
#define BITS 29

#define BARRIER_COUNT 1000
pthread_barrier_t barrier;

/* Argumentos da thread para o radix sort */
struct rs_args {
  int id;         /* index da thread */
  unsigned *val;  /* vetor */
  unsigned *tmp;  /* vetor temporário */
  int n;          /* tamanho do array */
  int *nzeros;    /* contador de vetores de zeros */
  int *nones;     /* contador de vetores de um */
  int t;          /* número de threads */
};

/* Variáveis globais e utilidades. */
struct rs_args *args;




void copy_array (unsigned *dest, unsigned *src, int n)
{
  for ( ; n > 0; n-- )
    *dest++ = *src++;
}

/*
void print_array (unsigned *val, int n)
{
  int i;
  for ( i = 0; i < n; i++ )
    printf ("%d \n", val[i]);
  printf ("\n");
}
*/
/* 
/* Checa se o vetor está ordenado. */
int array_is_sorted (unsigned *val, int n)
{
  int i;
  for ( i = 1; i < n; i++ )
    if ( val[i-1] > val[i] )
      return 0;
  return 1;
}



/* Individual thread part of radix sort. */
void radix_sort_thread (unsigned *val, /* Vetor. */
			unsigned *tmp,           /* Vetor temporário. */
			int start, int n,        /* Pedaço do array. */
			int *nzeros, int *nones, /* Contadores */
			int thread_index,        /* Indíce da thread. */
			int t)                   /* Número de threads */
{
  /* THIS ROUTINE WILL REQUIRE SOME SYNCHRONIZATION. */
  /* MAYBE A CALL TO barrier() or TWO. */

  unsigned *src, *dest;
  int bit_pos;
  int index0, index1;
  int i;


  /* Inicializa fonte e destina */
  src = val;
  dest = tmp;

  /* Para cada bit */
  for ( bit_pos = 0; bit_pos < BITS; bit_pos++ ) {

    /* Conta elementos com 0 na bit_pos */
    nzeros[thread_index] = 0;
    for ( i = start; i < start + n; i++ ) {
      if ( ((src[i] >> bit_pos) & 1) == 0 ) {
	  	nzeros[thread_index]++;      	
      }	
    }
    nones[thread_index] = n - nzeros[thread_index];
	
    /*Espera que todas as threads cheguem*/
    pthread_barrier_wait(&barrier);

    /* pega os índices de início */
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
	
    /* Espera que todas as threads cheguem */
    pthread_barrier_wait(&barrier);

    /* Move valores para as posições corretas. */
    for ( i = start; i < start + n; i++ ) {
      if ( ((src[i] >> bit_pos) & 1) == 0 ) {
	  	dest[index0++] = src[i];      	
      } else {
	  	dest[index1++] = src[i];      	
      }
    }
	
    /* Espera que todas as threads cheguem */
    pthread_barrier_wait(&barrier);
	
    /* troca os arrays. */
    tmp = src;
    src = dest;
    dest = tmp;
  }
}




void thread_work (int rank)
{
  int start, count, n;
  int index = rank;
  
  
  /* Obtem qual porção do vetor irá ser processada. */
  n = args[index].n / args[index].t; /* Número de elementos por qual essa thread está responsável */
  start = args[index].id * n;/* Thread está responsável por [start, start +n elementos */

  /* Executa radix sort. */
  radix_sort_thread (args[index].val, args[index].tmp, start, n,
  		     args[index].nzeros, args[index].nones, args[index].id, args[index].t);
}





void radix_sort (unsigned *val, int n, int t)
{
  unsigned *tmp;
  int *nzeros, *nones;
  int r, i;
  
  /* variáveis relacionadas a threads. */
  long thread;
  pthread_t* thread_handles;

  /* Aloca vetor temporário. */
  tmp = (unsigned *) malloc (n * sizeof(unsigned));
  if (!tmp) { fprintf (stderr, "Malloc failed.\n"); exit(1); }

  /* Aloca vetores de contadores. */
  nzeros = (int *) malloc (t * sizeof(int));
  if (!nzeros) { fprintf (stderr, "Malloc failed.\n"); exit(1); }
  nones = (int *) malloc (t * sizeof(int));
  if (!nones) { fprintf (stderr, "Malloc failed.\n"); exit(1); }

  /* Inicializa as threads handles e a barrier. */
  thread_handles = malloc (t * sizeof(pthread_t));
  pthread_barrier_init (&barrier, NULL, t);
  
  /* Inicializa argumentos da thread */
  for ( i = 0; i < t; i++ ) {
    args[i].id = i;
    args[i].val = val;
    args[i].tmp = tmp;
    args[i].n = n;
    args[i].nzeros = nzeros;
    args[i].nones = nones;
    args[i].t = t;
	
	/* Cria uma thread. */
	printf ("####### CRIANDO THREAD, id = %d\n", args[i].id);
    pthread_create (&thread_handles[i], NULL, thread_work, i);
  }
  
  printf ("####### THREADS DEVEM ESTAR FUNCIONANDO \n");
  
  /* Wait for threads to join and terminate. */
  for ( i = 0; i < t; i++ ) {
    pthread_join (thread_handles[i], NULL);
    printf ("####### THREAD %d DEVE ESTAR FINALIZADA \n", i);
  }

  /* Libera argumentos da threads. */
  pthread_barrier_destroy(&barrier);
  free (thread_handles);
  free (args);
 /*Printava o vetor antes de voltar para o main
  printf ("\n====== Before return to main: val array ======\n");
  print_array (val, n);
  printf ("\n====== Before return to main: tmp array ======\n");
  print_array (tmp, n);
*/
  /* Copia vetor se necessário. */
  if ( BITS % 2 == 1 ) {
    copy_array (val, tmp, n);  	
  }

  /* libera o vetor temporário e vetores contadores */
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
  double tempoGasto;
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
  clock_t begin = clock();
   radix_sort (num,novoTamanhoVetor,numThreads);
  clock_t end = clock();
  tempoGasto=(double)(end-begin)/ CLOCKS_PER_SEC;
  printf("Tamanho vetor: %d\n", novoTamanhoVetor);
  printf("Tempo gasto: %lf\n",tempoGasto);
  fs = fopen(argv[argc-1], "w");
  for (i=0; i<novoTamanhoVetor; i++){
      fprintf(fs, "%i\n",num[i]);
    }
  printf("Arquivo gerado com sucesso\n"); 
  
  pthread_exit(NULL);
  return 0;
}
