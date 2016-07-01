/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_YY_PARSER_H_INCLUDED
# define YY_YY_PARSER_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    TK_EQEQ = 258,
    TK_NE = 259,
    TK_LE = 260,
    TK_SHL = 261,
    TK_SHLEQ = 262,
    TK_GE = 263,
    TK_SHR = 264,
    TK_SHREQ = 265,
    TK_RARROW = 266,
    TK_MINUSEQ = 267,
    TK_ANDAND = 268,
    TK_ANDEQ = 269,
    TK_OROR = 270,
    TK_OREQ = 271,
    TK_PLUSEQ = 272,
    TK_STAREQ = 273,
    TK_SLASHEQ = 274,
    TK_CARETEQ = 275,
    TK_PERCENTEQ = 276,
    TK_DOTDOTDOT = 277,
    TK_DOTDOT = 278,
    TK_PLUSPLUS = 279,
    TK_MINUSMINUS = 280,
    TK_CAST = 281,
    TK_TEMPLATE = 282,
    TK_UNDERSCORE = 283,
    TK_AS = 284,
    TK_ANY = 285,
    TK_BREAK = 286,
    TK_CONST = 287,
    TK_CONTINUE = 288,
    TK_ELSE = 289,
    TK_ENUM = 290,
    TK_FN = 291,
    TK_FFI = 292,
    TK_FOR = 293,
    TK_IF = 294,
    TK_IN = 295,
    TK_MATCH = 296,
    TK_IMPORT = 297,
    TK_FORWARD = 298,
    TK_PUB = 299,
    TK_REF = 300,
    TK_RETURN = 301,
    TK_STATIC = 302,
    TK_STRUCT = 303,
    TK_TYPE = 304,
    TK_TYPEOF = 305,
    TK_USE = 306,
    TK_VAR = 307,
    TK_GLOBAL = 308,
    TK_WHERE = 309,
    TK_WHILE = 310,
    TK_DO = 311,
    TK_SIZEOF = 312,
    TK_LOG = 313,
    TK_FALSE = 314,
    TK_TRUE = 315,
    TK_NULL = 316,
    TK_FAT_ARROW = 317,
    AST_LIT_ID = 318,
    AST_LIT_INTEGER = 319,
    AST_LIT_FLOAT = 320,
    AST_LIT_STR = 321,
    AST_LIT_BYTE = 322,
    AST_IDENT = 323,
    AST_SHEBANG = 324,
    AST_SHEBANG_LINE = 325,
    AST_COMMENT = 326,
    AST_LIT_BYTE_STR = 327,
    AST_LIT_CHAR = 328,
    AST_LIT_BYTE_STR_RAW = 329,
    AST_STATIC_LIFETIME = 330,
    AST_LIFETIME = 331,
    IDENT = 332,
    TYPE = 333,
    FORTYPE = 334,
    FOR = 335,
    DOTDOT = 336,
    RETURN = 337,
    ASSIGNAMENT = 338,
    EQUALITY = 339,
    COMPARISON = 340,
    ADDITION = 341,
    MULTIPLICATION = 342
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 28 "parser.y" /* yacc.c:1909  */

  ast_t* node;
  string* string;
  int token;
  bool test;

#line 149 "parser.h" /* yacc.c:1909  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE YYLTYPE;
struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif


extern YYSTYPE yylval;
extern YYLTYPE yylloc;
int yyparse (ast_t **root);

#endif /* !YY_YY_PARSER_H_INCLUDED  */
