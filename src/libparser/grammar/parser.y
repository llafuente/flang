%{
  #define YYDEBUG 1

  #include "flang/flang.h"
  #include "flang/libast.h"
  #include "flang/libparser.h"
  #include "flang/debug.h"

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
%token <token> TK_ACCESS TK_PUSH


/* keywords */
%token <token> TK_UNDERSCORE "_"
%token <token> TK_ANY "any"

%token <token> TK_TEMPLATE "template"
%token <token> TK_IMPLEMENT "implement"


%token <token> TK_IF "if"
%token <token> TK_ELSE "else"

%token <token> TK_AS  "as"
%token <token> TK_CAST  "cast"
%token <token> TK_UNSAFE_CAST  "unsafe_cast"

%token <token> TK_FN  "fn' or 'function"
%token <token> TK_OPERATOR  "operator"
%token <token> TK_PROPERTY  "property"
%token <token> TK_FFI  "ffi"
%token <token> TK_RETURN  "return"

%token <token> TK_IMPORT  "import"
%token <token> TK_FORWARD  "forward"


%token <token> TK_REF  "ref"

%token <token> TK_STRUCT  "struct"
%token <token> TK_ALIAS  "alias"
%token <token> TK_TYPE  "type"
%token <token> TK_TYPEOF  "typeof"
%token <token> TK_USE  "use"

%token <token> TK_FALSE  "false"
%token <token> TK_TRUE  "true"
%token <token> TK_NULL  "null"

%token <token>TK_VAR  "var"
%token <token>TK_GLOBAL  "global"

%token <token> TK_WHILE  "while"
%token <token> TK_DO  "do"
%token <token> TK_BREAK  "break"
%token <token> TK_CONTINUE  "continue"
%token <token> TK_FOR  "for"

%token <token> TK_SIZEOF  "sizeof"
/* TODO rename to DOLLAR_LOG ? */
%token <token> TK_LOG  "log"

/* reserved but maybe not be used ever ^^ */
%token <token> TK_FAT_ARROW TK_WHERE TK_PUB TK_STATIC TK_CONST
%token <token> TK_ENUM TK_IN TK_MATCH

/* ast */
/* ast.literals */
%token <node> LIT_INTEGER "integer literal"
%token <node> LIT_FLOAT "float literal"
%token <node> LIT_STRING "string literal"
%token <node> LIT_BYTE "byte literal"
%token <node> IDENTIFIER "identifier"
%token <node> COMMENT "comment"

/* Define the type of node our nonterminal symbols represent.
   The types refer to the %union declaration above. Ex: when
   we call an ident (defined by union type ident) we are really
   calling an (NIdentifier*). It makes the compiler happy.
 */
%type <node> program stmts
%type <node> stmt var_decl maybe_argument_expression_list argument_expression_list comment
%type <node> maybe_type_list type_list ident_list
%type <node> struct_decl struct_decl_fields struct_decl_fields_list struct_decl_field
%type <node> block maybe_stmts log_expression

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
%type <node> attributes attribute
%type <node> fn_decl_with_return_type fn_decl_without_return_type fn_decl fn_parameters fn_parameter_list fn_parameter

%type <token> assignament_operator equality_operator relational_operator additive_operator multiplicative_operator unary_operator function_operators

/* terminals */
%type <node> type ty_primitive ident literal lit_integer lit_numeric lit_string

/* %expect 0 */

%precedence IDENTIFIER
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

%token END 0 "end of file"

%start program

%%

/* todo module definition */
program
  : stmts {
    //printf("stmts addr: %p\n", $1);
    ast_position($1, @1, @1);
    ast_t* block = ast_mk_block($1);
    block->block.scope = AST_SCOPE_GLOBAL;
    block->block.uids = pool_new(sizeof(hash_t));
    hash_new(block->block.uids, 100);
    ast_position(block, @1, @1);
    (*root)->program.body = block;
    ast_position(*root, @1, @1);
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
  | log_expression ';'
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
  // declare a template name
  | TK_TEMPLATE ident ';' {
    // TODO ensure ident start with '$' ??
    $$ = ast_mk_template($2, 0);
    ast_position($$, @1, @3);
  }
  // implement a type or a function
  | TK_IMPLEMENT ident '(' type_list ')' TK_AS ident ';' {
    $$ = ast_mk_implement($2, $4, $7);
    ast_position($$, @1, @8);
  }
  ;

import_stmt
  // import a module keeping with a new scope
  // only functions will be available
  : TK_IMPORT lit_string {
    $$ = ast_mk_import($2, false);
    ast_position($$, @1, @2);
  }
  // import a module and include all in the current scope
  // everything at first level will be available
  | TK_FORWARD TK_IMPORT lit_string {
    $$ = ast_mk_import($3, true);
    ast_position($$, @1, @3);
  }
  ;

var_decl
  // full local variable declaration and initialization
  : TK_VAR type ident '=' expression {
    $$ = ast_mk_list();
    ast_t* decl = ast_mk_var_decl($2, $3, AST_SCOPE_BLOCK);
    ast_position(decl, @1, @3);
    ast_mk_list_push($$, decl);
    ast_t* assignament = ast_mk_assignament(ast_clone(decl->var.id), '=', $5);
    ast_position(assignament, @3, @5);
    ast_mk_list_push($$, assignament);
    ast_position($$, @1, @5);
  }
  // inference local variable declaration and initialization
  | TK_VAR ident '=' expression {
    $$ = ast_mk_list();
    ast_t* decl = ast_mk_var_decl(0, $2, AST_SCOPE_BLOCK);
    ast_position(decl, @1, @2);
    ast_mk_list_push($$, decl);
    ast_t* assignament = ast_mk_assignament(ast_clone(decl->var.id), '=', $4);
    ast_position(assignament, @3, @4);
    ast_mk_list_push($$, assignament);
    ast_position($$, @1, @4);
  }
  // full local variable declaration
  | TK_VAR type ident {
    $$ = ast_mk_var_decl($2, $3, AST_SCOPE_BLOCK);
    ast_position($$, @1, @3);
  }
  // inference local variable declaration
  | TK_VAR ident {
    $$ = ast_mk_var_decl(0, $2, AST_SCOPE_BLOCK);
    ast_position($$, @1, @2);
  }
  // full global variable declaration
  | TK_GLOBAL type ident '=' expression {
    $$ = ast_mk_list();
    ast_t* decl = ast_mk_var_decl($2, $3, AST_SCOPE_GLOBAL);
    ast_position(decl, @1, @3);
    ast_mk_list_push($$, decl);
    ast_t* assignament = ast_mk_assignament(ast_clone(decl->var.id), '=', $5);
    ast_position(assignament, @3, @5);
    ast_mk_list_push($$, assignament);
    ast_position($$, @1, @5);
  }
  // coder hint for where the variable comes.
  | TK_GLOBAL ident {
    $$ = ast_mk_var_decl(0, $2, AST_SCOPE_GLOBAL);
    ast_position($$, @1, @2);
  }
  ;

struct_decl
  : TK_STRUCT ident struct_decl_fields {
    $$ = ast_mk_struct_decl($2, 0, $3);
    ast_position($$, @1, @3);
  }
  | TK_STRUCT ident '(' type_list ')' struct_decl_fields {
    $$ = ast_mk_struct_decl($2, $4, $6);
    ast_position($$, @1, @6);
  }
  // TODO struct extend
  ;

struct_decl_fields
  : '{' struct_decl_fields_list '}'      { $$ = $2; ast_position($$, @1, @3); }
  // allow last comma
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
  // REVIEW study if we want it back
  // | ident      { $$ = ast_mk_struct_decl_field($1, 0);  ast_position($$, @1, @1); }
  | TK_ALIAS ident ident {
    $$ = ast_mk_struct_decl_alias($2, $3);
    ast_position($$, @1, @3);
  }
  ;

attributes
  : attribute {
    $$ = ast_mk_list();
    ast_mk_list_push($$, $1);
  }
  | attributes attribute {
    ast_mk_list_push($1, $2);
    $$ = $1;
  }
  | %empty { $$ = 0; }
  ;

attribute
  : '#' ident '=' literal {
    $$ = ast_mk_attribute($2, $4);
    ast_position($$, @1, @4);

    // TODO do not allow attributes to repeat
    // TODO id must have a lit_id literal
  }
  | '#' ident {
    $$ = ast_mk_attribute($2, 0);
    ast_position($$, @1, @2);
  }
  | TK_FFI {
    ast_t* id = ast_mk_lit_id(st_newc("ffi", st_enc_utf8), false);
    $$ = ast_mk_attribute(id, 0);
    ast_position(id, @1, @1);
    ast_position($$, @1, @1);
  }
  ;

fn_decl_without_return_type
  : TK_FN ident fn_parameters {
    $$ = ast_mk_fn_decl($2, $3, 0, 0, 0, 0, AST_FUNC_FUNCTION);
    // XXX Hack!
    if ($3->parent == (ast_t*)1) {
      $$->func.varargs = true;
    }

    ast_position($$, @1, @3);
  }
  | TK_FN TK_OPERATOR function_operators fn_parameters {
    // can't be varargs
    if ($4->parent == (ast_t*)1) {
      yyerror(root, "syntax error, operator overloading is incompatible with varargs"); YYERROR;
    }

    // TODO when add more params this may not be true anymore
    if ($4->list.length != 2) {
      yyerror(root, "syntax error, operator overloading require 2 params"); YYERROR;
    }

    $$ = ast_mk_fn_decl(0, $4, 0, 0, 0, $3, AST_FUNC_OPERATOR);

    ast_position($$, @1, @4);
  }
  | TK_FN TK_PROPERTY ident fn_parameters {
    $$ = ast_mk_fn_decl($3, $4, 0, 0, 0, 0, AST_FUNC_PROPERTY);

    if ($4->list.length != 1) {
      yyerror(root, "syntax error, struct property overloading must have an unique parameter"); YYERROR;
    }

    if ($4->parent == (ast_t*)1) {
      yyerror(root, "syntax error, struct property overloading is incompatible with varargs"); YYERROR;
    }

    ast_position($$, @1, @3);
  }
  ;

fn_decl_with_return_type
  : fn_decl_without_return_type ':' type {
    $$ = $1;
    $$->func.ret_type = $3;
    ast_position($$, @1, @3);
  }
  ;

fn_decl
  // readable error
  : TK_FFI fn_decl_with_return_type block {
    yyerror(root, "syntax error, ffi cannot have a body and must have declared return type"); YYERROR;
  }
  // this is the good one
  | TK_FFI fn_decl_with_return_type {
    $$ = $2;
    $$->func.ffi = true;

    ast_position($$, @1, @2);
  }
  // readable error
  | TK_FFI fn_decl_without_return_type {
    yyerror(root, "syntax error, ffi function require a return type"); YYERROR;
  }
  | fn_decl_with_return_type block {
    $$ = $1;
    ast_mk_fn_decl_body($$, $2);

    // cannot be a ffi funcion with a body!
    if ($1->func.ffi) {
      yyerror(root, "syntax error, ffi cannot have a body"); YYERROR;
    }

    ast_position($$, @1, @2);
  }
  | fn_decl_without_return_type block {
    $$ = $1;
    ast_mk_fn_decl_body($$, $2);

    ast_position($$, @1, @2);
  }
  ;

fn_parameters
  : %empty                          { $$ = ast_mk_list(); ast_position($$, @0, @0); }
  | '(' ')'                         { $$ = ast_mk_list(); ast_position($$, @1, @2); }
  | '(' fn_parameter_list ')'                  { $$ = $2; }
  | '(' fn_parameter_list ',' ')'              { $$ = $2; }
  | '(' fn_parameter_list ',' TK_DOTDOTDOT ')' {
    $$ = $2;
    // TODO find a more elegant way?
    // use parent as hack
    $$->parent = (ast_t*)1;
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

function_operators
  : additive_operator
  | multiplicative_operator
  | TK_ACCESS    { $$ = TK_ACCESS; }
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
    $$ = ast_mk_member($1, $3, true, true);
    ast_position($$, @1, @4);
  }
  | postfix_expression '[' lit_integer ']' {
    /* TODO maybe_expr? */
    $$ = ast_mk_member($1, $3, false, true);
    ast_position($$, @1, @4);
  }
  | postfix_expression '(' maybe_argument_expression_list ')' {
    // TODO this cannot be assignament lhs, right ?
    $$ = ast_mk_call_expr($1, $3);
    ast_position($$, @1, @4);
  }
  | postfix_expression '.' ident {
    $$ = ast_mk_member($1, $3, false, false);
    ast_position($$, @1, @3);
  }
  | postfix_expression '.' lit_integer {
    $$ = ast_mk_member($1, $3, false, false);
    ast_position($$, @1, @3);
  }
  | postfix_expression TK_PLUSPLUS   { $$ = ast_mk_runary($1, TK_PLUSPLUS); ast_position($$, @1, @2); }
  | postfix_expression TK_MINUSMINUS { $$ = ast_mk_runary($1, TK_MINUSMINUS); ast_position($$, @1, @2); }
  /* TODO really? not needed!
  | postfix_expression PTR_OP IDENTIFIER '->'
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
    $$ = ast_mk_cast($3, $5, false);
    ast_position($$, @1, @5);
  }
  | TK_UNSAFE_CAST '(' type ')' cast_expression {
    $$ = ast_mk_cast($3, $5, true);
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

maybe_type_list
  : %empty                          { $$ = ast_mk_list(); }
  | type_list
  ;

type_list
  : type {
    $$ = ast_mk_list();
    ast_mk_list_push($$, $1);
  }
  | type_list ',' type {
    ast_mk_list_push($1, $3);
    $$ = $1;
  }
  ;

ident_list
  : ident {
    $$ = ast_mk_list();
    ast_mk_list_push($$, $1);
  }
  | ident_list ',' ident {
    ast_mk_list_push($1, $3);
    $$ = $1;
  }
  ;

block
  : '{' maybe_stmts '}' {
    $$ = ast_mk_block($2);
  }
  ;

log_expression
  : TK_LOG argument_expression_list {
    $$ = ast_mk_log($2);
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
    $$ = ast_mk_loop(AST_STMT_WHILE, 0, $2, 0, $3, 0);
    ast_position($$, @1, @3);
  }
  ;

expr_dowhile
  : TK_DO block TK_WHILE expression ';' {
    /* ast_mk_loop(init, pre_cond, update, block, post_cond) */
    $$ = ast_mk_loop(AST_STMT_DOWHILE, 0, 0, 0, $2, $4);
    ast_position($$, @1, @5);
  }
  ;

expr_for
  : TK_FOR expression ';' expression ';' expression block {
    /* ast_mk_loop(init, pre_cond, update, block, post_cond) */
    $$ = ast_mk_loop(AST_STMT_FOR, $2, $4, $6, $7, 0);
    ast_position($$, @1, @7);
  }
  ;

block_or_if
  : block
  | expr_if
  ;

comment
  : COMMENT {
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
  : LIT_STRING {
    $$ = ast_mk_lit_string(yytext, false);
    ast_position($$, @1, @1);
  }
  | LIT_BYTE {
    /* TODO what is the difference? */
    $$ = ast_mk_lit_string(yytext, false);
    ast_position($$, @1, @1);
  }
  ;

lit_numeric
  : LIT_FLOAT {
    $$ = ast_mk_lit_float(yytext);
    ast_position($$, @1, @1);
  }
  | lit_integer
  ;

lit_integer
  : LIT_INTEGER {
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
  | %prec TYPE ty_primitive '*' {
    ast_t* list = ast_mk_list();
    ast_mk_list_push(list, $1);

    $$ = ast_mk_type(st_newc("ref", st_enc_utf8), list);
    ast_position($$, @1, @2);
  }
  | %prec TYPE ty_primitive '(' type_list ')' {
    $1->ty.children = $3;
    ast_position($1, @1, @4);
    $$ = $1;
  }
  | %prec TYPE TK_REF '(' type_list ')' {
    $$ = ast_mk_type(st_newc("ref", st_enc_utf8), $3);
    ast_position($$, @1, @4);
  }
  ;

ty_primitive
  : %prec TYPE IDENTIFIER {
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
  : %prec IDENTIFIER IDENTIFIER {
    //printf("ident\n");
    $$ = ast_mk_lit_id(array_unshift(identifiers), true);
    ast_position($$, @1, @1);
  }
  ;

%%
