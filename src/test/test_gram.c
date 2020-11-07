#include <stdio.h>
#include <stdarg.h>


void emit(char *s, ...) {
    extern yylineno;
    va_list ap;
    va_start(ap, s);
    printf("rpn: ");
    vfprintf(stdout, s, ap);
    printf("\n");
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
    
}