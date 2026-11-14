#include "tokens.h"

void initHashTable(void);
void destroyHashTable(void);

typedef struct {
    char *symbol;
    char *error_msg;
} YYSTYPE;

extern YYSTYPE microc_yylval;