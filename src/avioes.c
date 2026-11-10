/*
 * avioes.c
 * CRUD de avioes sobre uma LISTA SIMPLESMENTE ENCADEADA com alocacao dinamica.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "aerocontrol.h"

static void copiar(char *dest, const char *orig, size_t tam)
{
    strncpy(dest, orig, tam - 1);
    dest[tam - 1] = '\0';
}

Aviao *aviaoCriar(int id, const char *nome, const char *modelo,
                  const char *fabricante, const char *codigoVoo,
                  int maxPassageiros, float velocidadeMaxima)
{
    Aviao *a = (Aviao *)malloc(sizeof(Aviao));

    if (a == NULL)
        return NULL;
    a->id = id;
    copiar(a->nome, nome, sizeof(a->nome));
    copiar(a->modelo, modelo, sizeof(a->modelo));
    copiar(a->fabricante, fabricante, sizeof(a->fabricante));
    copiar(a->codigoVoo, codigoVoo, sizeof(a->codigoVoo));
    a->maxPassageiros = maxPassageiros;
    a->velocidadeMaxima = velocidadeMaxima;
    a->proximo = NULL;
    return a;
}

/* Insere no final da lista para manter a ordem de cadastro. */
int aviaoInserir(Sistema *s, Aviao *novo)
{
    Aviao *atual;

    novo->proximo = NULL;
    if (s->avioes == NULL) {
        s->avioes = novo;
    } else {
        atual = s->avioes;
        while (atual->proximo != NULL)
            atual = atual->proximo;
        atual->proximo = novo;
    }
    s->aeroporto.totalAvioes++;
    return novo->id;
}

Aviao *aviaoBuscar(const Sistema *s, int id)
{
    Aviao *atual = s->avioes;

    while (atual != NULL) {
        if (atual->id == id)
            return atual;
        atual = atual->proximo;
    }
    return NULL;
}

int aviaoAtualizar(Sistema *s, int id, const char *nome, const char *modelo,
                   const char *fabricante, const char *codigoVoo,
                   int maxPassageiros, float velocidadeMaxima)
{
    Aviao *a = aviaoBuscar(s, id);

    if (a == NULL)
        return 0;
    copiar(a->nome, nome, sizeof(a->nome));
    copiar(a->modelo, modelo, sizeof(a->modelo));
    copiar(a->fabricante, fabricante, sizeof(a->fabricante));
    copiar(a->codigoVoo, codigoVoo, sizeof(a->codigoVoo));
    a->maxPassageiros = maxPassageiros;
    a->velocidadeMaxima = velocidadeMaxima;
    return 1;
}

int aviaoRemover(Sistema *s, int id)
{
    Aviao *atual = s->avioes;
    Aviao *anterior = NULL;

    while (atual != NULL && atual->id != id) {
        anterior = atual;
        atual = atual->proximo;
    }
    if (atual == NULL)
        return 0;

    /* Evita ponteiro pendente: retira o aviao de qualquer fila de decolagem. */
    controleRemoverAviaoDasFilas(s, atual);

    if (anterior == NULL)
        s->avioes = atual->proximo;
    else
        anterior->proximo = atual->proximo;
    free(atual);
    s->aeroporto.totalAvioes--;
    return 1;
}

void aviaoExibir(const Aviao *a)
{
    printf(FMT_CAMPO "%d\n",        "ID", a->id);
    printf(FMT_CAMPO "%s\n",        "Nome", a->nome);
    printf(FMT_CAMPO "%s\n",        "Modelo", a->modelo);
    printf(FMT_CAMPO "%s\n",        "Fabricante", a->fabricante);
    printf(FMT_CAMPO "%s\n",        "Codigo do voo", a->codigoVoo);
    printf(FMT_CAMPO "%d\n",        "Maximo de passageiros", a->maxPassageiros);
    printf(FMT_CAMPO "%.2f km/h\n", "Velocidade maxima", a->velocidadeMaxima);
}

