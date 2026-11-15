/*
 * microc.flex
 *
 * Esqueleto do analisador lexico (scanner) para a linguagem Micro C.
 * Disciplina: Compiladores I - FACOM
 *
 * Este arquivo NAO esta completo. Partes do reconhecimento de tokens
 * foram implementadas apenas como EXEMPLO, para orienta-lo(a) sobre o
 * padrao a seguir. As demais estao marcadas com "TODO(aluno)" e devem
 * ser completadas por voce.
 *
 * Compilacao:
 *      flex microc.flex
 *      gcc lex.yy.c -o lexer
 *
 * Uso:
 *      ./lexer test.mc
 */

%option prefix="microc_yy"
%option noyywrap

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include "tokens.h"
#include "tabela_strings.h"

/* ---------------------------------------------------------------------
 * Inicio implementação tabela hash
 * GitHub: https://github.com/WilkerSebastian/trabalho_ed_wilker
 * ------------------------------------------------------------------- */

#define HASH_MAX_SIZE 997

typedef struct StringNode {
    char *key;
    TokenType type;
} StringNode;

typedef struct {
    StringNode **table;
    uint32_t max;
    uint32_t length;
} HashTable;

static HashTable *hashTable = NULL;

static StringNode *createStringNode(const char *str, TokenType type) {

    StringNode *node = (StringNode*)malloc(sizeof(StringNode));
    node->key = strdup(str);
    node->type = type;

    return node;

}

static inline uint32_t firstIndexHash(uint64_t hash, uint32_t size) {

    return hash % size;

}

static inline uint32_t secondIndexHash(uint64_t preHash, uint64_t hash, uint32_t index, uint32_t size) {

    return (preHash + index * hash) % size;

}   

static uint64_t hashDJB2(const char *str) {
    
    uint64_t hash = 5381;
    
    int c;
    
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c;
    
    return hash;

}

static uint64_t hashSDBM(const char *str) {

    uint64_t hash = 0;

    int c;

    while ((c = *str++))
        hash = c + (hash << 6) + (hash << 16) - hash;
    
    return hash;

}

void initHashTable() {

    if (hashTable != NULL) 
        return;

    hashTable = (HashTable*)malloc(sizeof(HashTable));
    hashTable->max = HASH_MAX_SIZE;
    hashTable->length = 0;
    hashTable->table = (StringNode**)calloc(hashTable->max, sizeof(StringNode*));

    // ja inserindo as palavras reservadas na tabela
    
    const char *keywords[] = { "main", "if", "else", "for", "return", "int", "char", "print" };
    TokenType types[] = { MAIN,   IF,   ELSE,   FOR,   RETURN,   INT,   CHAR,   PRINT };

    for (int i = 0; i < sizeof(types) / sizeof(TokenType); i++) {

        uint64_t hash = hashDJB2(keywords[i]);

        uint32_t index = firstIndexHash(hash, hashTable->max);

        uint64_t hash2 = hashSDBM(keywords[i]);

        uint32_t iterations = 0;

        while (hashTable->table[index] != NULL && iterations < hashTable->max) {

            index = secondIndexHash(hash, hash2, iterations, hashTable->max);

            iterations++;

        }

        StringNode *node = createStringNode(keywords[i], types[i]);
        
        hashTable->table[index] = node;
        hashTable->length++;

    }

}

static StringNode *LookupInsertStr(const char *str) {

    if (hashTable == NULL) 
        initHashTable();

    uint64_t hash = hashDJB2(str);

    uint32_t index = firstIndexHash(hash, hashTable->max);

    uint64_t hash2 = hashSDBM(str);
    
    uint32_t iterations = 0;

    while (hashTable->table[index] != NULL && iterations < hashTable->max) {

        if (strcmp(hashTable->table[index]->key, str) == 0)
            return hashTable->table[index];

        index = secondIndexHash(hash, hash2, iterations, hashTable->max);

        iterations++;
        
    }

    StringNode *node = createStringNode(str, ID);  

    // fallback afinal vai que né
    if (iterations == hashTable->max)
        return node;

    hashTable->table[index] = node;
    hashTable->length++;

    return node;

}

void destroyHashTable() {

    if (hashTable == NULL) 
        return;

    for (uint32_t i = 0; i < hashTable->max; i++)
        if (hashTable->table[i] != NULL) {

            free(hashTable->table[i]->key);

            free(hashTable->table[i]);

        }
    
    free(hashTable->table);
    free(hashTable); 

    hashTable = NULL;

}

