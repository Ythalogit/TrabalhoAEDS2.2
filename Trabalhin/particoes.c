
#include "funcionarios.h"
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>

#include "particoes.h"

static TFunc *proximo_funcionario(FILE *arq, int *lidos, int total, int *erro_leitura) {
    if (*lidos >= total || *erro_leitura) {
        return NULL;
    }

    TFunc *func = le_funcionario(arq);
    if (func == NULL) {
        *erro_leitura = 1;
        return NULL;
    }

    (*lidos)++;
    return func;
}

static void preencher_reservatorio(TFunc **reservatorio, int capacidade, int inicio,
                                  int *quantidade, FILE *arq, int *lidos, int total,
                                  int *erro_leitura) {
    while (*quantidade < capacidade) {
        TFunc *func = proximo_funcionario(arq, lidos, total, erro_leitura);
        if (func == NULL) {
            break;
        }

        int posicao = (inicio + *quantidade) % capacidade;
        reservatorio[posicao] = func;
        (*quantidade)++;
    }
}

static TFunc *retirar_reservatorio(TFunc **reservatorio, int capacidade, int *inicio,
                                  int *quantidade, FILE *arq, int *lidos, int total,
                                  int *erro_leitura) {
    if (capacidade == 0) {
        return proximo_funcionario(arq, lidos, total, erro_leitura);
    }

    if (*quantidade == 0) {
        return NULL;
    }

    TFunc *func = reservatorio[*inicio];
    reservatorio[*inicio] = NULL;
    *inicio = (*inicio + 1) % capacidade;
    (*quantidade)--;

    preencher_reservatorio(reservatorio, capacidade, *inicio, quantidade, arq, lidos,
                           total, erro_leitura);
    return func;
}

void classificacao_interna(FILE *arq, Lista *nome_arquivos_saida, int M, int nFunc) {
    rewind(arq); //posiciona cursor no inicio do arquivo


    int reg = 0;

    while (reg != nFunc) {
        //le o arquivo e coloca no vetor
        TFunc *v[M];
        int i = 0;
        while (!feof(arq)) {
            fseek(arq, (reg) * tamanho_registro(), SEEK_SET);
            v[i] = le_funcionario(arq);
       //     imprime_funcionario(v[i]);
            i++;
            reg++;
            if(i>=M) break;
        }

        //ajusta tamanho M caso arquivo de entrada tenha terminado antes do vetor
        if (i != M) {
            M = i;
        }

        //faz ordenacao
        for (int j = 1; j < M; j++) {
            TFunc *f = v[j];
            i = j - 1;
            while ((i >= 0) && (v[i]->cod > f->cod)) {
                v[i + 1] = v[i];
                i = i - 1;
            }
            v[i + 1] = f;
        }

        //cria arquivo de particao e faz gravacao
        char *nome_particao = nome_arquivos_saida->nome;
        nome_arquivos_saida = nome_arquivos_saida->prox;
        printf("\n%s\n", nome_particao);
        FILE *p;
        if ((p = fopen(nome_particao, "wb+")) == NULL) {
            printf("Erro criar arquivo de saida\n");
        } else {
            for (int i = 0; i < M; i++) {
                fseek(p, (i) * tamanho_registro(), SEEK_SET);
                salva_funcionario(v[i], p);
                imprime_funcionario(v[i]);
            }
            fclose(p);
        }
        for(int jj = 0; jj<M; jj++)
            free(v[jj]);
    }
}

