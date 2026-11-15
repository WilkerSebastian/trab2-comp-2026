/*
 * verificacao.c - Verificacoes semanticas da gramatica (etapa 2).
 *
 * Ja implementados (use como modelo):
 *   - verif_indefinidos: nao terminais usados mas nao definidos;
 *   - verif_terminais:   terminais nomeados sem declaracao.
 *
 * TODO(aluno):
 *   - verif_inalcancaveis      (aviso)
 *   - verif_improdutivos       (erro)
 *   - verif_recursao_esquerda  (erro; direta e indireta)
 *
 * A duplicidade de definicoes ja e detectada durante a leitura
 * (ebnf_parser.c).
 */
#include <stdlib.h>
#include <string.h>
#include "verificacao.h"
#include "microc_mapa.h"
#include "util.h"

/* ------------------------------------------------------------------ */
/* Fornecido                                                          */
/* ------------------------------------------------------------------ */

static int verif_indefinidos(Gramatica *g)
{
    int i, erros = 0;
    for (i = 0; i < g->nnts; i++) {
        if (g->nts[i].regra == NULL) {
            diag_erro(g->nts[i].linha, g->nts[i].coluna,
                      "nao terminal %s usado mas nao definido", g->nts[i].nome);
            erros++;
        }
    }
    return erros;
}

static int verif_terminais(Gramatica *g, int modo_microc)
{
    int i, erros = 0, carregado = 0;
    Terminal *t;

    if (modo_microc)
        carregado = microc_carregar_tokens_h(g->tokens_externos) == 0;

    for (i = 1; i < g->nterms; i++) {
        t = &g->terms[i];
        if (modo_microc) {
            if (t->literal && microc_nome_literal(t->nome) == NULL) {
                diag_erro(t->linha, t->coluna, "literal %s nao corresponde a nenhum "
                          "token de Micro C", t->exibicao);
                erros++;
            } else if (!t->literal && carregado && !microc_token_existe(t->nome)) {
                diag_erro(t->linha, t->coluna, "terminal %s nao existe na enumeracao "
                          "TokenType de %s", t->nome, g->tokens_externos);
                erros++;
            }
        } else if (!t->declarado) {
            diag_erro(t->linha, t->coluna, "terminal %s nao declarado em TOKENS", t->nome);
            erros++;
        }
    }
    return erros;
}

/* ------------------------------------------------------------------ */
/* A implementar                                                      */
/* ------------------------------------------------------------------ */

static void marcar_na_regra(const Expr *e, int *marcado, int *mudou) {

    int i;
    
    if (e == NULL)
        return;
    
    if (e->tipo == E_NAOTERM && !marcado[e->simbolo]) {
    
        marcado[e->simbolo] = 1;
    
        *mudou = 1;
    
    }

    for (i = 0; i < e->nfilhos; i++)
        marcar_na_regra(e->filhos[i], marcado, mudou);

}

static int tem_ciclo(const Gramatica *g, Bitset *primeiros, int origem) {

    int *visitado = (int*)xcalloc((size_t) g->nnts, sizeof(int));
    
    int *fila = (int*)xmalloc((size_t) g->nnts * sizeof(int));

    int ini = 0;

    int fim = 0;

    int t;

    int achou = 0;

    for (t = bitset_proximo(&primeiros[origem], 0); t >= 0; t = bitset_proximo(&primeiros[origem], t + 1)) {
        
        if (t == origem) { 
            
            achou = 1; 
    
            break; 

        }

        if (!visitado[t]) { 

            visitado[t] = 1;

            fila[fim++] = t; 

        }

    }

    while (ini < fim && !achou) {

        int atual = fila[ini++];

        for (t = bitset_proximo(&primeiros[atual], 0); t >= 0; t = bitset_proximo(&primeiros[atual], t + 1)) {

            if (t == origem) { 
                
                achou = 1; 
                
                break; 

            }

            if (!visitado[t]) {

                visitado[t] = 1;
                
                fila[fim++] = t;

            }

        }

    }

    free(visitado);

    free(fila);

    return achou;

}

