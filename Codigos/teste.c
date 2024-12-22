#include <stdio.h>

int main(){
    float soma = 0, a = 0, b = 0;
    printf("Digite os valores da soma:");
    scanf("%f%f",&a,&b);

    soma = a + b;

    printf("Resultado da soma = %.2f",soma);

    return 0;
}