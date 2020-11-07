%option noyywrap  yylineno case-insensitive
%x COMMENT
%{
/*
 * lexical scanner for commands
 */ 
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "./tokens.h"

void yyerror(char *s, ...);

static int comment_level = 0;

extern char *parse_string;
extern char *parse_ch;
extern YYSTYPE yylval;
%}


%%
    /* keywords */
ALTER                               { return ALTER; }
AND                                 { return AND; }
AS                                  { return AS; }
ASC                                 { return ASC; }
BETWEEN                             { return BETWEEN; }
BY                                  { return BY; }
CREATE                              { return CREATE; }
DATABASE                            { return DATABASE; }
DELETE                              { return DELETE; }
DESC                                { return DESC; }
DROP                                { return DROP; }
FROM                                { return FROM; }
GROUP                               { return GROUP; }
HAVING                              { return HAVING; }
IN                                  { return IN; }
INDEX                               { return INDEX; }
INSERT                              { return INSERT; }
INTO                                { return INTO; }
IS                                  { return IS; }
NOT                                 { return NOT; }
ORDER                               { return ORDER; }
SELECT                              { return SELECT; }
SET                                 { return SET; }
TABLE                               { return TABLE; }
UPDATE                              { return UPDATE; }
WHERE                               { return WHERE; }

         
    /* integer */
-?[0-9]+                            { yylval.intval = atoi(yytext); return INTNUM; }
    /* float */
-?[0-9]+"."[0-9]* |
-?"."[0-9]+ |
-?[0-9]+E[-+]?[0-9]+ |
-?[0-9]+"."[0-9]*E[-+]?[0-9]+ |
-?"."[0-9]*E[-+]?[0-9]+             { yylval.floatval = atof(yytext); return FLOATNUM; }
    /* hex strings */
X'[0-9A-F]+' |
0X[0-9A-F]+                         { yylval.strval = strdup(yytext); return STRING; }
    /* bit strings */
0B[01]+ |
B'[01]+'                            { yylval.strval = strdup(yytext); return STRING; }

[A-Za-z][A-Za-z0-9_]*               { yylval.strval = strdup(yytext); return NAME; }
"/*"                                { comment_level += 1; BEGIN COMMENT; }
"*/"                                { /* error occurs */ }
<COMMENT>{
    "/*"        { comment_level += 1; }
    "*/"        { if (--comment_level == 0) BEGIN INITIAL; }
    (\n|\r\n)   { /* new line */ }
    .           { /* ignore */ }
    <<EOF>>     { yyerror("unclosed comment"); }
}
    /* else */
[ \t\n]                             { /* white space, do nothing */ }
.                                   { yyerror("unknown character '%c'", *yytext); }

%%

/*
#if defined(yywrap)
#undef yywrap
int yywrap() {
    return 1;
}
#endif  // yywrap

// called by parser
void init_io() {
    parse_ch = NULL;
    BEGIN INITIAL;
}

#if defined(input)
#undef input
#endif  // input

// get input from a string
int input() {
    if (parse_ch == NULL) {
        parse_ch = parse_string;
        return (*parse_ch++)
    } else if (*parse_ch == '\0') {
        return 0;
    } else {
        return (*parse_ch++);
    }
}

#if defined(unput)
#undef unput
#endif  // unput

// undo lex input from a string 
void unput(char c) {
    if (parse_ch == NULL) {
        // error
    } else if (c != 0) {
        *--parse_ch = c;
    }
}

// read input from a string
int myinput(char *buf, int max) {
    int len, copylen;
    if (parse_ch == NULL) {
        len = strlen(parse_string);
        if (len >= max) {
            copylen = max - 1;
        } else {
            copylen = len;
        }
        if (copylen > 0) {
            memcpy(buf, parse_string, copylen);
        }
        buf[copylen] = '\0';
        parse_ch = parse_string;
        return copylen;
    } else {
        return 0;
    }
}

*/
