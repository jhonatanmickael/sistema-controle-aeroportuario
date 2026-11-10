/*
 * aeroporto.c
 * Inicializacao/liberacao do sistema e cadastro/exibicao do Aeroporto
 * (struct simples).
 */
#include <stdio.h>
#include <string.h>
#include "aerocontrol.h"

/* Copia segura de string com terminacao garantida. */
static void copiar(char *dest, const char *orig, size_t tam)
{
    strncpy(dest, orig, tam - 1);
    dest[tam - 1] = '\0';
}

void sistemaInicializar(Sistema *s)
{
    int i;

    memset(s, 0, sizeof(*s));
    s->aeroportoCadastrado = 0;
    s->avioes = NULL;
    s->proximoIdAviao = 1;
    s->proximoIdPista = 1;
    for (i = 0; i < MAX_PISTAS; i++)
        filaInicializar(&s->pistas[i].filaEspera);
}

void sistemaLiberar(Sistema *s)
{
    int i;

    for (i = 0; i < s->aeroporto.totalPistas; i++)
        filaLiberar(&s->pistas[i].filaEspera);
    s->aeroporto.totalPistas = 0;
    avioesLiberar(s);
}

void aeroportoCadastrar(Sistema *s, const char *nome, const char *iata,
                        const char *tipo, long capacidadeAnual)
{
    Aeroporto *a = &s->aeroporto;
    size_t i;

    a->id = 1;
    copiar(a->nome, nome, sizeof(a->nome));
    copiar(a->codigoIATA, iata, sizeof(a->codigoIATA));
    for (i = 0; a->codigoIATA[i] != '\0'; i++)
        if (a->codigoIATA[i] >= 'a' && a->codigoIATA[i] <= 'z')
            a->codigoIATA[i] = (char)(a->codigoIATA[i] - 'a' + 'A');
    copiar(a->tipo, tipo, sizeof(a->tipo));
    a->capacidadePassageirosAnual = capacidadeAnual;
    /* totalAvioes e totalPistas sao mantidos pelos modulos de CRUD */
    s->aeroportoCadastrado = 1;
}

void aeroportoExibir(const Sistema *s)
{
    const Aeroporto *a = &s->aeroporto;

    if (!s->aeroportoCadastrado) {
        printf(" Nenhum aeroporto cadastrado ainda. Use a opcao 1.1.\n");
        return;
    }
    printf(FMT_CAMPO "%d\n",  "ID", a->id);
    printf(FMT_CAMPO "%s\n",  "Nome", a->nome);
    printf(FMT_CAMPO "%s\n",  "Codigo IATA", a->codigoIATA);
    printf(FMT_CAMPO "%s\n",  "Tipo", a->tipo);
    printf(FMT_CAMPO "%ld\n", "Capacidade anual (pax)", a->capacidadePassageirosAnual);
    printf(FMT_CAMPO "%d\n",  "Total de avioes", a->totalAvioes);
    printf(FMT_CAMPO "%d\n",  "Total de pistas", a->totalPistas);
}
