/*
 * microc_mapa.c - Correspondencia com os tokens de Micro C (ver microc_mapa.h).
 *
 * A tabela de literais segue o tokens.h do livro-texto (secao 2.3.1):
 *
 *   UNDEF, ID, END_OF_FILE, INTEGERCONST, CHARCONST, STRINGCONST,
 *   PLUS, MINUS, MUL, DIV, MOD, EQ, NEQ, LT, GT, LEQ, GEQ, AND, OR, NOT,
 *   ASSIGN, SEMICOLON, COMMA, LPAREN, RPAREN, LBRACE, RBRACE,
 *   LBRACKET, RBRACKET, MAIN, IF, ELSE, FOR, RETURN, INT, CHAR, PRINT
 *
 * Se o seu tokens.h do TP1 usar outros nomes, ajuste a tabela.
 *
 * TODO(aluno): implementar microc_carregar_tokens_h.
 */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "microc_mapa.h"
#include "util.h"

static const char *const literais[][2] = {
    { "+",  "PLUS" },      { "-",  "MINUS" },     { "*",  "MUL" },
    { "/",  "DIV" },       { "%",  "MOD" },
    { "==", "EQ" },        { "!=", "NEQ" },       { "<",  "LT" },
    { ">",  "GT" },        { "<=", "LEQ" },       { ">=", "GEQ" },
    { "&&", "AND" },       { "||", "OR" },        { "!",  "NOT" },
    { "=",  "ASSIGN" },    { ";",  "SEMICOLON" }, { ",",  "COMMA" },
    { "(",  "LPAREN" },    { ")",  "RPAREN" },    { "{",  "LBRACE" },
    { "}",  "RBRACE" },    { "[",  "LBRACKET" },  { "]",  "RBRACKET" },
    { "main", "MAIN" },    { "if", "IF" },        { "else", "ELSE" },
    { "for", "FOR" },      { "return", "RETURN" },{ "int", "INT" },
    { "char", "CHAR" },    { "print", "PRINT" }
};

static char **nomes = NULL;
static int nnomes = 0, capnomes = 0;

const char *microc_nome_literal(const char *literal)
{
    size_t i;
    for (i = 0; i < sizeof(literais) / sizeof(literais[0]); i++)
        if (strcmp(literais[i][0], literal) == 0)
            return literais[i][1];
    return NULL;
}

static void adicionar_nome(const char *s, size_t n)
{
    if (nnomes == capnomes) {
        capnomes = capnomes ? 2 * capnomes : 64;
        nomes = (char **) xrealloc(nomes, (size_t) capnomes * sizeof(char *));
    }
    nomes[nnomes++] = xstrndup(s, n);
}

static void pular_branco_comentario_tokens(const char **pp) {

    const char *p = *pp;
    
    while (1) {

        while (isspace((unsigned char) *p))
            p++;

        if (p[0] == '/' && p[1] == '*') {
            
            p += 2;
            
            while (*p != '\0' && !(p[0] == '*' && p[1] == '/'))
                p++;
            
            if (*p == '\0')
                break;
            
            p += 2;
            
            continue;
        
        }

        if (p[0] == '/' && p[1] == '/') {
    
            while (*p != '\0' && *p != '\n')
                p++;
    
            continue;
        
        }
    
        break;
    
    }

    *pp = p;

}

int microc_carregar_tokens_h(const char *caminho) {
    
    char *fonte;
    
    const char *p;

    if (caminho == NULL)
        return -1;
    
    fonte = ler_arquivo(caminho);
    
    if (fonte == NULL) {
        
        diag_aviso(0, 0, "nao foi possivel abrir %s: nomes de TokenType nao verificados", caminho);
    
        return -1;
    
    }

    p = strstr(fonte, "typedef enum");
    
    if (p != NULL)
        p = strchr(p, '{');
    
    if (p == NULL) {
        
        diag_aviso(0, 0, "enumeracao TokenType nao encontrada em %s", caminho);
        
        free(fonte);
        
        return -1;
    
    }

    p++;

    while (1) {

        pular_branco_comentario_tokens(&p);
        
        if (*p == '\0' || *p == '}')
            break;
        
        if (isalpha((unsigned char) *p) || *p == '_') {
        
            const char *ini = p;
        
            while (isalnum((unsigned char) *p) || *p == '_')
                p++;
        
            adicionar_nome(ini, (size_t) (p - ini));

        } 
        else
            p++;

    }

    free(fonte);

    if (nnomes == 0) {

        diag_aviso(0, 0, "nenhum nome lido da enumeracao TokenType em %s", caminho);
        
        return -1;

    }

    return 0;

}

int microc_token_existe(const char *nome)
{
    int i;
    for (i = 0; i < nnomes; i++)
        if (strcmp(nomes[i], nome) == 0)
            return 1;
    return 0;
}

void microc_liberar(void)
{
    int i;
    for (i = 0; i < nnomes; i++)
        free(nomes[i]);
    free(nomes);
    nomes = NULL;
    nnomes = capnomes = 0;
}
