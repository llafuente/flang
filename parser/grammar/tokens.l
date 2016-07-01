%{
#include "flang/common.h"
#include "flang/ast.h"
#include "parser/grammar/parser.h"

#define TOKEN(t)    (yylval.token = t)

static int num_hashes;
static int end_hashes;
static int saw_non_hash;
extern array* identifiers;

int yycolumn = 1;
#define UPDATE_COLUMN(n) yycolumn += n;

#define RESET_COLUMN yycolumn = 1;

#define SET_LOCATION yylloc.first_line = yylloc.last_line = yylineno;          \
  yylloc.first_column = yycolumn; yylloc.last_column = yycolumn + yyleng - 1;  \
  UPDATE_COLUMN(yyleng);

#define SET_LEFT yylloc.first_line = yylineno; \
  yylloc.first_column = yycolumn;              \
  UPDATE_COLUMN(yyleng);

#define SET_RIGHT yylloc.last_line = yylineno;   \
  yylloc.last_column = yycolumn + yyleng - 1;    \
  UPDATE_COLUMN(yyleng);

%}
/* ?? */
%option noyywrap

%option stack
%option yylineno

%x str
%x pound
%x shebang_or_attr
%x ltorchar
%x linecomment
%x doc_line
%x blockcomment
%x doc_block
%x suffix

ident [a-zA-Z\x80-\xff_\$][a-zA-Z0-9\x80-\xff_]*

%%

<suffix>{ident}            { BEGIN(INITIAL); }
<suffix>(.|\n)  { yyless(0); BEGIN(INITIAL); }

[ \t\r]             { UPDATE_COLUMN(1);}
\n                  { RESET_COLUMN; }

\xef\xbb\xbf {
  // UTF-8 byte order mark (BOM), ignore if in line 1, error otherwise
  if (yyget_lineno() != 1) {
    return -1;
  }
}

\/\/(\/|\!)           { SET_LEFT; BEGIN(doc_line); yymore(); }
<doc_line>\n          { SET_RIGHT; RESET_COLUMN; BEGIN(INITIAL);
                        yyleng--;
                        yytext[yyleng] = 0;
                        return AST_COMMENT;
                      }
<doc_line>[^\n]*      { yymore(); }

\/\/|\/\/\/\/         { SET_LEFT; BEGIN(linecomment); }
<linecomment>\n       { RESET_COLUMN; BEGIN(INITIAL);}
<linecomment>[^\n]*   { SET_RIGHT; return AST_COMMENT; }

\/\*(\*|\!)[^*]       { SET_LEFT; yy_push_state(INITIAL); yy_push_state(doc_block); yymore(); }
<doc_block>\/\*       { yy_push_state(doc_block); SET_LEFT; yymore(); }
<doc_block>\*\/       {
    yy_pop_state();
    if (yy_top_state() == doc_block) {
        yymore();
    } else {
        SET_RIGHT; return AST_COMMENT;
    }
}
<doc_block>\n         { RESET_COLUMN; yymore(); }
<doc_block>.          { yymore(); }
  /*this is ignored... */
\/\*                  { yy_push_state(blockcomment); }
<blockcomment>\/\*    { yy_push_state(blockcomment); }
<blockcomment>\*\/    { yy_pop_state(); }
<blockcomment>(.|\n)  { }

