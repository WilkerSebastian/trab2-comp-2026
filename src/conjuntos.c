/*
 * conjuntos.c - Calculo de anulavel, FIRST e FOLLOW (etapa 3).
 *
 * Fornecido: conj_iniciar e conj_imprimir.
 *
 * TODO(aluno): conj_anulavel, conj_first e conj_follow. Os tres sao
 * calculados por iteracao ate ponto fixo: repita uma passada sobre todas
 * as regras enquanto algum conjunto (ou flag) mudar. bitset_unir devolve
 * 1 quando o conjunto de destino muda, o que ajuda a detectar o ponto fixo.
 */
#include <stdio.h>
#include <string.h>
#include "conjuntos.h"
#include "util.h"

/* ------------------------------------------------------------------ */
/* Fornecido                                                          */
/* ------------------------------------------------------------------ */

static void iniciar_expr(Expr *e, void *ctx)
{
    int n = *(int *) ctx;
    bitset_liberar(&e->first);
    bitset_liberar(&e->follow);
    bitset_iniciar(&e->first, n);
    bitset_iniciar(&e->follow, n);
    e->anulavel = 0;
}

void conj_iniciar(Gramatica *g)
{
    int i, n = g->nterms;
    for (i = 0; i < g->nnts; i++) {
        bitset_liberar(&g->nts[i].first);
        bitset_liberar(&g->nts[i].follow);
        bitset_iniciar(&g->nts[i].first, n);
        bitset_iniciar(&g->nts[i].follow, n);
        g->nts[i].anulavel = 0;
        expr_percorrer(g->nts[i].regra, iniciar_expr, &n);
    }
}

void conj_imprimir(FILE *f, const Gramatica *g)
{
    int i;
    size_t largura = strlen("NAO TERMINAL");
    for (i = 0; i < g->nnts; i++)
        if (strlen(g->nts[i].nome) > largura)
            largura = strlen(g->nts[i].nome);
    fprintf(f, "%-*s  ANULAVEL  FIRST / FOLLOW\n", (int) largura, "NAO TERMINAL");
    for (i = 0; i < g->nnts; i++) {
        fprintf(f, "%-*s  %-8s  FIRST  = ", (int) largura, g->nts[i].nome,
                g->nts[i].anulavel ? "sim" : "nao");
        gram_imprimir_conjunto(f, g, &g->nts[i].first);
        fprintf(f, "\n%-*s            FOLLOW = ", (int) largura, "");
        gram_imprimir_conjunto(f, g, &g->nts[i].follow);
        fputc('\n', f);
    }
}

/* ------------------------------------------------------------------ */
/* A implementar                                                      */
/* ------------------------------------------------------------------ */

static void atualizar_anulavel(Expr *e, const Gramatica *g, int *mudou) {

    int i;
    
    int novo = 0;

    for (i = 0; i < e->nfilhos; i++)
        atualizar_anulavel(e->filhos[i], g, mudou);

    switch (e->tipo) {

        case E_TERM:

            novo = 0;

            break;

        case E_VAZIO:

            novo = 1;

            break;

        case E_NAOTERM:

            novo = g->nts[e->simbolo].anulavel;

            break;

        case E_SEQ:

            novo = 1;

            for (i = 0; i < e->nfilhos; i++)
                if (!e->filhos[i]->anulavel) {

                    novo = 0;

                    break;

                }

            break;

        case E_ALT:

            novo = 0;

            for (i = 0; i < e->nfilhos; i++)
                if (e->filhos[i]->anulavel) {

                    novo = 1;

                    break;

                }

            break;

        case E_OPC:
        case E_REP:

            novo = 1;

            break;

    }

    if (novo != e->anulavel) {

        e->anulavel = novo;

        *mudou = 1;

    }

}

void conj_anulavel(Gramatica *g) {

    int mudou = 1, i;

    while (mudou) {
 
        mudou = 0;
        
        for (i = 0; i < g->nnts; i++) {

            if (g->nts[i].regra == NULL)
                continue;

            atualizar_anulavel(g->nts[i].regra, g, &mudou);

            if (g->nts[i].regra->anulavel != g->nts[i].anulavel) {
    
                g->nts[i].anulavel = g->nts[i].regra->anulavel;

                mudou = 1;
    
            }
    
        }
    
    }

}

