%{
extern void yyerror(char *s, ...);
extern void emit(char *s, ...);
%}

%union {
    int intval;
    double floatval;
    char *strval;
}

    /* keywords */

%token ALTER
%token AND
%token AS
%token ASC
%token BETWEEN
%token BY
%token CREATE
%token DATABASE
%token DELETE
%token DESC
%token DROP
%token FROM
%token GROUP
%token HAVING
%token IN
%token INDEX
%token INSERT
%token INTO
%token IS
%token NOT
%token ORDER
%token SELECT
%token SET
%token TABLE
%token UPDATE
%token WHERE

    /* names and literal values */
%token <strval> NAME
%token <strval> STRING
%token <intval> INTNUM
%token <floatval> FLOATNUM


    /* precedence */
%start stmt_list

%%

stmt_list: stmt ';'
    | stmt_list stmt ';'
    ;

    /* statements: select statement */
stmt: select_stmt {}
    ;

opt_where: /* nil */
    | WHERE expr { emit("WHERE"); }
    ;

opt_groupby: /* nil */
    | GROUP BY groupby_list { emit("GROUP BY LIST %d", $3); }
    ;

groupby_list: groupby   { $$ = 1; }
    | groupby_list ',' groupby  { $$ = $1 + 1; }
    ;

    /* TODO: multiple nested NAMEs */
groupby: NAME   { emit("groupby %s", $1); }
    | NAME '.' NAME { emit("groupby %s.%s", $1, $3); }
    ;

opt_asc_desc: /* nil */ 
    | ASC               
    | DESC              
    ;

opt_limit: /* nil */
    | LIMIT expr { emit("LIMIT 1"); }
    | LIMIT expr ',' expr { emit("LIMIT 2"); }
    ;

opt_orderby: /* nil */
    | ORDER BY orderby_list { emit("ORDER BY LIST %d", $3); }
    ;

orderby_list: orderby { $$ = 1; }
    | orderby_list ',' orderby { $$ = $1 + 1; }
    ;

    /* TODO: multiple nested NAMEs */
orderby: NAME { emit("orderby %s", $1); }
    | NAME '.' NAME { emit("orderby %s.%s", $1, $3); }
    ;

opt_having: /* nil */
    | HAVING expr { emit("HAVING"); }
    ;

column_list: NAME { emit("COLUMN %s", $1); free($1); $$ = 1; }
    | column_list ',' NAME { emit("COLUMN %s", $3); free($3); $$ = $1 + 1; }
    ;

stmt: delete_stmt { emit("STMT"); }
    ;

    /* DELETE */
delete_stmt: DELETE FROM NAME 
    opt_where opt_orderby opt_limit { emit("DELETE %d %s", $2, $4); free($4); }
    ;

stmt: update_stmt { emit("STMT"); }
    ;

update_stmt: UPDATE 

stmt: insert_stmt { emit("STMT"); }
    ;

    /* INSERT */
insert_stmt: INSERT INTO NAME
    opt_col_names VALUES insert_vals_list
    ;

opt_col_names: /* nil */
    | '(' column_list ')' { emit("COLUMNS %d", $2); }
    ;

insert_vals_list: '(' insert_vals ')' { emit("VALUES %d", $2); $$ = 1; }  

stmt: update_stmt { emit("STMT"); }
    ;

    /* UPDATE */
update_stmt: UPDATE 


data_type:
    TINYINT 
    | SMALLINT
    | MEDIUMINT
    | INT
    | BIGINT
     

    /* expressions. TODO: multiple nested NAMEs */
expr: NAME      { emit("NAME %s", $1); free($1); }
    | NAME '.' NAME { emit("NAME %s.%s", $1, $3); free($1); free($3); }
    | STRING    { emit("STRING %s", $1); free($1); }
    | INTNUM    { emit("INT NUM %d", $1); }
    | FLOATNUM  { emit("FLOAT NUM %f", $1); }
    ;

expr: expr '+' expr { emit("ADD"); }
    | expr '-' expr { emit("SUB"); }
    | expr '*' expr { emit("MUL"); }
    | expr '/' expr { emit("DIV"); }
    | expr '%' expr { emit("MOD"); }
    | expr MOD expr { emit("MOD"); }
    | '-' expr %prec UMINUS { emit("NEG"); }
    | expr ANDOP expr { emit("AND"); }
    | expr OR expr { emit("OR"); }
    | expr XOR expr { emit("XOR"); }
    | expr COMPARISON expr { emit("CMP %d", $2); }
    | expr '|' expr { emit("BITOR"); }
    | expr '&' expr { emit("BITAND"); }
    | expr '^' expr { emit("BITXOR"); }
    | expr SHIFT expr { emit("SHIFT %s", $2 == 1 ? "LEFT" : "RIGHT"); }
    | NOT expr { emit("NOT"); }
    | '!' expr { emit("NOT"); }
    ;

expr: expr IS NULLX { emit("IS NULL"); }
    | expr IS NOT NULLX { emit("IS NOT NULL"); }
    ;

val_list: expr { $$ = 1; }
    | expr ',' val_list { $$ = $3 + 1; }
    ;

expr: expr IN '(' val_list ')'  { emit("IN %d", $4); }
    | expr NOT IN '(' val_list ')' { emit("NOT IN %d", $5); }
    ;

expr: expr LIKE expr { emit("LIKE"); }
    | expr NOT LIKE expr { emit("NOT LIKE"); }
    ;

