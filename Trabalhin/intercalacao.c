#include "intercalacao.h"
#include "funcionarios.h"
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void intercalacao_basico(char *nome_arquivo_saida, int num_p, Lista *nome_particoes) {

    int fim = 0; //variavel que controla fim do procedimento
    FILE *out; //declara ponteiro para arquivo

    //abre arquivo de saida para escrita
    if ((out = fopen(nome_arquivo_saida, "wb")) == NULL) {
        printf("Erro ao abrir arquivo de sa?da\n");
    } else {
        //cria vetor de particoes
        TVet v[num_p];

        //abre arquivos das particoes, colocando variavel de arquivo no campo f do vetor
        //e primeiro funcionario do arquivo no campo func do vetor
        for (int i=0; i < num_p; i++) {
            v[i].f = fopen(nome_particoes->nome, "rb");
            v[i].aux_p = 0;
            if (v[i].f != NULL) {
                fseek(v[i].f, v[i].aux_p * tamanho_registro(), SEEK_SET);
                TFunc *f = le_funcionario(v[i].f);
                if (f == NULL) {
                    //arquivo estava vazio
                    //coloca HIGH VALUE nessa posi??o do vetor
                    v[i].func = funcionario(INT_MAX, "","","",0);
                }
                else {
                    //conseguiu ler funcionario, coloca na posi??o atual do vetor
                    v[i].func = f;
                }
            }
            else {
                fim = 1;
            }
            nome_particoes = nome_particoes->prox;
        }

        int aux = 0;
        while (!(fim)) { //conseguiu abrir todos os arquivos
            int menor = INT_MAX;
            int pos_menor;
            //encontra o funcionario com menor chave no vetor
            for(int i = 0; i < num_p; i++){
                if(v[i].func->cod < menor){
                    menor = v[i].func->cod;
                    pos_menor = i;
                }
            }
            if (menor == INT_MAX) {
                fim = 1; //terminou processamento
            }
            else {
                //salva funcionario no arquivo de saída
                fseek(out, aux * tamanho_registro(), SEEK_SET);
                salva_funcionario(v[pos_menor].func, out);
                //atualiza posição pos_menor do vetor com pr?ximo funcionario do arquivo
                v[pos_menor].aux_p++;
                fseek(v[pos_menor].f, v[pos_menor].aux_p * tamanho_registro(), SEEK_SET);
                TFunc *f = le_funcionario(v[pos_menor].f);
                aux++;
                if (f == NULL) {
                    //arquivo estava vazio
                    //coloca HIGH VALUE nessa posiçao do vetor
                    v[pos_menor].func = funcionario(INT_MAX, "", "", "",0.0);
                }
                else {
                    v[pos_menor].func = f;
                }

            }
        }

        //fecha arquivos das partiÇões de entrada
        for(int i = 0; i < num_p; i++){
            fclose(v[i].f);
        //    free(v[i].func);
        }
        //fecha arquivo de saída
        fclose(out);
    }
}

static int ler_registro(FILE *arquivo, TFunc **funcionario) {
    TFunc *func = malloc(sizeof(*func));
    if (func == NULL) {
        return -1;
    }

    size_t lidos = fread(&func->cod, 1, sizeof(func->cod), arquivo);
    if (lidos == 0) {
        free(func);
        return feof(arquivo) ? 0 : -1;
    }

    if (lidos != sizeof(func->cod) ||
        fread(func->nome, 1, sizeof(func->nome), arquivo) != sizeof(func->nome) ||
        fread(func->cpf, 1, sizeof(func->cpf), arquivo) != sizeof(func->cpf) ||
        fread(func->data_nascimento, 1, sizeof(func->data_nascimento), arquivo) !=
            sizeof(func->data_nascimento) ||
        fread(&func->salario, 1, sizeof(func->salario), arquivo) != sizeof(func->salario)) {
        free(func);
        return -1;
    }

    *funcionario = func;
    return 1;
}

static int vencedor(int indice_a, int indice_b, TFunc **atuais) {
    if (indice_a == -1) {
        return indice_b;
    }
    if (indice_b == -1) {
        return indice_a;
    }

    if (atuais[indice_a]->cod < atuais[indice_b]->cod ||
        (atuais[indice_a]->cod == atuais[indice_b]->cod &&
         indice_a < indice_b)) {
        return indice_a;
    }
    return indice_b;
}