_        { SET_LOCATION; return TK_UNDERSCORE; }
cast     { SET_LOCATION; return TK_CAST; }
any      { SET_LOCATION; return TK_ANY; }
break    { SET_LOCATION; return TK_BREAK; }
const    { SET_LOCATION; return TK_CONST; }
continue { SET_LOCATION; return TK_CONTINUE; }
else     { SET_LOCATION; return TK_ELSE; }
enum     { SET_LOCATION; return TK_ENUM; }
fn       { SET_LOCATION; return TK_FN; }
function { SET_LOCATION; return TK_FN; }
ffi      { SET_LOCATION; return TK_FFI; }
for      { SET_LOCATION; return TK_FOR; }
if       { SET_LOCATION; return TK_IF; }
in       { SET_LOCATION; return TK_IN; }
match    { SET_LOCATION; return TK_MATCH; }
import   { SET_LOCATION; return TK_IMPORT; }
forward   { SET_LOCATION; return TK_FORWARD; }
pub      { SET_LOCATION; return TK_PUB; }
ref      { SET_LOCATION; return TK_REF; }
return   { SET_LOCATION; return TK_RETURN; }
static   { SET_LOCATION; return TK_STATIC; }
struct   { SET_LOCATION; return TK_STRUCT; }
type     { SET_LOCATION; return TK_TYPE; }
typeof   { SET_LOCATION; return TK_TYPEOF; }
use      { SET_LOCATION; return TK_USE; }
var      { SET_LOCATION; return TK_VAR; }
global   { SET_LOCATION; return TK_GLOBAL; }
where    { SET_LOCATION; return TK_WHERE; }
while    { SET_LOCATION; return TK_WHILE; }
do       { SET_LOCATION; return TK_DO; }
sizeof   { SET_LOCATION; return TK_SIZEOF; }
\$log    { SET_LOCATION; return TK_LOG; }
template { SET_LOCATION; return TK_TEMPLATE; }

false    { SET_LOCATION; return TK_FALSE; }
true     { SET_LOCATION; return TK_TRUE; }
null     { SET_LOCATION; return TK_NULL; }
nil      { SET_LOCATION; return TK_NULL; }

{ident}  {
  /* TODO save location!!! */
  array_append(identifiers, st_newc(yytext, st_enc_utf8));
  SET_LOCATION; return AST_IDENT;
}

0x[0-9a-fA-F_]+             { BEGIN(suffix); SET_LOCATION; return AST_LIT_INTEGER; }
0o[0-8_]+                   { BEGIN(suffix); SET_LOCATION; return AST_LIT_INTEGER; }
0b[01_]+                    { BEGIN(suffix); SET_LOCATION; return AST_LIT_INTEGER; }
[0-9][0-9_]*                { BEGIN(suffix); SET_LOCATION; return AST_LIT_INTEGER; }
[0-9][0-9_]*\.(\.|[a-zA-Z]) { yyless(yyleng - 2); BEGIN(suffix); SET_LOCATION; return AST_LIT_INTEGER; }

[0-9][0-9_]*\.[0-9_]*([eE][-\+]?[0-9_]+)? { BEGIN(suffix); SET_LOCATION; return AST_LIT_FLOAT; }
[0-9][0-9_]*(\.[0-9_]*)?[eE][-\+]?[0-9_]+ { BEGIN(suffix); SET_LOCATION; return AST_LIT_FLOAT; }

;      { SET_LOCATION; return ';'; }
,      { SET_LOCATION; return ','; }
\.\.\. { SET_LOCATION; return TK_DOTDOTDOT; }
\.\.   { SET_LOCATION; return TK_DOTDOT; }
\.     { SET_LOCATION; return '.'; }
\(     { SET_LOCATION; return '('; }
\)     { SET_LOCATION; return ')'; }
\{     { SET_LOCATION; return '{'; }
\}     { SET_LOCATION; return '}'; }
\[     { SET_LOCATION; return '['; }
\]     { SET_LOCATION; return ']'; }
@      { SET_LOCATION; return '@'; }

