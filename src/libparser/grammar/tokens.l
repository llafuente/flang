%{
#include "flang/flang.h"
#include "flang/libast.h"

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
                        return COMMENT;
                      }
<doc_line>[^\n]*      { yymore(); }

\/\/|\/\/\/\/         { SET_LEFT; BEGIN(linecomment); }
<linecomment>\n       { RESET_COLUMN; BEGIN(INITIAL);}
<linecomment>[^\n]*   { SET_RIGHT; return COMMENT; }

\/\*(\*|\!)[^*]       { SET_LEFT; yy_push_state(INITIAL); yy_push_state(doc_block); yymore(); }
<doc_block>\/\*       { yy_push_state(doc_block); SET_LEFT; yymore(); }
<doc_block>\*\/       {
    yy_pop_state();
    if (yy_top_state() == doc_block) {
        yymore();
    } else {
        SET_RIGHT; return COMMENT;
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
unsafe_cast     { SET_LOCATION; return TK_UNSAFE_CAST; }
any      { SET_LOCATION; return TK_ANY; }
break    { SET_LOCATION; return TK_BREAK; }
const    { SET_LOCATION; return TK_CONST; }
continue { SET_LOCATION; return TK_CONTINUE; }
else     { SET_LOCATION; return TK_ELSE; }
enum     { SET_LOCATION; return TK_ENUM; }
fn       { SET_LOCATION; return TK_FN; }
function { SET_LOCATION; return TK_FN; }
operator { SET_LOCATION; return TK_OPERATOR; }
property { SET_LOCATION; return TK_PROPERTY; }
ffi      { SET_LOCATION; return TK_FFI; }
for      { SET_LOCATION; return TK_FOR; }
if       { SET_LOCATION; return TK_IF; }
in       { SET_LOCATION; return TK_IN; }
match    { SET_LOCATION; return TK_MATCH; }
import   { SET_LOCATION; return TK_IMPORT; }
forward  { SET_LOCATION; return TK_FORWARD; }
pub      { SET_LOCATION; return TK_PUB; }
ref      { SET_LOCATION; return TK_REF; }
return   { SET_LOCATION; return TK_RETURN; }
static   { SET_LOCATION; return TK_STATIC; }
struct   { SET_LOCATION; return TK_STRUCT; }
alias    { SET_LOCATION; return TK_ALIAS; }
 /* type     { SET_LOCATION; return TK_TYPE; } */
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
new { SET_LOCATION; return TK_NEW; }
delete { SET_LOCATION; return TK_DELETE; }

false    { SET_LOCATION; return TK_FALSE; }
true     { SET_LOCATION; return TK_TRUE; }
null     { SET_LOCATION; return TK_NULL; }
nil      { SET_LOCATION; return TK_NULL; }

implicit  { SET_LOCATION; return TK_IMPLICIT; }
implement { SET_LOCATION; return TK_IMPLEMENT; }
as        { SET_LOCATION; return TK_AS; }



{ident}  {
  /* TODO save location!!! */
  array_push(identifiers, st_newc(yytext, st_enc_utf8));
  SET_LOCATION; return IDENTIFIER;
}

0x[0-9a-fA-F_]+             { BEGIN(suffix); SET_LOCATION; return LIT_INTEGER; }
0o[0-8_]+                   { BEGIN(suffix); SET_LOCATION; return LIT_INTEGER; }
0b[01_]+                    { BEGIN(suffix); SET_LOCATION; return LIT_INTEGER; }
[0-9][0-9_]*                { BEGIN(suffix); SET_LOCATION; return LIT_INTEGER; }
[0-9][0-9_]*\.(\.|[a-zA-Z]) { yyless(yyleng - 2); BEGIN(suffix); SET_LOCATION; return LIT_INTEGER; }

[0-9][0-9_]*\.[0-9_]*([eE][-\+]?[0-9_]+)? { BEGIN(suffix); SET_LOCATION; return LIT_FLOAT; }
[0-9][0-9_]*(\.[0-9_]*)?[eE][-\+]?[0-9_]+ { BEGIN(suffix); SET_LOCATION; return LIT_FLOAT; }

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
\[\]=  { SET_LOCATION; return TK_ACCESS_MOD; }
\[\]   { SET_LOCATION; return TK_ACCESS; }
@      { SET_LOCATION; return '@'; }
#      { SET_LOCATION; return '#'; } // TODO: handle shebang

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

\x22                     { SET_LEFT; BEGIN(str); yymore(); }
<str>\x22                { SET_RIGHT; BEGIN(suffix); return LIT_STRING; }


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
