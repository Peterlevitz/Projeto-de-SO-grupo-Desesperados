#include <stdio.h>
#include <stdlib.h>
#define TAMANHOMAX 100

void radixsort(int vetor[], int tamanho) {
    int i;
    int *b;
    int maior = vetor[0];
    int exp = 1;

    b = (int *)calloc(tamanho, sizeof(int));

    for (i = 0; i < tamanho; i++) {
        if (vetor[i] > maior)
    	    maior = vetor[i];
    }

    while (maior/exp > 0) {
        int bucket[10] = { 0 };
    	for (i = 0; i < tamanho; i++)
    	    bucket[(vetor[i] / exp) % 10]++;
    	for (i = 1; i < 10; i++)
    	    bucket[i] += bucket[i - 1];
    	for (i = tamanho - 1; i >= 0; i--)
    	    b[--bucket[(vetor[i] / exp) % 10]] = vetor[i];
    	for (i = 0; i < tamanho; i++)
    	    vetor[i] = b[i];
    	exp *= 10;
    }

    free(b);
}






int main(int argc, char *argv[])
{
  FILE *fs;
  int *num;
  int tamanhoVet=1;
  int novoTamanhoVetor;
  int c,c1;
  int teste=argc;
  register int i=0;

  if (argc<4)
  { 
    fprintf(stderr,"uso:\n\t %s num de threads arquivos de entrada arquivo de saida\n",argv[0]);
    exit(0);
  }
  num = calloc(TAMANHOMAX,sizeof(int));
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
       num = realloc(num, TAMANHOMAX*tamanhoVet*sizeof(int));
       fscanf(fs,"%i", &num[i]);
       i++;
    }
  }
  
}
  num = realloc(num, (i-1)*sizeof(int));
  novoTamanhoVetor = i-1;
  radixsort (num,novoTamanhoVetor);
  printf("Tamanho vetor: %d\n", novoTamanhoVetor);
  
  fs = fopen(argv[argc-1], "w");
  for (i=0; i<novoTamanhoVetor; i++){
      fprintf(fs, "%i\n",num[i]);
    }
   
  

  return 0;
}
