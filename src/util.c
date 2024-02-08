#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*
int main()
{
    int dec = 16843009;
    int bits = 0; 
    char *bin = dtob(dec, &bits);
    int d = btod(bin, bits);

    printf("----------\n");
    for(int u = 0; u < bits; u++)
        printf("%d", bin[u]);
    printf("\n----------\n");
    printf("dec: %d\n", d); 
}
*/

char * dtob(int dec, int *bit_num)
{
    int size = 8;
    char *bin;
    bin = calloc(size + 1, sizeof(char));
     
    int i = 0;
    for(i = 0; dec > 0; i++) {
        if(i == size){
            bin = realloc(bin, (size += 8) + 1);
            memset(bin + (size-8), 0, 8);
        }
        bin[i] = dec % 2;
        dec /= 2;
    }
    bin[size] = '\0';

    //reversing the order
    int k = size - 1;
    int j = 0;
    char temp;
    
    while(k > j){
        temp = bin[k]; 
        bin[k] = bin[j];
        bin[j] = temp;
        k--;
        j++;
    }

    *bit_num = size;
    return bin;
}

int btod(char *bin, int bit_num)
{
    int dec = 0;
    for(int i = 0; i < bit_num; i++)
        dec = bin[i]? dec*2 + 1 :  dec*2;
    
    return dec;
}
