#ifndef INTERCALACAO_H
#define INTERCALACAO_H

#include "funcionarios.h"
#include "lista.h"

/*estrutura para armazenar o vetor de partições*/
typedef struct vetor{
    TFunc *func;
    int aux_p;
    FILE *f;
}TVet;

/*Algoritmo de intercalacao de particoes básico, recebe
o nome do arquivo de saída, o número de partições a serem intercaladas
e uma lista com os nomes das partições de entrada a serem lidas.*/
void intercalacao_basico(char *nome_arquivo_saida, int num_p, Lista *nome_particoes);

/* Intercala particoes ordenadas por codigo usando uma arvore de vencedores.
   Retorna 1 em caso de sucesso e 0 se houver erro. */
int intercalacao_arvore_de_vencedores(char *nome_arquivo_saida, int num_p,
                                     Lista *nome_particoes);

#endif
