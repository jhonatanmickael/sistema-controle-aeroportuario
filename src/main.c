/*
 * ============================================================================
 *  AeroControl - Sistema de Controle Aeroportuario
 * ----------------------------------------------------------------------------
 *  Instituicao : Universidade Federal de Alagoas (UFAL) - Polo Penedo
 *  Disciplina  : Programacao 2 (Prof. Thyago Tenorio)
 *  Autores     : Jhonatan Mickael, Maximus Eduardo, Roger, Cauet Remigio
 * ----------------------------------------------------------------------------
 *  Estruturas de dados:
 *    Aeroporto      -> struct simples
 *    Avioes         -> lista simplesmente encadeada (malloc/free)
 *    Pistas         -> vetor (Pista pistas[MAX_PISTAS])
 *    Controle Aereo -> fila FIFO de decolagem em cada pista
 * ============================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "aerocontrol.h"

#define TAM_ENTRADA 256

static Sistema sistema;

/* ======================= LAYOUT DAS TELAS =======================
 *
 * Todas as telas (menus, formularios e resultados) seguem o MESMO modelo,
 * com LARGURA_TELA (70) colunas:
 *
 *   ======================================================================
 *                    AEROCONTROL - CONTROLE AEROPORTUARIO
 *   ======================================================================
 *                             TITULO DA SECAO
 *   ----------------------------------------------------------------------
 *    conteudo (opcoes, campos do formulario ou resultado)
 *   ----------------------------------------------------------------------
 *   Opcao:   |   Pressione ENTER para continuar...
 */

/* Limpa o terminal com sequencias ANSI: \033[H move o cursor para o topo
 * e \033[J apaga dali ate o fim da tela. Nao usa system("clear"). */
static void limparTela(void)
{
    printf("\033[H\033[J");
    fflush(stdout);
}

static void linha(char c)
{
    int i;
    for (i = 0; i < LARGURA_TELA; i++)
        putchar(c);
    putchar('\n');
}

static void centralizado(const char *texto)
{
    int len = (int)strlen(texto);
    int esq = (len < LARGURA_TELA) ? (LARGURA_TELA - len) / 2 : 0;

    printf("%*s%.*s\n", esq, "", LARGURA_TELA, texto);
}

/* Abre uma tela: limpa, cabecalho fixo, titulo da secao e divisoria. */
static void abrirTela(const char *titulo)
{
    limparTela();
    linha('=');
    centralizado("AEROCONTROL - CONTROLE AEROPORTUARIO");
    linha('=');
    centralizado(titulo);
    linha('-');
}

/* ======================= ENTRADA DE DADOS ======================= */

/* Le uma linha do stdin (sem '\n'). Em EOF libera a memoria e encerra. */
static void lerLinha(const char *prompt, char *buf, size_t tam)
{
    size_t n;

    printf("%s", prompt);
    fflush(stdout);
    if (fgets(buf, (int)tam, stdin) == NULL) {
        printf("\n Fim da entrada. Encerrando...\n");
        sistemaLiberar(&sistema);
        exit(EXIT_SUCCESS);
    }
    n = strlen(buf);
    if (n > 0 && buf[n - 1] == '\n') {
        buf[n - 1] = '\0';
    } else {
        int c; /* linha maior que o buffer: descarta o restante */
        while ((c = getchar()) != '\n' && c != EOF)
            ;
    }
    n = strlen(buf); /* remove espacos/'\r' no final */
    while (n > 0 && (buf[n - 1] == ' ' || buf[n - 1] == '\r' || buf[n - 1] == '\t'))
        buf[--n] = '\0';
}

static int converterLong(const char *txt, long *out)
{
    char *fim;
    long v;

    if (*txt == '\0')
        return 0;
    errno = 0;
    v = strtol(txt, &fim, 10);
    if (errno != 0 || *fim != '\0')
        return 0;
    *out = v;
    return 1;
}

static int converterFloat(const char *txt, float *out)
{
    char *fim;
    float v;

    if (*txt == '\0')
        return 0;
    errno = 0;
    v = strtof(txt, &fim);
    if (errno != 0 || *fim != '\0')
        return 0;
    *out = v;
    return 1;
}

/* Copia ate tam-1 caracteres, sempre terminando com '\0'. */
static void copiarLimitado(char *dest, const char *orig, size_t tam)
{
    size_t n = strlen(orig);

    if (n >= tam)
        n = tam - 1;
    memcpy(dest, orig, n);
    dest[n] = '\0';
}

