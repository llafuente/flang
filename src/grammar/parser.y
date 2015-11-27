%{
  #define YYDEBUG 1

  #include "flang.h"

  extern array* identifiers;
  extern int yylex();
  extern int yyget_lineno();
  extern char* yytext;
  void yyerror(ast_t** root, const char *s) {
    ast_t* err = ast_mk_error(s, "syntax");
    ast_position(err, yylloc, yylloc);
    (*root)->program.body = err;

    printf("%s at [%d:%d]\n%s\n", s, yylloc.first_line, yylloc.first_column, yytext);
  }
%}

%debug

%locations

%define parse.error verbose

%parse-param {ast_t **root}

/* Represents the many different ways we can access our data */
%union {
  ast_t* node;
  string* string;
  int token;
  bool test;
}

/* Define our terminal symbols (tokens). This should
   match our tokens.l lex file. We also define the node type
   they represent.
 */

%token <token> TK_EQEQ TK_NE TK_LE TK_SHL TK_SHLEQ
%token <token> TK_GE TK_SHR TK_SHREQ TK_RARROW TK_MINUSEQ
%token <token> TK_ANDAND TK_ANDEQ TK_OROR TK_OREQ TK_PLUSEQ
%token <token> TK_STAREQ TK_SLASHEQ TK_CARETEQ TK_PERCENTEQ
%token <token> TK_DOTDOTDOT TK_DOTDOT TK_PLUSPLUS TK_MINUSMINUS
%token <token> TK_CAST


/* keywords */
%token <token> TK_UNDERSCORE TK_AS TK_ANY TK_BREAK TK_CONST
%token <token> TK_CONTINUE TK_ELSE TK_ENUM TK_FN
%token <token> TK_FFI TK_FOR TK_IF TK_IN TK_MATCH
%token <token> TK_IMPORT TK_PUB TK_REF TK_RETURN TK_STATIC
%token <token> TK_STRUCT TK_TYPE TK_TYPEOF TK_USE
%token <token> TK_VAR TK_WHERE TK_WHILE TK_DO TK_SIZEOF

%token <token> TK_FALSE TK_TRUE TK_NULL

/* reserved but maybe not be used ever ^^ */
%token <token> TK_FAT_ARROW

/* ast */
/* ast.literals */
%token <node> AST_LIT_ID AST_LIT_INTEGER AST_LIT_FLOAT AST_LIT_STR AST_LIT_BYTE

%token <node> AST_IDENT
%token <node> AST_SHEBANG
%token <node> AST_SHEBANG_LINE

%token <node> AST_COMMENT
%token <node> AST_LIT_BYTE_STR

/* to remove! */
%token <node> AST_LIT_CHAR AST_LIT_BYTE_STR_RAW AST_STATIC_LIFETIME AST_LIFETIME

/* Define the type of node our nonterminal symbols represent.
   The types refer to the %union declaration above. Ex: when
   we call an ident (defined by union type ident) we are really
   calling an (NIdentifier*). It makes the compiler happy.
 */
%type <node> program stmts
%type <node> stmt var_decl maybe_argument_expression_list argument_expression_list comment
%type <node> struct_decl struct_decl_fields struct_decl_fields_list struct_decl_field
%type <node> block maybe_stmts

%type <node> expression maybe_expression
%type <node> assignment_expression
%type <node> conditional_expression
%type <node> logical_or_expression
%type <node> logical_and_expression
%type <node> inclusive_or_expression
%type <node> exclusive_or_expression
%type <node> and_expression
%type <node> equality_expression
%type <node> relational_expression
%type <node> shift_expression
%type <node> additive_expression
%type <node> multiplicative_expression
%type <node> cast_expression
%type <node> unary_expression
%type <node> postfix_expression
%type <node> primary_expression


/* TODO this should be a stmt ? */
%type <node> expr_if block_or_if
/* loops */
%type <node> expr_while expr_dowhile expr_for import_stmt
%type <node> fn_decl fn_parameters fn_parameter_list fn_parameter

%type <token> assignament_operator equality_operator relational_operator additive_operator multiplicative_operator unary_operator

/* terminals */
%type <node> type ty_primitive ident literal lit_integer lit_numeric lit_string

/* %expect 0 */

%precedence IDENT
%precedence TYPE

%precedence FORTYPE
%precedence FOR

%nonassoc DOTDOT

%precedence RETURN

%left '=' TK_SHLEQ TK_SHREQ TK_MINUSEQ TK_ANDEQ TK_OREQ TK_PLUSEQ TK_STAREQ TK_SLASHEQ TK_CARETEQ TK_PERCENTEQ
%precedence ASSIGNAMENT

