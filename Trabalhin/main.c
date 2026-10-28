#include "particoes.h"
#include "funcionarios.h"
#include "lista.h"
#include <stdio.h>
#include <stdlib.h>

FILE *base = fopen("output\\funcionarios.dat", "wb");
FILE *indice = fopen("output\\indice.dat", "wb");
FILE *indiceNaoOrdenado = fopen("output\\indice_nao_ordenado.dat", "wb");

if (base == NULL || indice == NULL || indiceNaoOrdenado == NULL) {
    perror("Erro ao criar arquivos de teste");
    if (base != NULL) fclose(base);
    if (indice != NULL) fclose(indice);
    if (indiceNaoOrdenado != NULL) fclose(indiceNaoOrdenado);
    return 1;
}

cria_base_dados_funcionarios_teste(base, indice, indiceNaoOrdenado, 20);

fclose(base);
fclose(indice);
fclose(indiceNaoOrdenado);

int main(void) {
    FILE *entrada = fopen("output\\funcionarios.dat", "rb");
    if (entrada == NULL) {
        perror("Erro ao abrir arquivo de funcionarios");
        return 1;
    }

    fseek(entrada, 0, SEEK_END);
    long bytes = ftell(entrada);
    int tamanho = tamanho_registro();

    if (bytes < 0 || tamanho <= 0 || bytes % tamanho != 0) {
        fprintf(stderr, "Arquivo de funcionarios invalido.\n");
        fclose(entrada);
        return 1;
    }

    int nFunc = (int)(bytes / tamanho);
    rewind(entrada);

    if (nFunc == 0) {
        printf("O arquivo nao possui funcionarios.\n");
        fclose(entrada);
        return 0;
    }

    Lista *particoes = NULL;

    /* Cria espaço para até nFunc partições. */
    for (int i = nFunc; i >= 1; i--) {
        char nome[80];
        snprintf(nome, sizeof(nome), "output\\particao_%03d.dat", i);

        char *nome_lista = cria_str(nome);
        if (nome_lista == NULL) {
            fprintf(stderr, "Erro ao alocar nome da particao.\n");
            libera(particoes);
            fclose(entrada);
            return 1;
        }

        Lista *nova = cria(nome_lista, particoes);
        if (nova == NULL) {
            free(nome_lista);
            libera(particoes);
            fclose(entrada);
            return 1;
        }
        particoes = nova;
    }

    int M = 3; /* capacidade da memória principal */
    int n = 2; /* capacidade do reservatório */

    selecao_natural(entrada, particoes, M, nFunc, n);

    fclose(entrada);
    libera(particoes);

    printf("Geracao das particoes concluida.\n");
    return 0;
}