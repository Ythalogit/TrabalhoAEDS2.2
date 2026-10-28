#ifndef PARTICOES_H
#define PARTICOES_H

#include <stdio.h>

#include "lista.h"

/* Algoritmo de geracao de particoes por Classificacao Interna, recebe como parâmetro
o arquivo de dados de entrada, a lista contendo os nomes dos arquivos de saída das partições,
o númer de elementos M a ser armazenado em cada partição e o número de registro total do arquivo.*/
void classificacao_interna(FILE *arq, Lista *nome_arquivos_saida, int M, int nFunc);

/*Gera particoes ordenadas pelo metodo de selecao natural. Recebe o arquivo de
entrada, a lista de nomes dos arquivos de saida, a capacidade da memoria principal
(M), o numero de registros de entrada e a capacidade do reservatorio de leitura (n).*/
void selecao_natural(FILE *arq, Lista *nome_arquivos_saida, int M, int nFunc, int n);


#endif