static void atualizar_first(Expr *e, const Gramatica *g, int *mudou) {

    int i;

    for (i = 0; i < e->nfilhos; i++)
        atualizar_first(e->filhos[i], g, mudou);

    switch (e->tipo) {

        case E_TERM:

            if (!bitset_tem(&e->first, e->simbolo)) {
            
                bitset_add(&e->first, e->simbolo);

                *mudou = 1;
            
            }

            break;

        case E_VAZIO:
            break;

        case E_NAOTERM:

            if (bitset_unir(&e->first, &g->nts[e->simbolo].first))
                *mudou = 1;

            break;

        case E_SEQ:

            for (i = 0; i < e->nfilhos; i++) {

                if (bitset_unir(&e->first, &e->filhos[i]->first))
                    *mudou = 1;

                if (!e->filhos[i]->anulavel)
                    break;

            }

            break;

        case E_ALT:
        case E_OPC:
        case E_REP:

            for (i = 0; i < e->nfilhos; i++)
                if (bitset_unir(&e->first, &e->filhos[i]->first))
                    *mudou = 1;

            break;

    }

}

void conj_first(Gramatica *g) {
    
    int mudou = 1;

    int i;

    while (mudou) {

        mudou = 0;

        for (i = 0; i < g->nnts; i++) {

            if (g->nts[i].regra == NULL)
                continue;

            atualizar_first(g->nts[i].regra, g, &mudou);

            if (bitset_unir(&g->nts[i].first, &g->nts[i].regra->first))
                mudou = 1;

        }

    }

}

static void propagar_follow(Expr *e, Gramatica *g, int *mudou) {

    int i;

    switch (e->tipo) {

        case E_SEQ: {

            Bitset tmp;
            
            int sufixo_anul = 1;
            
            Expr *fi;
            
            bitset_iniciar(&tmp, g->nterms);
            
            for (i = e->nfilhos - 1; i >= 0; i--) {

                fi = e->filhos[i];

                if (bitset_unir(&fi->follow, &tmp))
                    *mudou = 1;

                if (sufixo_anul && bitset_unir(&fi->follow, &e->follow))
                    *mudou = 1;

                if (!fi->anulavel)
                    bitset_copiar(&tmp, &fi->first);

                else
                    bitset_unir(&tmp, &fi->first);

                sufixo_anul = sufixo_anul && fi->anulavel;

            }

            bitset_liberar(&tmp);

            break;

        }

        case E_ALT:
        case E_OPC:

            for (i = 0; i < e->nfilhos; i++)
                if (bitset_unir(&e->filhos[i]->follow, &e->follow))
                    *mudou = 1;

            break;

        case E_REP:

            for (i = 0; i < e->nfilhos; i++) {

                if (bitset_unir(&e->filhos[i]->follow, &e->follow))
                    *mudou = 1;

                if (bitset_unir(&e->filhos[i]->follow, &e->filhos[i]->first))
                    *mudou = 1;

            }

            break;

        case E_NAOTERM:

            if (bitset_unir(&g->nts[e->simbolo].follow, &e->follow))
                *mudou = 1;

            break;

        case E_TERM:
        case E_VAZIO:
            break;
    }

    for (i = 0; i < e->nfilhos; i++)
        propagar_follow(e->filhos[i], g, mudou);

}

void conj_follow(Gramatica *g) {

    int mudou = 1, i;

    bitset_add(&g->nts[g->inicial].follow, TERM_FIM);

    while (mudou) {

        mudou = 0;

        for (i = 0; i < g->nnts; i++) {

            if (g->nts[i].regra == NULL)
                continue;

            if (bitset_unir(&g->nts[i].regra->follow, &g->nts[i].follow))
                mudou = 1;

            propagar_follow(g->nts[i].regra, g, &mudou);

        }

    }

}
