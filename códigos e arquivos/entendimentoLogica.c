#include <stdio.h>
#include <stdlib.h>
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
int main (){
int v1[5]={83,71,94,29,43};
int v2[5]={34,10,79,89,41};
int v3[5]={88,46,95,97,47};
int v4[15];
int c,c1,c2;
printf("Resultado esperado: 10,29,34,41,43,46,47,71,79,83,88,89,94,95,97\n");

for (c=0;c<5;c++){
v4[c]=v1[c];
}
for (c=5;c<10;c++){
v4[c]=v2[c-5];
}
for (c=10;c<15;c++){
v4[c]=v3[c-10];
}
radixsort(v4,15);
for (c=0;c<15;c++){
printf("%i\n",v4[c]);
 }
}