/* Monta o prompt de um campo de formulario: " Rotulo [atual]: " */
static void montarPrompt(char *prompt, size_t tam, const char *rotulo, const char *atual)
{
    if (atual != NULL)
        snprintf(prompt, tam, " %s [%s]: ", rotulo, atual);
    else
        snprintf(prompt, tam, " %s: ", rotulo);
}

/* Inteiro em [min, max] com prompt pronto. Se atual != NULL, ENTER mantem. */
static long lerLongPrompt(const char *prompt, long min, long max, const long *atual)
{
    char buf[TAM_ENTRADA];
    long v;

    for (;;) {
        lerLinha(prompt, buf, sizeof(buf));
        if (buf[0] == '\0' && atual != NULL)
            return *atual;
        if (converterLong(buf, &v) && v >= min && v <= max)
            return v;
        printf(" ! Valor invalido. Informe um inteiro entre %ld e %ld.\n", min, max);
    }
}

/* Texto obrigatorio. Se 'atual' != NULL, ENTER mantem o valor atual. */
static void lerTexto(const char *rotulo, char *dest, size_t tam, const char *atual)
{
    char buf[TAM_ENTRADA];
    char prompt[TAM_ENTRADA];

    montarPrompt(prompt, sizeof(prompt), rotulo, atual);
    for (;;) {
        lerLinha(prompt, buf, sizeof(buf));
        if (buf[0] == '\0' && atual != NULL) {
            copiarLimitado(dest, atual, tam);
            return;
        }
        if (buf[0] != '\0')
            break;
        printf(" ! Valor obrigatorio.\n");
    }
    if (strlen(buf) >= tam)
        printf(" ! Texto truncado para %lu caracteres.\n", (unsigned long)(tam - 1));
    copiarLimitado(dest, buf, tam);
}

static long lerLong(const char *rotulo, long min, long max, const long *atual)
{
    char prompt[TAM_ENTRADA];
    char valor[32];

    if (atual != NULL)
        snprintf(valor, sizeof(valor), "%ld", *atual);
    montarPrompt(prompt, sizeof(prompt), rotulo, atual ? valor : NULL);
    return lerLongPrompt(prompt, min, max, atual);
}

static float lerFloat(const char *rotulo, float min, const float *atual)
{
    char buf[TAM_ENTRADA];
    char prompt[TAM_ENTRADA];
    char valor[32];
    float v;

    if (atual != NULL)
        snprintf(valor, sizeof(valor), "%.2f", (double)*atual);
    montarPrompt(prompt, sizeof(prompt), rotulo, atual ? valor : NULL);
    for (;;) {
        lerLinha(prompt, buf, sizeof(buf));
        if (buf[0] == '\0' && atual != NULL)
            return *atual;
        if (converterFloat(buf, &v) && v > min)
            return v;
        printf(" ! Valor invalido. Informe um numero maior que %.2f (use '.').\n",
               (double)min);
    }
}

static int lerId(const char *rotulo)
{
    return (int)lerLong(rotulo, 1, 1000000, NULL);
}

/* Fecha um menu (divisoria) e le a opcao escolhida. */
static long lerOpcao(long min, long max)
{
    linha('-');
    return lerLongPrompt("Opcao: ", min, max, NULL);
}

/* Fecha uma tela de acao (divisoria) e espera o usuario ler o resultado. */
static void pausar(void)
{
    char buf[8];

    linha('-');
    lerLinha("Pressione ENTER para continuar...", buf, sizeof(buf));
}

static int exigirAeroporto(void)
{
    if (!sistema.aeroportoCadastrado) {
        printf(" Cadastre primeiro os dados do aeroporto (opcao 1.1).\n");
        return 0;
    }
    return 1;
}

/* ======================= 1. AEROPORTO ======================= */