%left TK_OROR
%left TK_ANDAND

%left TK_EQEQ TK_NE
%precedence EQUALITY

%left '<' '>' TK_LE TK_GE
%precedence COMPARISON

%left '|'
%left '^'
%left '&'
%left TK_SHL TK_SHR
%left '+' '-'
%precedence ADDITION

%precedence MULTIPLICATION
%left '*' '/' '%'

%precedence '!'

%precedence '{' '[' '(' '.'

/* %precedence RANGE */

%start program

%%

/* todo module definition */
program
  : stmts {
    //printf("stmts addr: %p\n", $1);

    ast_t* block = ast_mk_block($1);
    ast_position(block, @1, @1);
    (*root)->program.body = block;
  }
  ;

stmts
  : stmt {
    //printf("create stmt list\n");
    // create and push
    $$ = ast_mk_list();
    ast_mk_list_push($$, $1);
    ast_position($1, @1, @1);
  }
  | stmts stmt {
    //printf("create stmt\n");
    // push
    if ($2) { // do not push empty stmts
      ast_position($2, @2, @2);
      ast_mk_list_push($1, $2);
    }
    $$ = $1;
  }
  ;

maybe_stmts
  : stmts
  /*
  | stmts nonblock_expr { $$ = ext_node($1, 1, $2); }
  | nonblock_expr
  */
  | %empty              { $$ = ast_mk_list(); }
  ;

stmt
  : ';' {
    $$ = 0; // empty stmt
  }
  | var_decl ';'
  | struct_decl
  | fn_decl
  | '{' stmts '}' {
    $$ = ast_mk_block($2);
  }
  | expression ';'
  | comment
  | expr_if
  | expr_while
  | expr_dowhile
  | expr_for
  | import_stmt ';'
  | TK_CONTINUE ';' {
    $$ = ast_mk_continue(0);
    ast_position($$, @1, @2);
  }
  | TK_CONTINUE ident ';' {
  $$ = ast_mk_continue($2);
  ast_position($$, @1, @3);
  }
  | TK_RETURN ';' {
    $$ = ast_mk_return(0);
    ast_position($$, @1, @2);
  }
  | TK_RETURN expression ';' {
    $$ = ast_mk_return($2);
    ast_position($$, @1, @3);
  }
  | TK_BREAK ';' {
    $$ = ast_mk_break(0);
    ast_position($$, @1, @2);
  }
  | TK_BREAK ident ';' {
    $$ = ast_mk_break($2);
    ast_position($$, @1, @3);
  }
  ;

import_stmt
  : TK_IMPORT lit_string {
    $$ = ast_mk_import($2);
    ast_position($$, @1, @2);
  }
  ;

var_decl
  : TK_VAR type ident '=' expression {
    $$ = ast_mk_list();
    ast_t* decl = ast_mk_var_decl($2, $3);
    ast_position(decl, @1, @3);
    ast_mk_list_push($$, decl);
    ast_t* assignament = ast_mk_assignament(ast_clone(decl->var.id), '=', $5);
    ast_position(assignament, @3, @5);
    ast_mk_list_push($$, assignament);
    ast_position($$, @1, @5);
  }
  | TK_VAR ident '=' expression {
    $$ = ast_mk_list();
    ast_t* decl = ast_mk_var_decl(0, $2);
    ast_position(decl, @1, @2);
    ast_mk_list_push($$, decl);
    ast_t* assignament = ast_mk_assignament(ast_clone(decl->var.id), '=', $4);
    ast_position(assignament, @3, @4);
    ast_mk_list_push($$, assignament);
    ast_position($$, @1, @4);
  }
  | TK_VAR type ident {
    $$ = ast_mk_var_decl($2, $3);
    ast_position($$, @1, @3);
  }
  | TK_VAR ident {
    $$ = ast_mk_var_decl(0, $2);
    ast_position($$, @1, @2);
  }
  ;

struct_decl
  : TK_STRUCT ident struct_decl_fields {
    $$ = ast_mk_struct_decl($2, $3);
    ast_position($$, @1, @3);
  }
  ;

struct_decl_fields
  : '{' struct_decl_fields_list '}'      { $$ = $2; ast_position($$, @1, @3); }
  | '{' struct_decl_fields_list ',' '}'  { $$ = $2; ast_position($$, @1, @4); }
  ;

struct_decl_fields_list
  : struct_decl_field {
    $$ = ast_mk_list();
    ast_mk_list_push($$, $1);
    ast_position($$, @1, @1);
  }
  | struct_decl_fields_list ',' struct_decl_field {
    ast_mk_list_push($1, $3);
    ast_position($3, @3, @3);
    $$ = $1;
  }
  | %empty { yyerror(root, "syntax error, empty struct declaration"); YYERROR; }
  ;

