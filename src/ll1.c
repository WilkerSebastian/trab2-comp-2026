/*
 * ll1.c - Verificacao da condicao LL(1) (etapa 4).
 *
 * Condicoes (secao 3 do enunciado):
 *   1. Em a1 | a2 | ... | an, os FIRST das alternativas sao disjuntos dois
 *      a dois e no maximo uma alternativa e anulavel.
 *   2. Se uma alternativa ai e anulavel, FOLLOW(alternativa) e disjunto
 *      dos FIRST das demais.
 *   3. Em [ a ] e { a }, a nao e anulavel e FIRST(a) e disjunto do
 *      FOLLOW da construcao.
 *
 * Violacoes de 1, 2 e da primeira parte de 3 sao ERROS. A violacao de
 * FIRST(a) inter FOLLOW nao vazio em [ a ] ou { a } e reportada como
 * AVISO e resolvida de forma gulosa: o analisador gerado entra na
 * construcao sempre que o token estiver em FIRST(a). E o caso classico
 * do "else pendente":  if = "if" "(" expr ")" cmd [ "else" cmd ] ;
 * em que o else fica com o if mais proximo, como em C.
 *
 * Fornecido: reportar (formata as mensagens).
 * TODO(aluno): verificar_expr e ll1_verificar.
 */
#include <stdarg.h>
#include <stdio.h>
#include "ll1.h"
#include "util.h"

/* Reporta um conflito na regra do nao terminal nt, na posicao de e.
 * comum (pode ser NULL) e o conjunto de tokens em conflito. */
static void reportar(const Gramatica *g, int nt, const Expr *e, int eh_aviso,
                     const Bitset *comum, const char *fmt, ...)
{
    va_list ap;
    fprintf(stderr, "%s:%d:%d: %s: conflito LL(1) em %s: ", g->arquivo, e->linha,
            e->coluna, eh_aviso ? "aviso" : "erro", g->nts[nt].nome);
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    if (comum != NULL) {
        fprintf(stderr, " ");
        gram_imprimir_conjunto(stderr, g, comum);
    }
    fputc('\n', stderr);
}

static int verificar_alt(const Gramatica *g, int nt, const Expr *e) {

    Bitset comum;
    
    int i; 

    int j; 

    int erros = 0;

    int anulaveis = 0;

    bitset_iniciar(&comum, g->nterms);

    for (i = 0; i < e->nfilhos; i++)
        for (j = i + 1; j < e->nfilhos; j++) {

            bitset_intersecao(&comum, &e->filhos[i]->first, &e->filhos[j]->first);

            if (!bitset_vazio(&comum)) {

                reportar(g, nt, e, 0, &comum, "alternativas %d e %d comecam com", i + 1, j + 1);

                erros++;

            }

        }

    for (i = 0; i < e->nfilhos; i++)
        if (e->filhos[i]->anulavel)
            anulaveis++;

    if (anulaveis > 1) {

        for (i = 0; i < e->nfilhos; i++)
            for (j = i + 1; j < e->nfilhos; j++)
                if (e->filhos[i]->anulavel && e->filhos[j]->anulavel) {

                    reportar(g, nt, e, 0, NULL, "alternativas %d e %d sao ambas anulaveis", i + 1, j + 1);

                    erros++;

                }

    }

    for (i = 0; i < e->nfilhos; i++) {

        if (!e->filhos[i]->anulavel)
            continue;

        for (j = 0; j < e->nfilhos; j++) {

            if (j == i)
                continue;

            bitset_intersecao(&comum, &e->follow, &e->filhos[j]->first);

            if (!bitset_vazio(&comum)) {
    
                reportar(g, nt, e, 0, &comum, "alternativa %d e anulavel e o FOLLOW intersecta o FIRST da alternativa %d em", i + 1, j + 1);

                erros++;

            }

        }

    }

    bitset_liberar(&comum);

    return erros;

}

static int verificar_opc_rep(const Gramatica *g, int nt, const Expr *e) {

    Bitset comum;

    const Expr *conteudo = e->filhos[0];

    const char *rotulo = (e->tipo == E_OPC) ? "[ ]" : "{ }";

    if (conteudo->anulavel) {

        reportar(g, nt, e, 0, NULL, "o conteudo de %s e anulavel", rotulo);

        return 1;

    }

    bitset_iniciar(&comum, g->nterms);
    
    bitset_intersecao(&comum, &conteudo->first, &e->follow);
    
    if (!bitset_vazio(&comum))
        reportar(g, nt, e, 1, &comum, "FIRST e FOLLOW de %s se intersectam em", rotulo);
    
    bitset_liberar(&comum);

    return 0;

}

static int verificar_expr(const Gramatica *g, int nt, const Expr *e) {

    int i;

    int erros = 0;

    if (e == NULL)
        return 0;

    switch (e->tipo) {

        case E_ALT:

            erros += verificar_alt(g, nt, e);

            break;
        
        case E_OPC:
        case E_REP:

            erros += verificar_opc_rep(g, nt, e);

            break;

        default:
            break;

    }

    for (i = 0; i < e->nfilhos; i++)
        erros += verificar_expr(g, nt, e->filhos[i]);

    return erros;

}

int ll1_verificar(const Gramatica *g) {

    int i;
    
    int erros = 0;

    for (i = 0; i < g->nnts; i++)
        if (g->nts[i].regra != NULL)
            erros += verificar_expr(g, i, g->nts[i].regra);

    return erros;

}