static void opCadastrarAeroporto(void)
{
    char nome[TAM_NOME], iata[TAM_IATA], tipo[TAM_TIPO];
    const Aeroporto *a = &sistema.aeroporto;
    int existe = sistema.aeroportoCadastrado;
    long cap;

    abrirTela("1.1 CADASTRAR DADOS DO AEROPORTO");
    if (existe)
        printf(" Aeroporto ja cadastrado: ENTER mantem o valor atual.\n\n");

    lerTexto("Nome", nome, sizeof(nome), existe ? a->nome : NULL);
    for (;;) {
        lerTexto("Codigo IATA (3 letras)", iata, sizeof(iata), existe ? a->codigoIATA : NULL);
        if (strlen(iata) == 3)
            break;
        printf(" ! O codigo IATA deve ter exatamente 3 letras.\n");
    }
    lerTexto("Tipo (Internacional/Domestico/Regional)", tipo, sizeof(tipo),
             existe ? a->tipo : NULL);
    cap = lerLong("Capacidade anual de passageiros", 0, 2000000000L,
                  existe ? &a->capacidadePassageirosAnual : NULL);

    aeroportoCadastrar(&sistema, nome, iata, tipo, cap);
    printf("\n Aeroporto %s com sucesso.\n", existe ? "atualizado" : "cadastrado");
}

static void opVisualizarAeroporto(void)
{
    abrirTela("1.2 VISUALIZAR INFORMACOES DO AEROPORTO");
    aeroportoExibir(&sistema);
}

static void menuAeroporto(void)
{
    for (;;) {
        abrirTela("1. AEROPORTO");
        printf(" 1.1 Cadastrar Dados do Aeroporto\n");
        printf(" 1.2 Visualizar Informacoes do Aeroporto\n");
        printf(" 0. Voltar\n");
        switch (lerOpcao(0, 2)) {
        case 1: opCadastrarAeroporto(); break;
        case 2: opVisualizarAeroporto(); break;
        default: return;
        }
        pausar();
    }
}

/* ======================= 2. AVIOES ======================= */

static void lerDadosAviao(char *nome, char *modelo, char *fabricante, char *voo,
                          int *maxPax, float *vel, const Aviao *atual)
{
    long paxAtual = atual ? atual->maxPassageiros : 0;

    lerTexto("Nome", nome, TAM_NOME, atual ? atual->nome : NULL);
    lerTexto("Modelo", modelo, TAM_MODELO, atual ? atual->modelo : NULL);
    lerTexto("Fabricante", fabricante, TAM_FABRIC, atual ? atual->fabricante : NULL);
    lerTexto("Codigo do voo", voo, TAM_VOO, atual ? atual->codigoVoo : NULL);
    *maxPax = (int)lerLong("Maximo de passageiros", 1, 1000, atual ? &paxAtual : NULL);
    *vel = lerFloat("Velocidade maxima (km/h)", 0.0f, atual ? &atual->velocidadeMaxima : NULL);
}

static void opCadastrarAviao(void)
{
    char nome[TAM_NOME], modelo[TAM_MODELO], fab[TAM_FABRIC], voo[TAM_VOO];
    int maxPax;
    float vel;
    Aviao *novo;

    abrirTela("2.1 CADASTRAR AVIAO NO AEROPORTO");
    if (!exigirAeroporto())
        return;
    lerDadosAviao(nome, modelo, fab, voo, &maxPax, &vel, NULL);
    novo = aviaoCriar(sistema.proximoIdAviao, nome, modelo, fab, voo, maxPax, vel);
    if (novo == NULL) {
        printf("\n ! Erro: memoria insuficiente.\n");
        return;
    }
    sistema.proximoIdAviao++;
    printf("\n Aviao cadastrado com ID %d.\n", aviaoInserir(&sistema, novo));
}

static void opListarAvioes(void)
{
    static const char *const nomes[] = {
        "", "ID (ordem de cadastro)", "Velocidade maxima (maior primeiro)",
        "Maximo de passageiros (maior primeiro)"
    };
    const long padrao = ORDENAR_POR_ID;
    long criterio = ORDENAR_POR_ID;

    abrirTela("2.2 LISTAR TODOS OS AVIOES DO AEROPORTO");
    if (sistema.aeroporto.totalAvioes > 1) {
        printf(" Ordenar por: 1. ID   2. Velocidade   3. Passageiros\n");
        criterio = lerLong("Criterio", ORDENAR_POR_ID, ORDENAR_POR_PASSAGEIROS, &padrao);
        printf("\n Ordenado por: %s\n\n", nomes[criterio]);
    }
    avioesListarOrdenado(&sistema, (CriterioOrdenacao)criterio);
}