struct_decl_field
  : type ident { $$ = ast_mk_struct_decl_field($2, $1); ast_position($$, @1, @2); }
  | ident      { $$ = ast_mk_struct_decl_field($1, 0);  ast_position($$, @1, @1); }

fn_decl
  /* TODO force parameter with types! */
  : TK_FFI TK_FN ident fn_parameters ':' type {
    $$ = ast_mk_fn_decl($3, $4, $6, 0);
    $$->func.ffi = true;
    if ($4->parent == 1) {
      $$->func.varargs = true;
    }
    ast_position($$, @1, @6);
  }
  | TK_FN ident fn_parameters ':' type block {
    $$ = ast_mk_fn_decl($2, $3, $5, $6);
    if ($3->parent == 1) {
      $$->func.varargs = true;
    }
    ast_position($$, @1, @6);
  }
  | TK_FN ident fn_parameters block {
    $$ = ast_mk_fn_decl($2, $3, 0, $4);
    if ($3->parent == 1) {
      $$->func.varargs = true;
    }
    ast_position($$, @1, @4);
  }
  ;

fn_parameters
  : %empty                          { $$ = ast_mk_list(); ast_position($$, @0, @0); }
  | '(' ')'                         { $$ = ast_mk_list(); ast_position($$, @1, @2); }
  | '(' fn_parameter_list ')'                  { $$ = $2; }
  | '(' fn_parameter_list ',' ')'              { $$ = $2; }
  | '(' fn_parameter_list ',' TK_DOTDOTDOT ')' {
    $$ = $2;
    // TODO more elegant way?
    // use parent as hack
    $$->parent = 1;
  }
  ;

fn_parameter_list
  : fn_parameter {
    $$ = ast_mk_list();
    ast_mk_list_push($$, $1);
    ast_position($$, @1, @1);
  }
  | fn_parameter_list ',' fn_parameter {
    ast_mk_list_push($1, $3);
    ast_position($3, @3, @3);
    $$ = $1;
  }
  ;

  /* TODO default! */
fn_parameter
  : type ident {
    $$ = ast_mk_fn_param($2, $1, 0);
    ast_position($$, @1, @2);
  }
  | ident {
    $$ = ast_mk_fn_param($1, 0, 0);
    ast_position($$, @1, @1);
  }
  ;

assignament_operator
  : '='          { $$ = '='; }
  | TK_SHLEQ     { $$ = TK_SHLEQ; }
  | TK_SHREQ     { $$ = TK_SHREQ; }
  | TK_MINUSEQ   { $$ = TK_MINUSEQ; }
  | TK_ANDEQ     { $$ = TK_ANDEQ; }
  | TK_OREQ      { $$ = TK_OREQ; }
  | TK_PLUSEQ    { $$ = TK_PLUSEQ; }
  | TK_STAREQ    { $$ = TK_STAREQ; }
  | TK_SLASHEQ   { $$ = TK_SLASHEQ; }
  | TK_CARETEQ   { $$ = TK_CARETEQ; }
  | TK_PERCENTEQ { $$ = TK_PERCENTEQ; }
  ;

equality_operator
  : TK_EQEQ      { $$ = TK_EQEQ; }
  | TK_NE        { $$ = TK_NE; }
  ;

relational_operator
  : '<'          { $$ = '<'; }
  | '>'          { $$ = '>'; }
  | TK_LE        { $$ = TK_LE; }
  | TK_GE        { $$ = TK_GE; }

additive_operator
  : '+'          { $$ = '+'; }
  | '-'          { $$ = '-'; }
  ;

multiplicative_operator
  : '*'          { $$ = '*'; }
  | '/'          { $$ = '/'; }
  | '%'          { $$ = '%'; }
  ;

unary_operator
  : '-' { $$ = '-'; }
  | '!' { $$ = '!'; }
  | '*' { $$ = '*'; }
  | '&' { $$ = '&'; }
  ;

primary_expression
  : literal {
    $$ = $1;
    ast_position($$, @1, @1);
  }
  | '(' maybe_expression ')'                            {
    $$ = $2;
    ast_position($$, @1, @3); // override position to include parens
  }
  | '@' block {
    // TODO short function decl
    // TODO move. this prio?
    fl_fatal_error("%s", "not implemented");
  }
  ;