/* ---------------------------------------------------------------------
 * Fim implementação tabela hash
 * ------------------------------------------------------------------- */

/* Nomes dos tokens, usados apenas pelo main() de teste abaixo para
 * imprimir o tipo de cada token de forma legivel. Mantenha esta lista
 * na MESMA ORDEM do enum TokenType. */
static const char *nome_token[] = {
    "UNDEF", "ID", "END_OF_FILE",
    "INTEGERCONST", "CHARCONST", "STRINGCONST",
    "PLUS", "MINUS", "MUL", "DIV", "MOD",
    "EQ", "NEQ", "LT", "GT", "LEQ", "GEQ", "AND", "OR", "NOT",
    "ASSIGN", "SEMICOLON", "COMMA", "LPAREN", "RPAREN",
    "LBRACE", "RBRACE", "LBRACKET", "RBRACKET",
    "MAIN", "IF", "ELSE", "FOR", "RETURN", "INT", "CHAR", "PRINT"
};

/* Valor semantico do token corrente. */
YYSTYPE microc_yylval;

/* Linha atual do arquivo-fonte sendo processado. Deve ser incrementada
 * toda vez que uma quebra de linha for consumida pelo scanner (seja em
 * codigo "normal", dentro de comentarios ou dentro de strings). */
int linha_atual = 1;

// variaveis pra lidar com comentario
char bufferChar[1024];
int bufferIndex = 0;

/* Funcao auxiliar para preencher microc_yylval.symbol com uma copia do
 * texto reconhecido (yytext). Sinta-se livre para usar/adaptar. */
static TokenType guarda_lexema(void) {

    StringNode *node = LookupInsertStr(yytext);

    microc_yylval.symbol = node->key;

    return node->type;

}

%}

/* -----------------------------------------------------------------------
 * 2. SECAO DE DEFINICOES
 * ------------------------------------------------------------------- */

DIGIT       [0-9]
LETRA       [a-zA-Z_]
ALFANUM     [a-zA-Z0-9_]

%x COMMENT
%x STR_STATE
%x CHAR_STATE

%%

 /* -----------------------------------------------------------------------
  * 3. SECAO DE REGRAS
  * --------------------------------------------------------------------- */

 /* --- Fim de arquivo -----------------------------------------------------
  * Tratada explicitamente (em vez de depender do retorno automatico 0
  * do flex), pois o token UNDEF tambem vale 0 no enum TokenType -- se
  * dependessemos do comportamento padrao, um erro lexico seria
  * confundido com o fim do arquivo pelo main() de teste abaixo. */
<<EOF>>             { return END_OF_FILE; }

 /* --- Espacos em branco e quebras de linha ---------------------------- */
\n                  { linha_atual++; }
[ \t\r]+            { /* ignora espacos em branco */ }

 /* --- Comentarios ------------------------------------------------------
  * Estes ja estao implementados como exemplo de uso de estados (%x) e
  * de tratamento de erro via EOF dentro de um estado especial. */
"//".*              { /* comentario de linha: ignora ate o fim da linha */ }

"/*"                { BEGIN(COMMENT); }
<COMMENT>"*/"       { BEGIN(INITIAL); }
<COMMENT>\n         { linha_atual++; }
<COMMENT><<EOF>>    {
                        microc_yylval.error_msg = "EOF em comentario";
                        return UNDEF;
                    }
<COMMENT>.          { /* consome qualquer outro caractere dentro do comentario */ }

 /* Fechamento de comentario sem abertura correspondente. */
"*/"                {
                        microc_yylval.error_msg = "Comentario nao iniciado";
                        return UNDEF;
                    }

 /* --- Palavras reservadas e identificadores ---------------------------- */
{LETRA}{ALFANUM}*   {
                    
                        return guarda_lexema();

                    }

 /* --- Constantes inteiras ----------------------------------------------- */
{DIGIT}+            {

                        // os números negativos estão sendo tratados no '-'
    
                        guarda_lexema();
    
                        return INTEGERCONST;
                    
                    }

 /* --- Constantes de caractere -------------------------------------------- */
\'                  { 

                        BEGIN(CHAR_STATE); 
                        
                        bufferIndex = 0; 
                    
                    }

<CHAR_STATE>\\n     { bufferChar[bufferIndex++] = '\n'; }
<CHAR_STATE>\\t     { bufferChar[bufferIndex++] = '\t'; }
<CHAR_STATE>\\\\    { bufferChar[bufferIndex++] = '\\'; }
<CHAR_STATE>\\\'    { bufferChar[bufferIndex++] = '\''; }