#      { BEGIN(pound); yymore(); }
<pound>\! { BEGIN(shebang_or_attr); yymore(); }
<shebang_or_attr>\[ {
  BEGIN(INITIAL);
  yyless(2);
  return AST_SHEBANG;
}
<shebang_or_attr>[^\[\n]*\n {
  // Since the \n was eaten as part of the token, yylineno will have
  // been incremented to the value 2 if the shebang was on the first
  // line. This yyless undoes that, setting yylineno back to 1.
  yyless(yyleng - 1);
  if (yyget_lineno() == 1) {
    BEGIN(INITIAL);
    return AST_SHEBANG_LINE;
  } else {
    BEGIN(INITIAL);
    yyless(2);
    return AST_SHEBANG;
  }
}
<pound>. { BEGIN(INITIAL); yyless(1); return '#'; }

\~     { SET_LOCATION; return '~'; }
:      { SET_LOCATION; return ':'; }
\?     { SET_LOCATION; return '?'; }

==    { SET_LOCATION; return TK_EQEQ; }
=>    { SET_LOCATION; return TK_FAT_ARROW; }
=     { SET_LOCATION; return '='; }
\!=   { SET_LOCATION; return TK_NE; }
\!    { SET_LOCATION; return '!'; }
\<=   { SET_LOCATION; return TK_LE; }
\<\<  { SET_LOCATION; return TK_SHL; }
\<\<= { SET_LOCATION; return TK_SHLEQ; }
\<    { SET_LOCATION; return '<'; }
\>=   { SET_LOCATION; return TK_GE; }
\>\>  { SET_LOCATION; return TK_SHR; }
\>\>= { SET_LOCATION; return TK_SHREQ; }
\>    { SET_LOCATION; return '>'; }

\x27                                  { SET_LEFT; BEGIN(ltorchar); yymore(); }
<ltorchar>static                      { SET_RIGHT; BEGIN(INITIAL); return AST_STATIC_LIFETIME; }
<ltorchar>{ident}                     { SET_RIGHT; BEGIN(INITIAL); return AST_LIFETIME; }
<ltorchar>\\[nrt\\\x27\x220]\x27      { SET_RIGHT; BEGIN(suffix); return AST_LIT_CHAR; }
<ltorchar>\\x[0-9a-fA-F]{2}\x27       { SET_RIGHT; BEGIN(suffix); return AST_LIT_CHAR; }
<ltorchar>\\u\{[0-9a-fA-F]?{6}\}\x27  { SET_RIGHT; BEGIN(suffix); return AST_LIT_CHAR; }
<ltorchar>.\x27                       { SET_RIGHT; BEGIN(suffix); return AST_LIT_CHAR; }
<ltorchar>[\x80-\xff]{2,4}\x27        { SET_RIGHT; BEGIN(suffix); return AST_LIT_CHAR; }
<ltorchar><<EOF>>                     { SET_RIGHT; BEGIN(INITIAL); return -1; }

\x22                     { SET_LEFT; BEGIN(str); yymore(); }
<str>\x22                { SET_RIGHT; BEGIN(suffix); return AST_LIT_STR; }

<str><<EOF>>                { return -1; }
<str>\n                     { RESET_COLUMN; yymore(); }
<str>\\[n\nr\rt\\\x27\x220] { yymore(); }
<str>\\x[0-9a-fA-F]{2}      { yymore(); }
<str>\\u\{[0-9a-fA-F]?{6}\} { yymore(); }
<str>\\[^n\nrt\\\x27\x220]  { return -1; }
<str>(.|\n)                 { yymore(); }

-\>  { SET_LOCATION; return TK_RARROW; }
--   { SET_LOCATION; return TK_MINUSMINUS; }
-    { SET_LOCATION; return '-'; }
-=   { SET_LOCATION; return TK_MINUSEQ; }
&&   { SET_LOCATION; return TK_ANDAND; }
&    { SET_LOCATION; return '&'; }
&=   { SET_LOCATION; return TK_ANDEQ; }
\|\| { SET_LOCATION; return TK_OROR; }
\|   { SET_LOCATION; return '|'; }
\|=  { SET_LOCATION; return TK_OREQ; }
\+\+ { SET_LOCATION; return TK_PLUSPLUS; }
\+   { SET_LOCATION; return '+'; }
\+=  { SET_LOCATION; return TK_PLUSEQ; }
\*   { SET_LOCATION; return '*'; }
\*=  { SET_LOCATION; return TK_STAREQ; }
\/   { SET_LOCATION; return '/'; }
\/=  { SET_LOCATION; return TK_SLASHEQ; }
\^   { SET_LOCATION; return '^'; }
\^=  { SET_LOCATION; return TK_CARETEQ; }
%    { SET_LOCATION; return '%'; }
%=   { SET_LOCATION; return TK_PERCENTEQ; }

<<EOF>> { SET_LOCATION; return 0; }

%%
