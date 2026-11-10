#ifndef AEROCONTROL_H
#define AEROCONTROL_H

/*
 * aerocontrol.h
 * Modelos de dados e prototipos do sistema de controle aeroportuario.
 *
 * Mapeamento de estruturas de dados:
 *   - Aeroporto      -> struct simples
 *   - Avioes         -> lista simplesmente encadeada (Aviao*)
 *   - Pistas         -> vetor estatico (Pista pistas[MAX_PISTAS])
 *   - Controle Aereo -> fila FIFO (FilaDecolagem) em cada pista
 */

/* Layout padrao das telas (compartilhado por main.c e modulos de exibicao) */
#define LARGURA_TELA 70
#define FMT_CAMPO    " %-26s: "   /* rotulo alinhado em fichas de detalhe */

#define MAX_PISTAS   10
#define TAM_NOME     64
#define TAM_IATA     4   /* 3 letras + '\0' */
#define TAM_TIPO     32
#define TAM_MODELO   48
#define TAM_FABRIC   48
#define TAM_VOO      12
#define TAM_SUPERF   32

/* ======================= MODELOS ======================= */

typedef struct {
    int  id;
    char nome[TAM_NOME];
    char codigoIATA[TAM_IATA];
    char tipo[TAM_TIPO];
    long capacidadePassageirosAnual;
    int  totalAvioes;
    int  totalPistas;
} Aeroporto;

typedef struct Aviao {
    int   id;
    char  nome[TAM_NOME];
    char  modelo[TAM_MODELO];
    char  fabricante[TAM_FABRIC];
    char  codigoVoo[TAM_VOO];
    int   maxPassageiros;
    float velocidadeMaxima;
    struct Aviao *proximo;
} Aviao;

/* No da fila: referencia um aviao que esta na lista encadeada. */
typedef struct NoFila {
    Aviao         *aviao;
    struct NoFila *proximo;
} NoFila;

typedef struct {
    NoFila *inicio;   /* primeiro a decolar (frente) */
    NoFila *fim;      /* ultimo a entrar (cauda)     */
    int     tamanho;
} FilaDecolagem;

typedef struct {
    int           id;
    char          nome[TAM_NOME];
    float         comprimento;
    char          superficie[TAM_SUPERF];
    FilaDecolagem filaEspera;
} Pista;

/* Estado global do sistema (agrupa as tres estruturas). */
typedef struct {
    Aeroporto aeroporto;
    int       aeroportoCadastrado;
    Aviao    *avioes;               /* cabeca da lista encadeada */
    int       proximoIdAviao;
    Pista     pistas[MAX_PISTAS];   /* vetor de pistas           */
    int       proximoIdPista;
} Sistema;

/* ======================= AEROPORTO (aeroporto.c) ======================= */

void sistemaInicializar(Sistema *s);
void sistemaLiberar(Sistema *s);
void aeroportoCadastrar(Sistema *s, const char *nome, const char *iata,
                        const char *tipo, long capacidadeAnual);
void aeroportoExibir(const Sistema *s);

/* ======================= AVIOES (avioes.c) ======================= */

Aviao *aviaoCriar(int id, const char *nome, const char *modelo,
                  const char *fabricante, const char *codigoVoo,
                  int maxPassageiros, float velocidadeMaxima);
int    aviaoInserir(Sistema *s, Aviao *novo);        /* retorna id */
Aviao *aviaoBuscar(const Sistema *s, int id);
int    aviaoAtualizar(Sistema *s, int id, const char *nome, const char *modelo,
                      const char *fabricante, const char *codigoVoo,
                      int maxPassageiros, float velocidadeMaxima);
int    aviaoRemover(Sistema *s, int id);             /* 1 = ok, 0 = nao achou */
void   aviaoExibir(const Aviao *a);
void   avioesListar(const Sistema *s);
void   avioesLiberar(Sistema *s);

/* Ordenacao da lista (Insertion Sort). Velocidade e passageiros: decrescente. */
typedef enum {
    ORDENAR_POR_ID = 1,          /* ordem de cadastro (ordem padrao da lista) */
    ORDENAR_POR_VELOCIDADE,
    ORDENAR_POR_PASSAGEIROS
} CriterioOrdenacao;

void   avioesOrdenar(Sistema *s, CriterioOrdenacao criterio);
void   avioesListarOrdenado(Sistema *s, CriterioOrdenacao criterio);

/* ======================= PISTAS (pistas.c) ======================= */

int    pistaInserir(Sistema *s, const char *nome, float comprimento,
                    const char *superficie);         /* id ou -1 se cheio */
Pista *pistaBuscar(Sistema *s, int id);
int    pistaAtualizar(Sistema *s, int id, const char *nome, float comprimento,
                      const char *superficie);
int    pistaRemover(Sistema *s, int id);
void   pistaExibir(const Pista *p);
void   pistasListar(const Sistema *s);

/* ======================= CONTROLE AEREO (controle.c) ======================= */

void   filaInicializar(FilaDecolagem *f);
int    filaVazia(const FilaDecolagem *f);
int    filaEnfileirar(FilaDecolagem *f, Aviao *a);   /* 1 = ok, 0 = sem memoria */
Aviao *filaDesenfileirar(FilaDecolagem *f);
Aviao *filaPrimeiro(const FilaDecolagem *f);
int    filaTamanho(const FilaDecolagem *f);
int    filaRemoverAviao(FilaDecolagem *f, const Aviao *a);
void   filaLiberar(FilaDecolagem *f);

Pista *controleLocalizarAviao(Sistema *s, const Aviao *a); /* pista onde esta, ou NULL */
void   controleRemoverAviaoDasFilas(Sistema *s, const Aviao *a);
void   controleListarFila(const Pista *p);
int    controleTotalEmEspera(const Sistema *s);

#endif /* AEROCONTROL_H */