static void opRecuperarAviao(void)
{
    Aviao *a;
    Pista *p;
    char situacao[TAM_NOME + 48];

    abrirTela("2.3 RECUPERAR AVIAO DO AEROPORTO POR ID");
    a = aviaoBuscar(&sistema, lerId("ID do aviao"));
    if (a == NULL) {
        printf("\n Aviao nao encontrado.\n");
        return;
    }
    putchar('\n');
    aviaoExibir(a);
    p = controleLocalizarAviao(&sistema, a);
    if (p != NULL)
        snprintf(situacao, sizeof(situacao), "Na fila da pista '%s' (ID %d)", p->nome, p->id);
    else
        snprintf(situacao, sizeof(situacao), "Fora de fila de decolagem");
    printf(FMT_CAMPO "%s\n", "Situacao", situacao);
}

static void opAtualizarAviao(void)
{
    char nome[TAM_NOME], modelo[TAM_MODELO], fab[TAM_FABRIC], voo[TAM_VOO];
    int maxPax, id;
    float vel;
    Aviao *a;

    abrirTela("2.4 ATUALIZAR AVIAO DO AEROPORTO");
    id = lerId("ID do aviao");
    a = aviaoBuscar(&sistema, id);
    if (a == NULL) {
        printf("\n Aviao nao encontrado.\n");
        return;
    }
    printf("\n ENTER mantem o valor atual.\n\n");
    lerDadosAviao(nome, modelo, fab, voo, &maxPax, &vel, a);
    aviaoAtualizar(&sistema, id, nome, modelo, fab, voo, maxPax, vel);
    printf("\n Aviao atualizado com sucesso.\n");
}

static void opRemoverAviao(void)
{
    int id;
    Aviao *a;
    Pista *p;

    abrirTela("2.5 REMOVER AVIAO DO AEROPORTO");
    id = lerId("ID do aviao");
    a = aviaoBuscar(&sistema, id);
    if (a == NULL) {
        printf("\n Aviao nao encontrado.\n");
        return;
    }
    p = controleLocalizarAviao(&sistema, a);
    putchar('\n');
    if (p != NULL)
        printf(" Aviao retirado da fila de espera da pista '%s'.\n", p->nome);
    aviaoRemover(&sistema, id);
    printf(" Aviao ID %d removido com sucesso.\n", id);
}

static void menuAvioes(void)
{
    for (;;) {
        abrirTela("2. AVIOES");
        printf(" 2.1 Cadastrar Aviao no Aeroporto\n");
        printf(" 2.2 Listar Todos os Avioes do Aeroporto\n");
        printf(" 2.3 Recuperar Aviao do Aeroporto por ID\n");
        printf(" 2.4 Atualizar Aviao do Aeroporto\n");
        printf(" 2.5 Remover Aviao do Aeroporto\n");
        printf(" 0. Voltar\n");
        switch (lerOpcao(0, 5)) {
        case 1: opCadastrarAviao(); break;
        case 2: opListarAvioes(); break;
        case 3: opRecuperarAviao(); break;
        case 4: opAtualizarAviao(); break;
        case 5: opRemoverAviao(); break;
        default: return;
        }
        pausar();
    }
}

/* ======================= 3. PISTAS ======================= */

static void opCadastrarPista(void)
{
    char nome[TAM_NOME], sup[TAM_SUPERF];
    float comp;
    int id;

    abrirTela("3.1 CADASTRAR PISTA NO AEROPORTO");
    if (!exigirAeroporto())
        return;
    if (sistema.aeroporto.totalPistas >= MAX_PISTAS) {
        printf(" Limite de %d pistas atingido.\n", MAX_PISTAS);
        return;
    }
    lerTexto("Nome (ex.: 09/27)", nome, sizeof(nome), NULL);
    comp = lerFloat("Comprimento (m)", 0.0f, NULL);
    lerTexto("Superficie (ex.: Asfalto, Concreto)", sup, sizeof(sup), NULL);
    id = pistaInserir(&sistema, nome, comp, sup);
    printf("\n Pista cadastrada com ID %d.\n", id);
}

static void opListarPistas(void)
{
    abrirTela("3.2 LISTAR TODAS AS PISTAS DO AEROPORTO");
    pistasListar(&sistema);
}

static void opRecuperarPista(void)
{
    Pista *p;

    abrirTela("3.3 RECUPERAR PISTA DO AEROPORTO POR ID");
    p = pistaBuscar(&sistema, lerId("ID da pista"));
    putchar('\n');
    if (p == NULL)
        printf(" Pista nao encontrada.\n");
    else
        pistaExibir(p);
}