/* Tabela com exatamente LARGURA_TELA (70) colunas. */
void avioesListar(const Sistema *s)
{
    const Aviao *a = s->avioes;

    if (a == NULL) {
        printf(" Nenhum aviao cadastrado.\n");
        return;
    }
    printf(" %-4s %-16s %-12s %-10s %-8s %5s %8s\n",
           "ID", "NOME", "MODELO", "FABRICANTE", "VOO", "PAX", "KM/H");
    for (; a != NULL; a = a->proximo)
        printf(" %-4d %-16.16s %-12.12s %-10.10s %-8.8s %5d %8.1f\n",
               a->id, a->nome, a->modelo, a->fabricante, a->codigoVoo,
               a->maxPassageiros, a->velocidadeMaxima);
    printf("\n Total: %d aviao(oes) cadastrado(s).\n", s->aeroporto.totalAvioes);
}

/* Retorna 1 se 'a' deve vir ESTRITAMENTE antes de 'b' no criterio dado.
 * Empates retornam 0, o que garante a estabilidade da ordenacao. */
static int vemAntes(const Aviao *a, const Aviao *b, CriterioOrdenacao criterio)
{
    switch (criterio) {
    case ORDENAR_POR_VELOCIDADE:  return a->velocidadeMaxima > b->velocidadeMaxima;
    case ORDENAR_POR_PASSAGEIROS: return a->maxPassageiros > b->maxPassageiros;
    case ORDENAR_POR_ID:
    default:                      return a->id < b->id;
    }
}

/* Otimização Teórica: Algoritmo Insertion Sort aplicado para ordenação de nós em Lista Encadeada */
/*
 * Retira um no por vez da lista original e o insere na posicao correta de
 * uma lista "ordenada" que cresce a cada passo. Apenas os ponteiros
 * 'proximo' sao religados: nenhum no e alocado, copiado ou liberado, entao
 * os enderecos dos avioes nao mudam e as filas de decolagem (que guardam
 * Aviao*) continuam validas. Estavel: em empate, o no novo vai DEPOIS dos
 * iguais ja inseridos. Custo: O(n^2) comparacoes no pior caso, O(1) memoria.
 */
void avioesOrdenar(Sistema *s, CriterioOrdenacao criterio)
{
    Aviao *ordenada = NULL;          /* cabeca da lista ja ordenada   */
    Aviao *restante = s->avioes;     /* nos ainda nao processados     */
    Aviao *no, *atual;

    while (restante != NULL) {
        no = restante;               /* desengata o primeiro no restante */
        restante = restante->proximo;

        if (ordenada == NULL || vemAntes(no, ordenada, criterio)) {
            no->proximo = ordenada;  /* insere na cabeca */
            ordenada = no;
        } else {
            atual = ordenada;        /* avanca enquanto o proximo nao vier depois do no */
            while (atual->proximo != NULL && !vemAntes(no, atual->proximo, criterio))
                atual = atual->proximo;
            no->proximo = atual->proximo;
            atual->proximo = no;
        }
    }
    s->avioes = ordenada;
}

/* Lista os avioes no criterio escolhido e depois restaura a ordem de
 * cadastro (por ID), para que a ordenacao nao altere o estado do sistema. */
void avioesListarOrdenado(Sistema *s, CriterioOrdenacao criterio)
{
    if (criterio != ORDENAR_POR_ID)
        avioesOrdenar(s, criterio);
    avioesListar(s);
    if (criterio != ORDENAR_POR_ID)
        avioesOrdenar(s, ORDENAR_POR_ID);
}

void avioesLiberar(Sistema *s)
{
    Aviao *atual = s->avioes;
    Aviao *prox;

    while (atual != NULL) {
        prox = atual->proximo;
        free(atual);
        atual = prox;
    }
    s->avioes = NULL;
    s->aeroporto.totalAvioes = 0;
}
