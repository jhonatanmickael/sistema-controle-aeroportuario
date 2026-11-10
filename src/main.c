/*
 * AeroControl - main.c (Versao Enxuta com Limpeza de Tela)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "aerocontrol.h"

static Sistema sistema;

// Limpa o terminal
static void limparTela(void)
{
    printf("\033[H\033[J");
    fflush(stdout);
}

// Ler texto
static void lerTextoSimples(const char *prompt, char *dest, size_t tam)
{
    printf("%s", prompt);
    fflush(stdout);
    if (fgets(dest, (int)tam, stdin) != NULL) {
        size_t len = strlen(dest);
        if (len > 0 && dest[len - 1] == '\n')
            dest[len - 1] = '\0';
    }
}

// Ler inteiros
static int lerIntSimples(const char *prompt)
{
    int val;
    char buf[32];
    printf("%s", prompt);
    fflush(stdout);
    if (fgets(buf, sizeof(buf), stdin) != NULL)
        sscanf(buf, "%d", &val);
    return val;
}

// Ler float
static float lerFloatSimples(const char *prompt)
{
    float val;
    char buf[32];
    printf("%s", prompt);
    fflush(stdout);
    if (fgets(buf, sizeof(buf), stdin) != NULL)
        sscanf(buf, "%f", &val);
    return val;
}

// Pausa o terminal ate dar ENTER
static void pausar(void)
{
    char buf[8];
    printf("\nPressione ENTER para continuar...");
    fgets(buf, sizeof(buf), stdin);
}

// Interface de cadastrar aeroporto
static void opCadastrarAeroporto(void)
{
    char nome[TAM_NOME], iata[TAM_IATA], tipo[TAM_TIPO];
    long cap;

    limparTela();
    printf("--- CADASTRAR AEROPORTO ---\n\n");
    lerTextoSimples("Nome: ", nome, sizeof(nome));
    lerTextoSimples("Codigo IATA (3 letras): ", iata, sizeof(iata));
    lerTextoSimples("Tipo: ", tipo, sizeof(tipo));
    cap = (long)lerIntSimples("Capacidade anual: ");

    aeroportoCadastrar(&sistema, nome, iata, tipo, cap);
    printf("\nAeroporto cadastrado/atualizado com sucesso.\n");
}

// Interface do menu do aeroporto
static void menuAeroporto(void)
{
    int op;
    do {
        limparTela();
        printf("=== 1. AEROPORTO ===\n\n");
        printf("1. Cadastrar Dados\n");
        printf("2. Visualizar Informacoes\n");
        printf("0. Voltar\n\n");
        op = lerIntSimples("Opcao: ");

        if (op == 1) {
            opCadastrarAeroporto();
            pausar();
        } else if (op == 2) {
            limparTela();
            printf("--- INFORMACOES DO AEROPORTO ---\n\n");
            aeroportoExibir(&sistema);
            pausar();
        }
    } while (op != 0);
}

// interface da tela de cadastrar avião
static void opCadastrarAviao(void)
{
    char nome[TAM_NOME], modelo[TAM_MODELO], fab[TAM_FABRIC], voo[TAM_VOO];
    int maxPax;
    float vel;
    Aviao *novo;

    limparTela();
    printf("--- CADASTRAR AVIAO ---\n\n");
    lerTextoSimples("Nome: ", nome, sizeof(nome));
    lerTextoSimples("Modelo: ", modelo, sizeof(modelo));
    lerTextoSimples("Fabricante: ", fab, sizeof(fab));
    lerTextoSimples("Codigo do Voo: ", voo, sizeof(voo));
    maxPax = lerIntSimples("Maximo de passageiros: ");
    vel = lerFloatSimples("Velocidade maxima (km/h): ");

    novo = aviaoCriar(sistema.proximoIdAviao, nome, modelo, fab, voo, maxPax, vel);
    if (novo != NULL) {
        sistema.proximoIdAviao++;
        printf("\nAviao cadastrado com ID %d.\n", aviaoInserir(&sistema, novo));
    }
}

// interface da tela de menu de aviões
static void menuAvioes(void)
{
    int op;
    do {
        limparTela();
        printf("=== 2. AVIOES ===\n\n");
        printf("1. Cadastrar Aviao\n");
        printf("2. Listar Avioes (por ID, Velocidade ou PAX)\n");
        printf("3. Recuperar por ID\n");
        printf("4. Remover Aviao\n");
        printf("0. Voltar\n\n");
        op = lerIntSimples("Opcao: ");

        if (op == 1) {
            opCadastrarAviao();
            pausar();
        } else if (op == 2) {
            limparTela();
            printf("--- LISTA DE AVIOES ---\n\n");
            int crit = lerIntSimples("Ordenar por (1-ID, 2-Velocidade, 3-PAX): ");
            limparTela();
            avioesListarOrdenado(&sistema, (CriterioOrdenacao)crit);
            pausar();
        } else if (op == 3) {
            limparTela();
            printf("--- BUSCAR AVIAO POR ID ---\n\n");
            int id = lerIntSimples("ID do aviao: ");
            Aviao *a = aviaoBuscar(&sistema, id);
            printf("\n");
            if (a) aviaoExibir(a); else printf("Aviao nao encontrado.\n");
            pausar();
        } else if (op == 4) {
            limparTela();
            printf("--- REMOVER AVIAO ---\n\n");
            int id = lerIntSimples("ID do aviao a remover: ");
            printf("\n");
            if (aviaoRemover(&sistema, id)) printf("Aviao removido com sucesso.\n");
            else printf("Aviao nao encontrado.\n");
            pausar();
        }
    } while (op != 0);
}

// interface da tela de menu de pistas
static void menuPistas(void)
{
    int op;
    do {
        limparTela();
        printf("=== 3. PISTAS ===\n\n");
        printf("1. Cadastrar Pista\n");
        printf("2. Listar Pistas\n");
        printf("3. Buscar Pista por ID (Pesquisa Binaria)\n");
        printf("4. Remover Pista\n");
        printf("0. Voltar\n\n");
        op = lerIntSimples("Opcao: ");

        if (op == 1) {
            char nome[TAM_NOME], sup[TAM_SUPERF];
            limparTela();
            printf("--- CADASTRAR PISTA ---\n\n");
            lerTextoSimples("Nome: ", nome, sizeof(nome));
            float comp = lerFloatSimples("Comprimento (m): ");
            lerTextoSimples("Superficie: ", sup, sizeof(sup));
            pistaInserir(&sistema, nome, comp, sup);
            printf("\nPista cadastrada com sucesso.\n");
            pausar();
        } else if (op == 2) {
            limparTela();
            printf("--- LISTA DE PISTAS ---\n\n");
            pistasListar(&sistema);
            pausar();
        } else if (op == 3) {
            limparTela();
            printf("--- BUSCAR PISTA POR ID ---\n\n");
            int id = lerIntSimples("ID da pista: ");
            Pista *p = pistaBuscar(&sistema, id);
            printf("\n");
            if (p) pistaExibir(p); else printf("Pista nao encontrada.\n");
            pausar();
        } else if (op == 4) {
            limparTela();
            printf("--- REMOVER PISTA ---\n\n");
            int id = lerIntSimples("ID da pista a remover: ");
            printf("\n");
            if (pistaRemover(&sistema, id)) printf("Pista removida com sucesso.\n");
            else printf("Pista nao encontrada.\n");
            pausar();
        }
    } while (op != 0);
}

// interface da tela de menu controle
static void menuControle(void)
{
    int op;
    do {
        limparTela();
        printf("=== 4. CONTROLE AEREO ===\n\n");
        printf("1. Adicionar Aviao a Fila de Pista\n");
        printf("2. Listar Fila de Pista\n");
        printf("3. Autorizar Decolagem (Desenfileirar)\n");
        printf("0. Voltar\n\n");
        op = lerIntSimples("Opcao: ");

        if (op == 1) {
            limparTela();
            printf("--- ADICIONAR A FILA DE DECOLAGEM ---\n\n");
            int idPista = lerIntSimples("ID da pista: ");
            int idAviao = lerIntSimples("ID do aviao: ");
            Pista *p = pistaBuscar(&sistema, idPista);
            Aviao *a = aviaoBuscar(&sistema, idAviao);
            printf("\n");
            if (p && a) {
                filaEnfileirar(&p->filaEspera, a);
                printf("Aviao '%s' adicionado a fila da pista '%s'.\n", a->nome, p->nome);
            } else printf("Pista ou aviao nao encontrado.\n");
            pausar();
        } else if (op == 2) {
            limparTela();
            printf("--- FILA DE DECOLAGEM ---\n\n");
            int idPista = lerIntSimples("ID da pista: ");
            Pista *p = pistaBuscar(&sistema, idPista);
            printf("\n");
            if (p) controleListarFila(p); else printf("Pista nao encontrada.\n");
            pausar();
        } else if (op == 3) {
            limparTela();
            printf("--- AUTORIZAR DECOLAGEM ---\n\n");
            int idPista = lerIntSimples("ID da pista: ");
            Pista *p = pistaBuscar(&sistema, idPista);
            printf("\n");
            if (p) {
                Aviao *a = filaDesenfileirar(&p->filaEspera);
                if (a) printf("Decolagem AUTORIZADA para o voo %s (%s)!\n", a->codigoVoo, a->nome);
                else printf("Fila de decolagem vazia.\n");
            } else printf("Pista nao encontrada.\n");
            pausar();
        }
    } while (op != 0);
}

int main(void)
{
    int op;
    sistemaInicializar(&sistema);

    do {
        limparTela();
        printf("=========================================\n");
        printf("   AEROCONTROL - SISTEMA AEROPORTUARIO   \n");
        printf("=========================================\n\n");
        printf("1. Aeroporto\n");
        printf("2. Avioes\n");
        printf("3. Pistas\n");
        printf("4. Controle Aereo\n");
        printf("0. Sair\n\n");
        op = lerIntSimples("Opcao: ");

        switch (op) {
        case 1: menuAeroporto(); break;
        case 2: menuAvioes(); break;
        case 3: menuPistas(); break;
        case 4: menuControle(); break;
        case 0:
            sistemaLiberar(&sistema);
            limparTela();
            printf("Memoria liberada. Encerramento concluido.\n");
            break;
        default: break;
        }
    } while (op != 0);

    return EXIT_SUCCESS;
}