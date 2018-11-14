#include <stdio.h>
#include <stdlib.h>

int main (int argc, char *argv[])
{
  char caracter;
  int c, n;
  int x, y, z;
  int *vetor = NULL;
  FILE *arquivo;
 
  
  printf ("\nInforme a quantidade de elementos do conjunto: ");
  scanf  ("%d", &n);
  
  vetor = (int*) malloc (n * sizeof (int) );
  
 
 



srand ( time(NULL) );
  
  for (c = 1; c <= n; c++)
  {
    fprintf (arquivo, "%i ", ( (int) rand() % 1001));
      fprintf (arquivo, "\n");
  }
  
  fclose (arquivo);  
  system ("clear");
    
  return 0;
}