static void opAtualizarPista(void)
{
    char nome[TAM_NOME], sup[TAM_SUPERF];
    float comp;
    int id;
    Pista *p;

    abrirTela("3.4 ATUALIZAR PISTA DO AEROPORTO");
    id = lerId("ID da pista");
    p = pistaBuscar(&sistema, id);
    if (p == NULL) {
        printf("\n Pista nao encontrada.\n");
        return;
    }
    printf("\n ENTER mantem o valor atual.\n\n");
    lerTexto("Nome", nome, sizeof(nome), p->nome);
    comp = lerFloat("Comprimento (m)", 0.0f, &p->comprimento);
    lerTexto("Superficie", sup, sizeof(sup), p->superficie);
    pistaAtualizar(&sistema, id, nome, comp, sup);
    printf("\n Pista atualizada com sucesso.\n");
}

static void opRemoverPista(void)
{
    int id, naFila;
    Pista *p;

    abrirTela("3.5 REMOVER PISTA DO AEROPORTO");
    id = lerId("ID da pista");
    p = pistaBuscar(&sistema, id);
    if (p == NULL) {
        printf("\n Pista nao encontrada.\n");
        return;
    }
    naFila = filaTamanho(&p->filaEspera);
    pistaRemover(&sistema, id);
    printf("\n Pista ID %d removida com sucesso.\n", id);
    if (naFila > 0)
        printf(" %d aviao(oes) sairam da fila desta pista e continuam cadastrados.\n",
               naFila);
}

static void menuPistas(void)
{
    for (;;) {
        abrirTela("3. PISTAS");
        printf(" 3.1 Cadastrar Pista no Aeroporto\n");
        printf(" 3.2 Listar Todas as Pistas do Aeroporto\n");
        printf(" 3.3 Recuperar Pista do Aeroporto por ID\n");
        printf(" 3.4 Atualizar Pista do Aeroporto\n");
        printf(" 3.5 Remover Pista do Aeroporto\n");
        printf(" 0. Voltar\n");
        switch (lerOpcao(0, 5)) {
        case 1: opCadastrarPista(); break;
        case 2: opListarPistas(); break;
        case 3: opRecuperarPista(); break;
        case 4: opAtualizarPista(); break;
        case 5: opRemoverPista(); break;
        default: return;
        }
        pausar();
    }
}

/* ======================= 4. CONTROLE AEREO ======================= */

/* Pede o ID de uma pista; retorna NULL (com mensagem) se nao existir. */
static Pista *selecionarPista(void)
{
    Pista *p;

    if (sistema.aeroporto.totalPistas == 0) {
        printf(" Nenhuma pista cadastrada.\n");
        return NULL;
    }
    p = pistaBuscar(&sistema, lerId("ID da pista"));
    if (p == NULL)
        printf("\n Pista nao encontrada.\n");
    return p;
}

static void opAdicionarNaFila(void)
{
    Pista *p, *atual;
    Aviao *a;

    abrirTela("4.1 ADICIONAR AVIAO A FILA DE ESPERA DE UMA PISTA");
    if ((p = selecionarPista()) == NULL)
        return;
    a = aviaoBuscar(&sistema, lerId("ID do aviao"));
    putchar('\n');
    if (a == NULL) {
        printf(" Aviao nao encontrado.\n");
        return;
    }
    atual = controleLocalizarAviao(&sistema, a);
    if (atual != NULL) {
        printf(" O aviao ja esta na fila da pista '%s' (ID %d).\n", atual->nome, atual->id);
        return;
    }
    if (!filaEnfileirar(&p->filaEspera, a)) {
        printf(" ! Erro: memoria insuficiente.\n");
        return;
    }
    printf(" Aviao '%s' adicionado a fila da pista '%s' (posicao %d).\n",
           a->nome, p->nome, filaTamanho(&p->filaEspera));
}

static void opListarFila(void)
{
    Pista *p;

    abrirTela("4.2 LISTAR AVIOES DA FILA DE ESPERA DE UMA PISTA");
    if ((p = selecionarPista()) != NULL) {
        putchar('\n');
        controleListarFila(p);
    }
}

static void opTamanhoFila(void)
{
    Pista *p;

    abrirTela("4.3 NUMERO DE AVIOES NA FILA DE DECOLAGEM DE UMA PISTA");
    if ((p = selecionarPista()) != NULL)
        printf("\n Avioes aguardando na pista '%s': %d\n", p->nome,
               filaTamanho(&p->filaEspera));
}