<CHAR_STATE>\\0     { 

                        BEGIN(INITIAL);
                        
                        microc_yylval.error_msg = "String contem caractere nulo";

                        return UNDEF;
    
                    }

<CHAR_STATE>\'      { 

                        bufferChar[bufferIndex] = '\0';

                        BEGIN(INITIAL);

                        microc_yylval.symbol = LookupInsertStr(bufferChar)->key;

                        return CHARCONST; 

                    }

<CHAR_STATE>\n      { 

                        linha_atual++;

                        BEGIN(INITIAL);

                        microc_yylval.error_msg = "String nao terminada";

                        return UNDEF; 

                    }

<CHAR_STATE><<EOF>> { 

                        BEGIN(INITIAL);

                        microc_yylval.error_msg = "EOF em string";

                        return UNDEF; 
                        
                    }

<CHAR_STATE>.       { 
                        bufferChar[bufferIndex++] = yytext[0]; 
                    }

 /* --- Constantes de string ------------------------------------------- */

\"                  { 

                        BEGIN(STR_STATE); 

                        bufferIndex = 0; 
                    
                    }

<STR_STATE>\\n      { bufferChar[bufferIndex++] = '\n'; }
<STR_STATE>\\t      { bufferChar[bufferIndex++] = '\t'; }
<STR_STATE>\\\\     { bufferChar[bufferIndex++] = '\\'; }
<STR_STATE>\\\"     { bufferChar[bufferIndex++] = '\"'; }

<STR_STATE>\\0      {

                        BEGIN(INITIAL);
                        
                        microc_yylval.error_msg = "String contem caractere nulo";

                        return UNDEF;
                    
                    }
    
<STR_STATE>\"       { 
                        
                        bufferChar[bufferIndex] = '\0';
                        
                        BEGIN(INITIAL);

                        microc_yylval.symbol = LookupInsertStr(bufferChar)->key;
                        
                        return STRINGCONST; 

                    }
    
<STR_STATE>\n       { 

                        linha_atual++;

                        BEGIN(INITIAL);

                        microc_yylval.error_msg = "String nao terminada";

                        return UNDEF; 

                    }

<STR_STATE><<EOF>>  { 

                        BEGIN(INITIAL);

                        microc_yylval.error_msg = "EOF em string";

                        return UNDEF;

                    }

<STR_STATE>.        { 
                        bufferChar[bufferIndex++] = yytext[0]; 
                    }


 /* --- Operadores relacionais e logicos --------------------------------- */
"=="                { return EQ; }
"!="                { return NEQ; }
"<="                { return LEQ; }
">="                { return GEQ; }
"="                 { return ASSIGN; }
"<"                 { return LT; }
">"                 { return GT; }
"!"                 { return NOT; }
"&&"                { return AND; }
"||"                { return OR; }

 /* --- Operadores aritmeticos e simbolos de pontuacao (ja prontos) ------ */
"+"                 { return PLUS; }
"-"                 { 

                      int next_c = input();
                        
                      if (isdigit(next_c)) {

                        char buffer[22];
                            
                        int i = 0;
                            
                        buffer[i++] = '-';
                            
                        buffer[i++] = next_c;
                            
                        while (i < 22 && isdigit(next_c = input()))
                            buffer[i++] = next_c;
                            
                        buffer[i] = '\0';
                            
                        unput(next_c);
                            
                        microc_yylval.symbol = LookupInsertStr(buffer)->key;

                        return INTEGERCONST;
                            
                      }

                      unput(next_c);

                      yytext[1] = '\0'; 

                      return MINUS;

}
"*"                 { return MUL; }
"/"                 { return DIV; }
"%"                 { return MOD; }
";"                 { return SEMICOLON; }
","                 { return COMMA; }
"("                 { return LPAREN; }
")"                 { return RPAREN; }
"{"                 { return LBRACE; }
"}"                 { return RBRACE; }
"["                 { return LBRACKET; }
"]"                 { return RBRACKET; }

 /* --- Caractere invalido -------------------------------------------------
  * Casa com qualquer caractere que nao tenha correspondido a nenhuma
  * regra anterior. Deve ser SEMPRE a ultima regra do arquivo. */
.                   {
                        microc_yylval.error_msg = strdup(yytext);
                        return UNDEF;
                    }

%%