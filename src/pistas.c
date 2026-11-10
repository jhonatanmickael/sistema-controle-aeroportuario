/*
 * pistas.c
 * CRUD de pistas sobre um VETOR estatico (Pista pistas[MAX_PISTAS]).
 * As posicoes ocupadas sao sempre 0 .. totalPistas-1 (vetor compacto).
 */
#include <stdio.h>
#include <string.h>
#include "aerocontrol.h"

static void copiar(char *dest, const char *orig, size_t tam)
{
    strncpy(dest, orig, tam - 1);
    dest[tam - 1] = '\0';
}

/* Otimização Teórica: Pesquisa Binária O(log n) aplicada sobre o vetor ordenado de pistas */
/*
 * Invariante: s->pistas[0 .. totalPistas-1] esta ordenado por id crescente.
 *   - pistaInserir sempre anexa no fim com id = proximoIdPista++ (maior que todos);
 *   - pistaRemover desloca os elementos seguintes para a esquerda (ordem mantida).
 * Retorna o indice da pista no vetor, ou -1 se o id nao existir.
 */
static int indicePista(const Sistema *s, int id)
{
    int low = 0;
    int high = s->aeroporto.totalPistas - 1;
    int mid;

    while (low <= high) {
        mid = low + (high - low) / 2;   /* evita overflow de (low + high) */
        if (s->pistas[mid].id == id)
            return mid;
        if (s->pistas[mid].id < id)
            low = mid + 1;              /* alvo na metade direita  */
        else
            high = mid - 1;             /* alvo na metade esquerda */
    }
    return -1;
}

int pistaInserir(Sistema *s, const char *nome, float comprimento,
                 const char *superficie)
{
    Pista *p;

    if (s->aeroporto.totalPistas >= MAX_PISTAS)
        return -1;
    p = &s->pistas[s->aeroporto.totalPistas];
    p->id = s->proximoIdPista++;
    copiar(p->nome, nome, sizeof(p->nome));
    p->comprimento = comprimento;
    copiar(p->superficie, superficie, sizeof(p->superficie));
    filaInicializar(&p->filaEspera);
    s->aeroporto.totalPistas++;
    return p->id;
}

Pista *pistaBuscar(Sistema *s, int id)
{
    int i = indicePista(s, id);
    return (i < 0) ? NULL : &s->pistas[i];
}

int pistaAtualizar(Sistema *s, int id, const char *nome, float comprimento,
                   const char *superficie)
{
    Pista *p = pistaBuscar(s, id);

    if (p == NULL)
        return 0;
    copiar(p->nome, nome, sizeof(p->nome));
    p->comprimento = comprimento;
    copiar(p->superficie, superficie, sizeof(p->superficie));
    return 1;
}

/* Remove a pista, libera sua fila e desloca os elementos seguintes. */
int pistaRemover(Sistema *s, int id)
{
    int i = indicePista(s, id);
    int j;

    if (i < 0)
        return 0;
    filaLiberar(&s->pistas[i].filaEspera);
    for (j = i; j < s->aeroporto.totalPistas - 1; j++)
        s->pistas[j] = s->pistas[j + 1];
    s->aeroporto.totalPistas--;
    filaInicializar(&s->pistas[s->aeroporto.totalPistas].filaEspera);
    return 1;
}

void pistaExibir(const Pista *p)
{
    printf(FMT_CAMPO "%d\n",      "ID", p->id);
    printf(FMT_CAMPO "%s\n",      "Nome", p->nome);
    printf(FMT_CAMPO "%.2f m\n",  "Comprimento", p->comprimento);
    printf(FMT_CAMPO "%s\n",      "Superficie", p->superficie);
    printf(FMT_CAMPO "%d\n",      "Avioes na fila de espera", filaTamanho(&p->filaEspera));
}

/* Tabela com exatamente LARGURA_TELA (70) colunas. */
void pistasListar(const Sistema *s)
{
    int i;
    const Pista *p;

    if (s->aeroporto.totalPistas == 0) {
        printf(" Nenhuma pista cadastrada.\n");
        return;
    }
    printf(" %-4s %-24s %-18s %10s %9s\n",
           "ID", "NOME", "SUPERFICIE", "COMPR.(m)", "NA FILA");
    for (i = 0; i < s->aeroporto.totalPistas; i++) {
        p = &s->pistas[i];
        printf(" %-4d %-24.24s %-18.18s %10.1f %9d\n", p->id, p->nome,
               p->superficie, p->comprimento, filaTamanho(&p->filaEspera));
    }
    printf("\n Total: %d de %d pista(s) cadastrada(s).\n",
           s->aeroporto.totalPistas, MAX_PISTAS);
}
