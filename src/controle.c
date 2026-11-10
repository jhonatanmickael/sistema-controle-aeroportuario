/*
 * controle.c
 * Controle aereo: FILA FIFO (FilaDecolagem) por pista.
 * Enfileira no fim, desenfileira no inicio -> O(1) em ambas operacoes.
 */
#include <stdio.h>
#include <stdlib.h>
#include "aerocontrol.h"

void filaInicializar(FilaDecolagem *f)
{
    f->inicio = NULL;
    f->fim = NULL;
    f->tamanho = 0;
}

int filaVazia(const FilaDecolagem *f)
{
    return f->inicio == NULL;
}

int filaEnfileirar(FilaDecolagem *f, Aviao *a)
{
    NoFila *no = (NoFila *)malloc(sizeof(NoFila));

    if (no == NULL)
        return 0;
    no->aviao = a;
    no->proximo = NULL;
    if (f->fim == NULL)
        f->inicio = no;
    else
        f->fim->proximo = no;
    f->fim = no;
    f->tamanho++;
    return 1;
}

Aviao *filaDesenfileirar(FilaDecolagem *f)
{
    NoFila *no;
    Aviao *a;

    if (filaVazia(f))
        return NULL;
    no = f->inicio;
    a = no->aviao;
    f->inicio = no->proximo;
    if (f->inicio == NULL)
        f->fim = NULL;
    free(no);
    f->tamanho--;
    return a;
}

Aviao *filaPrimeiro(const FilaDecolagem *f)
{
    return filaVazia(f) ? NULL : f->inicio->aviao;
}

int filaTamanho(const FilaDecolagem *f)
{
    return f->tamanho;
}

/* Remove um aviao especifico da fila (usado quando o aviao e excluido). */
int filaRemoverAviao(FilaDecolagem *f, const Aviao *a)
{
    NoFila *atual = f->inicio;
    NoFila *anterior = NULL;

    while (atual != NULL && atual->aviao != a) {
        anterior = atual;
        atual = atual->proximo;
    }
    if (atual == NULL)
        return 0;
    if (anterior == NULL)
        f->inicio = atual->proximo;
    else
        anterior->proximo = atual->proximo;
    if (f->fim == atual)
        f->fim = anterior;
    free(atual);
    f->tamanho--;
    return 1;
}

void filaLiberar(FilaDecolagem *f)
{
    while (!filaVazia(f))
        filaDesenfileirar(f);
}

Pista *controleLocalizarAviao(Sistema *s, const Aviao *a)
{
    int i;
    const NoFila *no;

    for (i = 0; i < s->aeroporto.totalPistas; i++)
        for (no = s->pistas[i].filaEspera.inicio; no != NULL; no = no->proximo)
            if (no->aviao == a)
                return &s->pistas[i];
    return NULL;
}

void controleRemoverAviaoDasFilas(Sistema *s, const Aviao *a)
{
    int i;

    for (i = 0; i < s->aeroporto.totalPistas; i++)
        while (filaRemoverAviao(&s->pistas[i].filaEspera, a))
            ;
}

/* Tabela com exatamente LARGURA_TELA (70) colunas, da frente para o fim. */
void controleListarFila(const Pista *p)
{
    const NoFila *no = p->filaEspera.inicio;
    int pos = 1;

    if (no == NULL) {
        printf(" A fila da pista '%s' esta vazia.\n", p->nome);
        return;
    }
    printf(" Pista '%s' (frente -> fim):\n\n", p->nome);
    printf(" %-5s %-4s %-24s %-10s %-22s\n", "POS", "ID", "NOME", "VOO", "MODELO");
    for (; no != NULL; no = no->proximo)
        printf(" %-5d %-4d %-24.24s %-10.10s %-22.22s\n", pos++, no->aviao->id,
               no->aviao->nome, no->aviao->codigoVoo, no->aviao->modelo);
}

int controleTotalEmEspera(const Sistema *s)
{
    int i, total = 0;

    for (i = 0; i < s->aeroporto.totalPistas; i++)
        total += filaTamanho(&s->pistas[i].filaEspera);
    return total;
}