static int expr_produtiva(const Expr *e, const int *produtivo) {

    int i;

    switch (e->tipo) {

        case E_TERM:
        case E_VAZIO:
            return 1;

        case E_NAOTERM:
            return produtivo[e->simbolo];

        case E_SEQ:

            for (i = 0; i < e->nfilhos; i++)
                if (!expr_produtiva(e->filhos[i], produtivo))
                    return 0;

            return 1;

        case E_ALT:

            for (i = 0; i < e->nfilhos; i++)
                if (expr_produtiva(e->filhos[i], produtivo))
                    return 1;

            return 0;

        case E_OPC:
        case E_REP:
            return 1;

    }

    return 0;

}

static void coletar_primeiros(const Expr *e, Bitset *resultado) {

    int i;

    switch (e->tipo) {

        case E_TERM:
        case E_VAZIO:
            break;

        case E_NAOTERM:

            bitset_add(resultado, e->simbolo);

            break;

        case E_SEQ:

            for (i = 0; i < e->nfilhos; i++) {

                coletar_primeiros(e->filhos[i], resultado);

                if (!e->filhos[i]->anulavel)
                    break;

            }

            break;

        case E_ALT:

            for (i = 0; i < e->nfilhos; i++)
                coletar_primeiros(e->filhos[i], resultado);

            break;

        case E_OPC:
        case E_REP:

            coletar_primeiros(e->filhos[0], resultado);

            break;

    }

}

static int verif_inalcancaveis(Gramatica *g) {

    int *marcado = (int *) xcalloc((size_t) g->nnts, sizeof(int));
    
    int mudou = 1, i;

    marcado[g->inicial] = 1;
    
    while (mudou) {
    
        mudou = 0;
    
        for (i = 0; i < g->nnts; i++)
            if (marcado[i] && g->nts[i].regra != NULL)
                marcar_na_regra(g->nts[i].regra, marcado, &mudou);
    
    }

    for (i = 0; i < g->nnts; i++)
        if (!marcado[i] && g->nts[i].regra != NULL)
            diag_aviso(g->nts[i].linha_def, g->nts[i].coluna, "nao terminal %s inalcancavel a partir de %s", g->nts[i].nome, g->nts[g->inicial].nome);

    free(marcado);

    return 0;

}

static int verif_improdutivos(Gramatica *g) {

    int *produtivo = (int*)xcalloc((size_t) g->nnts, sizeof(int));
    
    int mudou = 1;
    
    int i;
    
    int erros = 0;

    while (mudou) {
    
        mudou = 0;
    
        for (i = 0; i < g->nnts; i++)
            if (!produtivo[i] && g->nts[i].regra != NULL && expr_produtiva(g->nts[i].regra, produtivo)) {
                
                produtivo[i] = 1;
                
                mudou = 1;
            
            }

    }

    for (i = 0; i < g->nnts; i++)
        if (!produtivo[i] && g->nts[i].regra != NULL) {

            diag_erro(g->nts[i].linha_def, g->nts[i].coluna, "nao terminal %s nao deriva nenhuma cadeia de terminais", g->nts[i].nome);

            erros++;

        }

    free(produtivo);

    return erros;

}

int verif_recursao_esquerda(Gramatica *g) {

    Bitset *primeiros = (Bitset*)xcalloc((size_t) g->nnts, sizeof(Bitset));
    
    int i; 
    
    int erros = 0;

    for (i = 0; i < g->nnts; i++) {

        bitset_iniciar(&primeiros[i], g->nnts);

        if (g->nts[i].regra != NULL)
            coletar_primeiros(g->nts[i].regra, &primeiros[i]);

    }

    for (i = 0; i < g->nnts; i++) {

        if (g->nts[i].regra == NULL)
            continue;
        
        if (tem_ciclo(g, primeiros, i)) {
        
            int direta = bitset_tem(&primeiros[i], i);
        
            diag_erro(g->nts[i].linha_def, 0, "nao terminal %s tem recursao a esquerda %s", g->nts[i].nome, direta ? "direta" : "indireta");

            erros++;

        }

    }

    for (i = 0; i < g->nnts; i++)
        bitset_liberar(&primeiros[i]);

    free(primeiros);

    return erros;

}

int verif_basica(Gramatica *g, int modo_microc)
{
    int erros = 0;
    if (g->inicial < 0) {
        diag_erro(0, 0, "a gramatica nao tem nenhuma regra");
        return 1;
    }
    erros += verif_indefinidos(g);
    erros += verif_terminais(g, modo_microc);
    erros += verif_inalcancaveis(g);
    if (erros == 0)
        erros += verif_improdutivos(g);
    return erros;
}
