#include <stdio.h>
#include <stdarg.h>
#include "../parse/tokens.h"

YYSTYPE yylval;

int yylex(void);

char *token_names[] = {
    "ALTER",
    "AND",
    "AS",
    "ASC",
    "BETWEEN",
    "BY",
    "CREATE",
    "DATABASE",
    "DELETE",
    "DESC",
    "DROP",
    "FROM",
    "GROUP",
    "HAVING",
    "IN",
    "INDEX",
    "INSERT",
    "INTO",
    "IS",
    "NOT",
    "ORDER",
    "SELECT",
    "SET",
    "TABLE",
    "UPDATE",
    "WHERE",
    "INTNUM",
    "FLOATNUM",
    "STRING",
    "NAME"
};

char *token_name(int token) {
    return token < 257 || token > 286 ? "BAD_TOKEN" : token_names[token - 257];
}

void yyerror(char *s, ...) {
    extern yylineno;

    va_list ap;
    va_start(ap, s);

    fprintf(stderr, "%d: error: ", yylineno);
    vfprintf(stderr, s, ap);
    fprintf(stderr, "\n");
}

int main() {
    int token;
    while(1) {
        token = yylex();
        if (token == 0) {
            break;
        }
        switch (token) {
        case NAME: case STRING:
            printf("%10s %s\n", token_name(token), yylval.strval);
            break;
        case INTNUM:
            printf("%10s %d\n", token_name(token), yylval.intval);
        case FLOATNUM:
            printf("%10s %f\n", token_name(token), yylval.floatval);
        default:
            printf("%10s\n", token_name(token));
        }
    }
}