void selecao_natural(FILE *arq, Lista *nome_arquivos_saida, int M, int nFunc, int n) {
    if (arq == NULL || M <= 0 || nFunc < 0 || n < 0) {
        fprintf(stderr, "Parametros invalidos para selecao natural.\n");
        return;
    }

    if (nFunc == 0) {
        return;
    }

    TFunc **memoria = calloc((size_t) M, sizeof(*memoria));
    int *congelado = calloc((size_t) M, sizeof(*congelado));
    TFunc **reservatorio = n > 0 ? calloc((size_t) n, sizeof(*reservatorio)) : NULL;

    if (memoria == NULL || congelado == NULL || (n > 0 && reservatorio == NULL)) {
        fprintf(stderr, "Erro ao alocar memoria para selecao natural.\n");
        free(memoria);
        free(congelado);
        free(reservatorio);
        return;
    }

    rewind(arq);

    int lidos = 0;
    int erro_leitura = 0;
    int inicio_reservatorio = 0;
    int quantidade_reservatorio = 0;
    int quantidade_memoria = 0;

    while (quantidade_memoria < M) {
        TFunc *func = proximo_funcionario(arq, &lidos, nFunc, &erro_leitura);
        if (func == NULL) {
            break;
        }
        memoria[quantidade_memoria++] = func;
    }

    if (n > 0) {
        preencher_reservatorio(reservatorio, n, inicio_reservatorio,
                               &quantidade_reservatorio, arq, &lidos, nFunc,
                               &erro_leitura);
    }

    Lista *proxima_particao = nome_arquivos_saida;
    FILE *saida = NULL;
    int terminou = 0;
    int registros_gravados = 0;

    while (!terminou && registros_gravados < nFunc) {
        int ativos = 0;
        for (int i = 0; i < M; i++) {
            if (memoria[i] != NULL && !congelado[i]) {
                ativos++;
            }
        }

        if (ativos == 0) {
            if (saida != NULL) {
                fclose(saida);
                saida = NULL;
            }

            for (int i = 0; i < M; i++) {
                if (memoria[i] != NULL) {
                    congelado[i] = 0;
                }
            }

            for (int i = 0; i < M; i++) {
                if (memoria[i] == NULL) {
                    memoria[i] = retirar_reservatorio(
                        reservatorio, n, &inicio_reservatorio,
                        &quantidade_reservatorio, arq, &lidos, nFunc,
                        &erro_leitura);
                }
            }

            for (int i = 0; i < M; i++) {
                if (memoria[i] != NULL && !congelado[i]) {
                    ativos++;
                }
            }

            if (ativos == 0) {
                terminou = 1;
                continue;
            }
        }

        if (saida == NULL) {
            if (proxima_particao == NULL) {
                fprintf(stderr, "A lista de arquivos de saida nao possui particoes suficientes.\n");
                break;
            }

            saida = fopen(proxima_particao->nome, "wb");
            if (saida == NULL) {
                fprintf(stderr, "Erro ao criar a particao %s.\n", proxima_particao->nome);
                break;
            }
            proxima_particao = proxima_particao->prox;
        }

        int menor = -1;
        for (int i = 0; i < M; i++) {
            if (memoria[i] != NULL && !congelado[i] &&
                (menor == -1 || memoria[i]->cod < memoria[menor]->cod)) {
                menor = i;
            }
        }

        if (menor == -1) {
            continue;
        }

        int ultima_chave = memoria[menor]->cod;
        salva_funcionario(memoria[menor], saida);
        registros_gravados++;
        free(memoria[menor]);

        memoria[menor] = retirar_reservatorio(
            reservatorio, n, &inicio_reservatorio, &quantidade_reservatorio,
            arq, &lidos, nFunc, &erro_leitura);

        if (memoria[menor] != NULL) {
            congelado[menor] = memoria[menor]->cod < ultima_chave;
        } else {
            congelado[menor] = 0;
        }
    }

    if (saida != NULL) {
        fclose(saida);
    }

    if (erro_leitura) {
        fprintf(stderr, "O arquivo de entrada terminou antes dos %d registros informados.\n",
                nFunc);
    }

    for (int i = 0; i < M; i++) {
        free(memoria[i]);
    }
    for (int i = 0; i < n; i++) {
        free(reservatorio[i]);
    }
    free(memoria);
    free(congelado);
    free(reservatorio);
}