postfix_expression
  : primary_expression
  | postfix_expression '[' expression ']' {
    /* TODO maybe_expr? */
    $$ = ast_mk_member($1, $3, true);
    ast_position($$, @1, @4);
  }
  | postfix_expression '[' lit_integer ']' {
    /* TODO maybe_expr? */
    $$ = ast_mk_member($1, $3, false);
    ast_position($$, @1, @4);
  }
  | postfix_expression '(' maybe_argument_expression_list ')' {
    // TODO this cannot be assignament lhs, right ?
    $$ = ast_mk_call_expr($1, $3);
    ast_position($$, @1, @4);
  }
  | postfix_expression '.' ident {
    $$ = ast_mk_member($1, $3, false);
    ast_position($$, @1, @3);
  }
  | postfix_expression '.' lit_integer {
    $$ = ast_mk_member($1, $3, false);
    ast_position($$, @1, @3);
  }
  | postfix_expression TK_PLUSPLUS   { $$ = ast_mk_runary($1, TK_PLUSPLUS); ast_position($$, @1, @2); }
  | postfix_expression TK_MINUSMINUS { $$ = ast_mk_runary($1, TK_MINUSMINUS); ast_position($$, @1, @2); }
  /* TODO really? not needed!
  | postfix_expression PTR_OP IDENT '->'
  */
  ;

unary_expression
  : postfix_expression             { $$ = $1; }
  | TK_PLUSPLUS unary_expression   { $$ = ast_mk_lunary($2, TK_PLUSPLUS); ast_position($$, @1, @2); }
  | TK_MINUSMINUS unary_expression { $$ = ast_mk_lunary($2, TK_MINUSMINUS); ast_position($$, @1, @2); }
  | unary_operator cast_expression { $$ = ast_mk_lunary($2, $1); ast_position($$, @1, @2); }
  | TK_ANDAND cast_expression {
    ast_t* lun = ast_mk_lunary($2, '&');
    ast_position(lun, @1, @2);

    $$ = ast_mk_lunary(lun, '&');
    ast_position($$, @1, @2);
  }
  /*
    TODO sizeof variable?
    | SIZEOF unary_expression
  */
  | TK_SIZEOF '(' type ')' {
    $$ = ast_mk_sizeof($3);
    ast_position($$, @1, @4);
  }
  ;

cast_expression
  : unary_expression
  | TK_CAST '(' type ')' cast_expression {
    $$ = ast_mk_cast($3, $5);
    ast_position($$, @1, @5);
  }
  ;

multiplicative_expression
  : cast_expression
  | multiplicative_expression multiplicative_operator cast_expression {
    $$ = ast_mk_binop($1, $2, $3);
    ast_position($$, @1, @3);
  }
  ;

additive_expression
  : multiplicative_expression
  | additive_expression additive_operator  multiplicative_expression {
    $$ = ast_mk_binop($1, $2, $3);
    ast_position($$, @1, @3);
  }
  ;

shift_expression
  : additive_expression
  | shift_expression TK_SHL additive_expression {
    $$ = ast_mk_binop($1, TK_SHL, $3);
    ast_position($$, @1, @3);
  }
  | shift_expression TK_SHR additive_expression {
    $$ = ast_mk_binop($1, TK_SHR, $3);
    ast_position($$, @1, @3);
  }

relational_expression
  : shift_expression
  | relational_expression relational_operator shift_expression {
    $$ = ast_mk_binop($1, $2, $3);
    ast_position($$, @1, @3);
  }
  ;

equality_expression
  : relational_expression
  | equality_expression equality_operator relational_expression {
    $$ = ast_mk_binop($1, $2, $3);
    ast_position($$, @1, @3);
  }
  ;

and_expression
  : equality_expression
  | and_expression '&' equality_expression {
    $$ = ast_mk_binop($1, '&', $3);
    ast_position($$, @1, @3);
  }
  ;

exclusive_or_expression
  : and_expression
  | exclusive_or_expression '^' and_expression {
    $$ = ast_mk_binop($1, '^', $3);
    ast_position($$, @1, @3);
  }
  ;

inclusive_or_expression
  : exclusive_or_expression
  | inclusive_or_expression '|' exclusive_or_expression {
    $$ = ast_mk_binop($1, '|', $3);
    ast_position($$, @1, @3);
  }
  ;

logical_and_expression
  : inclusive_or_expression
  | logical_and_expression TK_ANDAND inclusive_or_expression {
    $$ = ast_mk_binop($1, TK_ANDAND, $3);
    ast_position($$, @1, @3);
  }
  ;

logical_or_expression
  : logical_and_expression
  | logical_or_expression TK_OROR logical_and_expression {
    $$ = ast_mk_binop($1, TK_OROR, $3);
    ast_position($$, @1, @3);
  }
  ;