int intercalacao_arvore_de_vencedores(char *nome_arquivo_saida, int num_p,
                                     Lista *nome_particoes) {
    if (nome_arquivo_saida == NULL || num_p <= 0 || nome_particoes == NULL) {
        fprintf(stderr, "Parametros invalidos para intercalacao por arvore de vencedores.\n");
        return 0;
    }

    size_t folhas = 1;
    while (folhas < (size_t) num_p) {
        if (folhas > SIZE_MAX / 2) {
            fprintf(stderr, "Numero de particoes grande demais para a arvore de vencedores.\n");
            return 0;
        }
        folhas *= 2;
    }
    if (folhas > SIZE_MAX / 2) {
        fprintf(stderr, "Numero de particoes grande demais para a arvore de vencedores.\n");
        return 0;
    }

    FILE **arquivos = calloc((size_t) num_p, sizeof(*arquivos));
    TFunc **atuais = calloc((size_t) num_p, sizeof(*atuais));
    int *arvore = calloc(2 * folhas, sizeof(*arvore));
    if (arquivos == NULL || atuais == NULL || arvore == NULL) {
        fprintf(stderr, "Erro ao alocar memoria para a arvore de vencedores.\n");
        free(arquivos);
        free(atuais);
        free(arvore);
        return 0;
    }

    int sucesso = 1;
    Lista *particao = nome_particoes;
    for (int i = 0; i < num_p; i++) {
        if (particao == NULL || particao->nome == NULL) {
            fprintf(stderr, "A lista possui menos nomes de particoes que o informado.\n");
            sucesso = 0;
            break;
        }
        if (strcmp(nome_arquivo_saida, particao->nome) == 0) {
            fprintf(stderr, "O arquivo de saida nao pode ser uma das particoes de entrada.\n");
            sucesso = 0;
            break;
        }

        arquivos[i] = fopen(particao->nome, "rb");
        if (arquivos[i] == NULL) {
            fprintf(stderr, "Erro ao abrir a particao %s.\n", particao->nome);
            sucesso = 0;
            break;
        }

        int leitura = ler_registro(arquivos[i], &atuais[i]);
        if (leitura < 0) {
            fprintf(stderr, "Erro ao ler a particao %s.\n", particao->nome);
            sucesso = 0;
            break;
        }
        particao = particao->prox;
    }

    FILE *saida = NULL;
    if (sucesso) {
        saida = fopen(nome_arquivo_saida, "wb");
        if (saida == NULL) {
            fprintf(stderr, "Erro ao abrir o arquivo de saida %s.\n", nome_arquivo_saida);
            sucesso = 0;
        }
    }

    if (sucesso) {
        for (size_t i = 0; i < folhas; i++) {
            int indice = i < (size_t) num_p && atuais[i] != NULL ? (int) i : -1;
            arvore[folhas + i] = indice;
        }
        for (size_t i = folhas - 1; i > 0; i--) {
            arvore[i] = vencedor(arvore[2 * i], arvore[2 * i + 1], atuais);
        }

        while (arvore[1] != -1) {
            int ganhador = arvore[1];
            salva_funcionario(atuais[ganhador], saida);
            if (ferror(saida)) {
                fprintf(stderr, "Erro ao gravar no arquivo de saida %s.\n", nome_arquivo_saida);
                sucesso = 0;
                break;
            }

            free(atuais[ganhador]);
            atuais[ganhador] = NULL;
            int leitura = ler_registro(arquivos[ganhador], &atuais[ganhador]);
            if (leitura < 0) {
                fprintf(stderr, "Erro ao ler uma particao de entrada.\n");
                sucesso = 0;
                break;
            }

            size_t no = folhas + (size_t) ganhador;
            arvore[no] = leitura == 1 ? ganhador : -1;
            while (no > 1) {
                no /= 2;
                arvore[no] = vencedor(arvore[2 * no], arvore[2 * no + 1], atuais);
            }
        }
    }

    for (int i = 0; i < num_p; i++) {
        free(atuais[i]);
        if (arquivos[i] != NULL && fclose(arquivos[i]) != 0) {
            fprintf(stderr, "Erro ao fechar uma particao de entrada.\n");
            sucesso = 0;
        }
    }
    if (saida != NULL && fclose(saida) != 0) {
        fprintf(stderr, "Erro ao fechar o arquivo de saida %s.\n", nome_arquivo_saida);
        sucesso = 0;
    }

    free(arquivos);
    free(atuais);
    free(arvore);
    return sucesso;
}