static void opAutorizarDecolagem(void)
{
    Pista *p;
    Aviao *a;

    abrirTela("4.4 AUTORIZAR DECOLAGEM DO PRIMEIRO AVIAO DA FILA");
    if ((p = selecionarPista()) == NULL)
        return;
    a = filaDesenfileirar(&p->filaEspera);
    putchar('\n');
    if (a == NULL) {
        printf(" Nao ha avioes na fila da pista '%s'.\n", p->nome);
        return;
    }
    printf(" Decolagem AUTORIZADA pela pista '%s':\n", p->nome);
    printf(" [ID %d] %s - Voo %s\n\n", a->id, a->nome, a->codigoVoo);
    printf(" Restam %d aviao(oes) na fila.\n", filaTamanho(&p->filaEspera));
}

static void opPrimeiroDaFila(void)
{
    Pista *p;
    Aviao *a;

    abrirTela("4.5 CARACTERISTICAS DO PRIMEIRO AVIAO DA FILA");
    if ((p = selecionarPista()) == NULL)
        return;
    a = filaPrimeiro(&p->filaEspera);
    putchar('\n');
    if (a == NULL)
        printf(" Nao ha avioes na fila da pista '%s'.\n", p->nome);
    else
        aviaoExibir(a);
}

static void opTotalEmEspera(void)
{
    int i;
    const Pista *p;

    abrirTela("4.6 TOTAL DE AVIOES AGUARDANDO EM FILAS DE ESPERA");
    if (sistema.aeroporto.totalPistas > 0) {
        printf(" %-4s %-40s %23s\n", "ID", "PISTA", "AVIOES NA FILA");
        for (i = 0; i < sistema.aeroporto.totalPistas; i++) {
            p = &sistema.pistas[i];
            printf(" %-4d %-40.40s %23d\n", p->id, p->nome, filaTamanho(&p->filaEspera));
        }
        putchar('\n');
    }
    printf(" Total de avioes aguardando: %d\n", controleTotalEmEspera(&sistema));
}

static void menuControle(void)
{
    for (;;) {
        abrirTela("4. CONTROLE AEREO");
        printf(" 4.1 Adicionar Aviao a fila de espera de uma Pista do Aeroporto\n");
        printf(" 4.2 Listar Todos os Avioes da Fila de Espera de uma Pista\n");
        printf(" 4.3 Listar o numero de avioes aguardando na fila de decolagem de uma Pista\n");
        printf(" 4.4 Autorizar a decolagem do primeiro aviao da fila de uma pista\n");
        printf(" 4.5 Listar as caracteristicas do primeiro aviao da fila de uma pista\n");
        printf(" 4.6 Listar o numero total de avioes aguardando em filas de espera\n");
        printf(" 0. Voltar\n");
        switch (lerOpcao(0, 6)) {
        case 1: opAdicionarNaFila(); break;
        case 2: opListarFila(); break;
        case 3: opTamanhoFila(); break;
        case 4: opAutorizarDecolagem(); break;
        case 5: opPrimeiroDaFila(); break;
        case 6: opTotalEmEspera(); break;
        default: return;
        }
        pausar();
    }
}

/* ======================= MENU PRINCIPAL ======================= */

int main(void)
{
    char titulo[LARGURA_TELA + 1];

    sistemaInicializar(&sistema);

    for (;;) {
        if (sistema.aeroportoCadastrado)
            snprintf(titulo, sizeof(titulo), "MENU PRINCIPAL - %s", sistema.aeroporto.codigoIATA);
        else
            snprintf(titulo, sizeof(titulo), "MENU PRINCIPAL");
        abrirTela(titulo);
        printf(" 1. Aeroporto\n");
        printf(" 2. Avioes\n");
        printf(" 3. Pistas\n");
        printf(" 4. Controle Aereo\n");
        printf(" 5. Sair\n");
        switch (lerOpcao(1, 5)) {
        case 1: menuAeroporto(); break;
        case 2: menuAvioes(); break;
        case 3: menuPistas(); break;
        case 4: menuControle(); break;
        default:
            sistemaLiberar(&sistema);
            limparTela();
            printf(" Memoria liberada. Ate logo!\n");
            return EXIT_SUCCESS;
        }
    }
}