conditional_expression
  : logical_or_expression
  | logical_or_expression '?' expression ':' conditional_expression {
    printf("TODO logical_or_expression");
    exit(99);
  }
  ;

assignment_expression
  : conditional_expression
  | %prec ASSIGNAMENT unary_expression assignament_operator assignment_expression                     {
    $$ = ast_mk_assignament($1, $2, $3);
    ast_position($$, @1, @3);
  }
  ;

/* NOTE comma expression is removed on purpose! */
expression
  : assignment_expression
  ;

maybe_expression
  : expression
  | %empty { $$ = 0;}
  ;

constant_expression
  : conditional_expression
  ;


maybe_argument_expression_list
  : %empty                          { $$ = ast_mk_list(); }
  | argument_expression_list
  ;

argument_expression_list
  : assignment_expression {
    $$ = ast_mk_list();
    ast_mk_list_push($$, $1);
  }
  | argument_expression_list ',' assignment_expression {
    ast_mk_list_push($1, $3);
    $$ = $1;
  }
  ;

block
  : '{' maybe_stmts '}' {
    $$ = ast_mk_block($2);
  }
  ;

expr_if
  : TK_IF expression block {
    $$ = ast_mk_if($2, $3, 0);
    ast_position($$, @1, @3);
  }
  | TK_IF expression block TK_ELSE block_or_if {
    $$ = ast_mk_if($2, $3, $5);
    ast_position($$, @1, @5);
  }
  ;

expr_while
  : TK_WHILE expression block {
    /* ast_mk_loop(init, pre_cond, update, block, post_cond) */
    $$ = ast_mk_loop(0, $2, 0, $3, 0);
    ast_position($$, @1, @3);
  }
  ;

expr_dowhile
  : TK_DO block TK_WHILE expression ';' {
    /* ast_mk_loop(init, pre_cond, update, block, post_cond) */
    $$ = ast_mk_loop(0, 0, 0, $2, $4);
    ast_position($$, @1, @5);
  }
  ;

expr_for
  : TK_FOR expression ';' expression ';' expression block {
    /* ast_mk_loop(init, pre_cond, update, block, post_cond) */
    $$ = ast_mk_loop($2, $4, $6, $7, 0);
    ast_position($$, @1, @7);
  }
  ;

block_or_if
  : block
  | expr_if
  ;

comment
  : AST_COMMENT {
    $$ = ast_mk_comment(st_newc(yytext, st_enc_utf8));
    ast_position($$, @1, @1);
  }
  ;

//
// terminals
//

literal
  : ident {
    $$ = $1;
  }
  | lit_string
  | lit_numeric
  | TK_NULL {
    $$ = ast_mk_lit_null();
    ast_position($$, @1, @1);
  }
  | TK_TRUE {
    $$ = ast_mk_lit_boolean(true);
    ast_position($$, @1, @1);
  }
  | TK_FALSE {
    $$ = ast_mk_lit_boolean(false);
    ast_position($$, @1, @1);
  }
  ;

lit_string
  : AST_LIT_STR {
    $$ = ast_mk_lit_string(yytext, false);
    ast_position($$, @1, @1);
  }
  | AST_LIT_BYTE {
    /* TODO what is the difference? */
    $$ = ast_mk_lit_string(yytext, false);
    ast_position($$, @1, @1);
  }
  ;

lit_numeric
  : AST_LIT_FLOAT {
    $$ = ast_mk_lit_float(yytext);
    ast_position($$, @1, @1);
  }
  | lit_integer
  ;

lit_integer
  : AST_LIT_INTEGER {
    $$ = ast_mk_lit_integer(yytext);
    ast_position($$, @1, @1);
  }
  ;

/* TODO */
type
  : %prec TYPE ty_primitive {
    $$ = $1;
    ast_position($$, @1, @1);
  }
  | %prec TYPE ty_primitive '(' type ')' {
    //printf("ty_primitive<ty_primitive>\n");
    $1->ty.child = $3;
    ast_position($1, @1, @4);
    $$ = $1;
  }
  ;

ty_primitive
  : %prec TYPE AST_IDENT {
    //printf("ty_primitive\n");
    $$ = ast_mk_type(array_unshift(identifiers), 0);
    ast_position($$, @1, @1);
  }
  | %prec TYPE TK_ANY {
    $$ = ast_mk_type(0, 0); // TODO set type ?
    ast_position($$, @1, @1);
  }
  ;

ident
  : %prec IDENT AST_IDENT {
    //printf("ident\n");
    $$ = ast_mk_lit_id(array_unshift(identifiers), true);
    ast_position($$, @1, @1);
  }
  ;

%%
