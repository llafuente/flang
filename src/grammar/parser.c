/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison implementation for Yacc-like parsers in C

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.0.4"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Copy the first part of user declarations.  */
#line 1 "parser.y" /* yacc.c:339  */

#define YYDEBUG 1

#include "flang.h"

extern array* identifiers;
extern int yylex();
extern int yyget_lineno();
extern char* yytext;
void yyerror(ast_t** root, const char* s) {
  ast_t* err = ast_mk_error(s, "syntax");
  ast_position(err, yylloc, yylloc);
  (*root)->program.body = err;

  printf("%s at [%d:%d]\n%s\n", s, yylloc.first_line, yylloc.first_column,
         yytext);
}

#line 84 "parser.c" /* yacc.c:339  */

#ifndef YY_NULLPTR
#if defined __cplusplus && 201103L <= __cplusplus
#define YY_NULLPTR nullptr
#else
#define YY_NULLPTR 0
#endif
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
#undef YYERROR_VERBOSE
#define YYERROR_VERBOSE 1
#else
#define YYERROR_VERBOSE 1
#endif

/* In a future release of Bison, this section will be replaced
   by #include "parser.h".  */
#ifndef YY_YY_PARSER_H_INCLUDED
#define YY_YY_PARSER_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
#define YYDEBUG 1
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
#define YYTOKENTYPE
enum yytokentype {
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
#if !defined YYSTYPE && !defined YYSTYPE_IS_DECLARED

union YYSTYPE {
#line 28 "parser.y" /* yacc.c:355  */

  ast_t* node;
  string* string;
  int token;
  bool test;

#line 219 "parser.c" /* yacc.c:355  */
};

typedef union YYSTYPE YYSTYPE;
#define YYSTYPE_IS_TRIVIAL 1
#define YYSTYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if !defined YYLTYPE && !defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE YYLTYPE;
struct YYLTYPE {
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
#define YYLTYPE_IS_DECLARED 1
#define YYLTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;
extern YYLTYPE yylloc;
int yyparse(ast_t** root);

#endif /* !YY_YY_PARSER_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 250 "parser.c" /* yacc.c:358  */

#ifdef short
#undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
#ifdef __SIZE_TYPE__
#define YYSIZE_T __SIZE_TYPE__
#elif defined size_t
#define YYSIZE_T size_t
#elif !defined YYSIZE_T
#include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#define YYSIZE_T size_t
#else
#define YYSIZE_T unsigned int
#endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T)-1)

#ifndef YY_
#if defined YYENABLE_NLS && YYENABLE_NLS
#if ENABLE_NLS
#include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#define YY_(Msgid) dgettext("bison-runtime", Msgid)
#endif
#endif
#ifndef YY_
#define YY_(Msgid) Msgid
#endif
#endif

#ifndef YY_ATTRIBUTE
#if (defined __GNUC__ &&                                                       \
     (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__))) ||             \
    defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#define YY_ATTRIBUTE(Spec) __attribute__(Spec)
#else
#define YY_ATTRIBUTE(Spec) /* empty */
#endif
#endif

#ifndef YY_ATTRIBUTE_PURE
#define YY_ATTRIBUTE_PURE YY_ATTRIBUTE((__pure__))
#endif

#ifndef YY_ATTRIBUTE_UNUSED
#define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE((__unused__))
#endif

#if !defined _Noreturn &&                                                      \
    (!defined __STDC_VERSION__ || __STDC_VERSION__ < 201112)
#if defined _MSC_VER && 1200 <= _MSC_VER
#define _Noreturn __declspec(noreturn)
#else
#define _Noreturn YY_ATTRIBUTE((__noreturn__))
#endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if !defined lint || defined __GNUC__
#define YYUSE(E) ((void)(E))
#else
#define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                                    \
  _Pragma("GCC diagnostic push")                                               \
      _Pragma("GCC diagnostic ignored \"-Wuninitialized\"")                    \
      _Pragma("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
#define YY_IGNORE_MAYBE_UNINITIALIZED_END _Pragma("GCC diagnostic pop")
#else
#define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
#define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
#define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
#define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if !defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

#ifdef YYSTACK_USE_ALLOCA
#if YYSTACK_USE_ALLOCA
#ifdef __GNUC__
#define YYSTACK_ALLOC __builtin_alloca
#elif defined __BUILTIN_VA_ARG_INCR
#include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#elif defined _AIX
#define YYSTACK_ALLOC __alloca
#elif defined _MSC_VER
#include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#define alloca _alloca
#else
#define YYSTACK_ALLOC alloca
#if !defined _ALLOCA_H && !defined EXIT_SUCCESS
#include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
/* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#endif
#endif
#endif
#endif
#endif

#ifdef YYSTACK_ALLOC
/* Pacify GCC's 'empty if-body' warning.  */
#define YYSTACK_FREE(Ptr)                                                      \
  do {/* empty */                                                              \
    ;                                                                          \
  } while (0)
#ifndef YYSTACK_ALLOC_MAXIMUM
/* The OS might guarantee only one guard page at the bottom of the stack,
   and a page size can be as small as 4096 bytes.  So we cannot safely
   invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
   to allow for a few compiler-allocated temporary stack slots.  */
#define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#endif
#else
#define YYSTACK_ALLOC YYMALLOC
#define YYSTACK_FREE YYFREE
#ifndef YYSTACK_ALLOC_MAXIMUM
#define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#endif
#if (defined __cplusplus && !defined EXIT_SUCCESS &&                           \
     !((defined YYMALLOC || defined malloc) &&                                 \
       (defined YYFREE || defined free)))
#include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#endif
#endif
#ifndef YYMALLOC
#define YYMALLOC malloc
#if !defined malloc && !defined EXIT_SUCCESS
void* malloc(YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#endif
#endif
#ifndef YYFREE
#define YYFREE free
#if !defined free && !defined EXIT_SUCCESS
void free(void*);       /* INFRINGES ON USER NAME SPACE */
#endif
#endif
#endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */

#if (!defined yyoverflow &&                                                    \
     (!defined __cplusplus ||                                                  \
      (defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL &&                     \
       defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc {
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
  YYLTYPE yyls_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
#define YYSTACK_GAP_MAXIMUM (sizeof(union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
#define YYSTACK_BYTES(N)                                                       \
  ((N) * (sizeof(yytype_int16) + sizeof(YYSTYPE) + sizeof(YYLTYPE)) +          \
   2 * YYSTACK_GAP_MAXIMUM)

#define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
#define YYSTACK_RELOCATE(Stack_alloc, Stack)                                   \
  do {                                                                         \
    YYSIZE_T yynewbytes;                                                       \
    YYCOPY(&yyptr->Stack_alloc, Stack, yysize);                                \
    Stack = &yyptr->Stack_alloc;                                               \
    yynewbytes = yystacksize * sizeof(*Stack) + YYSTACK_GAP_MAXIMUM;           \
    yyptr += yynewbytes / sizeof(*yyptr);                                      \
  } while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
#ifndef YYCOPY
#if defined __GNUC__ && 1 < __GNUC__
#define YYCOPY(Dst, Src, Count)                                                \
  __builtin_memcpy(Dst, Src, (Count) * sizeof(*(Src)))
#else
#define YYCOPY(Dst, Src, Count)                                                \
  do {                                                                         \
    YYSIZE_T yyi;                                                              \
    for (yyi = 0; yyi < (Count); yyi++)                                        \
      (Dst)[yyi] = (Src)[yyi];                                                 \
  } while (0)
#endif
#endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL 116
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST 534

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS 113
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS 61
/* YYNRULES -- Number of rules.  */
#define YYNRULES 166
/* YYNSTATES -- Number of states.  */
#define YYNSTATES 264

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK 2
#define YYMAXUTOK 342

#define YYTRANSLATE(YYX)                                                       \
  ((unsigned int)(YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] = {
    0,  2,   2,  2,  2,   2,  2,   2,   2,   2,  2,   2,  2,   2,   2,  2,  2,
    2,  2,   2,  2,  2,   2,  2,   2,   2,   2,  2,   2,  2,   2,   2,  2,  99,
    2,  107, 2,  98, 91,  2,  102, 109, 96,  92, 106, 93, 103, 97,  2,  2,  2,
    2,  2,   2,  2,  2,   2,  2,   108, 104, 86, 83,  87, 112, 110, 2,  2,  2,
    2,  2,   2,  2,  2,   2,  2,   2,   2,   2,  2,   2,  2,   2,   2,  2,  2,
    2,  2,   2,  2,  2,   2,  101, 2,   111, 90, 2,   2,  2,   2,   2,  2,  2,
    2,  2,   2,  2,  2,   2,  2,   2,   2,   2,  2,   2,  2,   2,   2,  2,  2,
    2,  2,   2,  2,  100, 89, 105, 2,   2,   2,  2,   2,  2,   2,   2,  2,  2,
    2,  2,   2,  2,  2,   2,  2,   2,   2,   2,  2,   2,  2,   2,   2,  2,  2,
    2,  2,   2,  2,  2,   2,  2,   2,   2,   2,  2,   2,  2,   2,   2,  2,  2,
    2,  2,   2,  2,  2,   2,  2,   2,   2,   2,  2,   2,  2,   2,   2,  2,  2,
    2,  2,   2,  2,  2,   2,  2,   2,   2,   2,  2,   2,  2,   2,   2,  2,  2,
    2,  2,   2,  2,  2,   2,  2,   2,   2,   2,  2,   2,  2,   2,   2,  2,  2,
    2,  2,   2,  2,  2,   2,  2,   2,   2,   2,  2,   2,  2,   2,   2,  2,  2,
    2,  2,   2,  2,  2,   2,  2,   2,   2,   2,  2,   2,  2,   2,   2,  2,  2,
    2,  1,   2,  3,  4,   5,  6,   7,   8,   9,  10,  11, 12,  13,  14, 15, 16,
    17, 18,  19, 20, 21,  22, 23,  24,  25,  26, 27,  28, 29,  30,  31, 32, 33,
    34, 35,  36, 37, 38,  39, 40,  41,  42,  43, 44,  45, 46,  47,  48, 49, 50,
    51, 52,  53, 54, 55,  56, 57,  58,  59,  60, 61,  62, 63,  64,  65, 66, 67,
    68, 69,  70, 71, 72,  73, 74,  75,  76,  77, 78,  79, 80,  81,  82, 84, 85,
    88, 94,  95};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] = {
    0,   162, 162, 176, 183, 195, 200, 204, 207, 208, 209, 210, 213, 214,
    215, 216, 217, 218, 219, 220, 221, 225, 229, 233, 237, 241, 245, 253,
    257, 264, 274, 284, 288, 292, 302, 309, 316, 317, 321, 326, 331, 335,
    336, 340, 344, 351, 352, 357, 364, 368, 377, 395, 403, 414, 417, 430,
    431, 432, 433, 434, 443, 448, 457, 461, 468, 469, 470, 471, 472, 473,
    474, 475, 476, 477, 478, 482, 483, 487, 488, 489, 490, 493, 494, 498,
    499, 500, 504, 505, 506, 507, 511, 515, 519, 527, 528, 533, 538, 543,
    547, 551, 552, 559, 560, 561, 562, 563, 574, 581, 582, 589, 590, 597,
    598, 605, 606, 610, 616, 617, 624, 625, 632, 633, 640, 641, 648, 649,
    656, 657, 664, 665, 672, 673, 680, 681, 689, 693, 694, 703, 704, 708,
    712, 719, 725, 731, 735, 742, 750, 758, 766, 767, 771, 782, 785, 786,
    787, 791, 795, 802, 806, 814, 818, 822, 830, 834, 843, 848, 855};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 1
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char* const yytname[] = {
    "$end",                     "error",
    "$undefined",               "TK_EQEQ",
    "TK_NE",                    "TK_LE",
    "TK_SHL",                   "TK_SHLEQ",
    "TK_GE",                    "TK_SHR",
    "TK_SHREQ",                 "TK_RARROW",
    "TK_MINUSEQ",               "TK_ANDAND",
    "TK_ANDEQ",                 "TK_OROR",
    "TK_OREQ",                  "TK_PLUSEQ",
    "TK_STAREQ",                "TK_SLASHEQ",
    "TK_CARETEQ",               "TK_PERCENTEQ",
    "TK_DOTDOTDOT",             "TK_DOTDOT",
    "TK_PLUSPLUS",              "TK_MINUSMINUS",
    "TK_CAST",                  "TK_TEMPLATE",
    "TK_UNDERSCORE",            "TK_AS",
    "TK_ANY",                   "TK_BREAK",
    "TK_CONST",                 "TK_CONTINUE",
    "TK_ELSE",                  "TK_ENUM",
    "TK_FN",                    "TK_FFI",
    "TK_FOR",                   "TK_IF",
    "TK_IN",                    "TK_MATCH",
    "TK_IMPORT",                "TK_FORWARD",
    "TK_PUB",                   "TK_REF",
    "TK_RETURN",                "TK_STATIC",
    "TK_STRUCT",                "TK_TYPE",
    "TK_TYPEOF",                "TK_USE",
    "TK_VAR",                   "TK_GLOBAL",
    "TK_WHERE",                 "TK_WHILE",
    "TK_DO",                    "TK_SIZEOF",
    "TK_LOG",                   "TK_FALSE",
    "TK_TRUE",                  "TK_NULL",
    "TK_FAT_ARROW",             "AST_LIT_ID",
    "AST_LIT_INTEGER",          "AST_LIT_FLOAT",
    "AST_LIT_STR",              "AST_LIT_BYTE",
    "AST_IDENT",                "AST_SHEBANG",
    "AST_SHEBANG_LINE",         "AST_COMMENT",
    "AST_LIT_BYTE_STR",         "AST_LIT_CHAR",
    "AST_LIT_BYTE_STR_RAW",     "AST_STATIC_LIFETIME",
    "AST_LIFETIME",             "IDENT",
    "TYPE",                     "FORTYPE",
    "FOR",                      "DOTDOT",
    "RETURN",                   "'='",
    "ASSIGNAMENT",              "EQUALITY",
    "'<'",                      "'>'",
    "COMPARISON",               "'|'",
    "'^'",                      "'&'",
    "'+'",                      "'-'",
    "ADDITION",                 "MULTIPLICATION",
    "'*'",                      "'/'",
    "'%'",                      "'!'",
    "'{'",                      "'['",
    "'('",                      "'.'",
    "';'",                      "'}'",
    "','",                      "'#'",
    "':'",                      "')'",
    "'@'",                      "']'",
    "'?'",                      "$accept",
    "program",                  "stmts",
    "maybe_stmts",              "stmt",
    "import_stmt",              "var_decl",
    "struct_decl",              "struct_decl_fields",
    "struct_decl_fields_list",  "struct_decl_field",
    "attributes",               "attributes_and_fn",
    "attribute",                "fn_partial_decl",
    "fn_full_decl",             "fn_decl",
    "fn_parameters",            "fn_parameter_list",
    "fn_parameter",             "assignament_operator",
    "equality_operator",        "relational_operator",
    "additive_operator",        "multiplicative_operator",
    "unary_operator",           "primary_expression",
    "postfix_expression",       "unary_expression",
    "cast_expression",          "multiplicative_expression",
    "additive_expression",      "shift_expression",
    "relational_expression",    "equality_expression",
    "and_expression",           "exclusive_or_expression",
    "inclusive_or_expression",  "logical_and_expression",
    "logical_or_expression",    "conditional_expression",
    "assignment_expression",    "expression",
    "maybe_expression",         "maybe_argument_expression_list",
    "argument_expression_list", "block",
    "log_expression",           "expr_if",
    "expr_while",               "expr_dowhile",
    "expr_for",                 "block_or_if",
    "comment",                  "literal",
    "lit_string",               "lit_numeric",
    "lit_integer",              "type",
    "ty_primitive",             "ident",
    YY_NULLPTR};
#endif

#ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] = {
    0,   256, 257, 258, 259, 260, 261, 262, 263, 264, 265, 266, 267, 268, 269,
    270, 271, 272, 273, 274, 275, 276, 277, 278, 279, 280, 281, 282, 283, 284,
    285, 286, 287, 288, 289, 290, 291, 292, 293, 294, 295, 296, 297, 298, 299,
    300, 301, 302, 303, 304, 305, 306, 307, 308, 309, 310, 311, 312, 313, 314,
    315, 316, 317, 318, 319, 320, 321, 322, 323, 324, 325, 326, 327, 328, 329,
    330, 331, 332, 333, 334, 335, 336, 337, 61,  338, 339, 60,  62,  340, 124,
    94,  38,  43,  45,  341, 342, 42,  47,  37,  33,  123, 91,  40,  46,  59,
    125, 44,  35,  58,  41,  64,  93,  63};
#endif

#define YYPACT_NINF -68

#define yypact_value_is_default(Yystate) (!!((Yystate) == (-68)))

#define YYTABLE_NINF -165

#define yytable_value_is_error(Yytable_value) 0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] = {
    312, 368, 424, 424, -66, -16, -43, -18, -68, -68, 368, 368, 27,  21,  13,
    -16, 16,  16,  368, -33, -31, 368, -68, -68, -68, -68, -68, -68, -68, -68,
    -68, -68, -68, -68, -68, 312, 368, -68, -16, -33, 82,  312, -68, -13, -7,
    -68, -6,  -16, -68, -60, -33, -68, 368, -68, 19,  123, -68, -8,  35,  45,
    24,  55,  12,  18,  36,  92,  7,   -68, -68, 9,   25,  -68, -68, -68, -68,
    -68, -68, -68, -68, -68, -68, -68, -68, -68, -68, 17,  28,  -68, 32,  -68,
    34,  43,  -33, -68, 27,  -68, 44,  49,  -68, -34, -16, 14,  73,  -16, -68,
    -33, 312, 102, 17,  -68, 52,  224, -68, 50,  79,  -68, -68, -68, -68, -68,
    -68, -68, 67,  17,  -68, -68, -68, -68, -68, 368, 368, -15, -68, -68, -68,
    -68, -68, -68, -68, -68, -68, -68, -68, 368, -68, -68, -68, 368, -68, -68,
    368, 368, 368, -68, -68, -68, -68, 368, -68, -68, 368, 368, 368, 368, 368,
    368, 368, -68, -68, -68, 65,  -68, -68, -68, 368, 141, -68, -68, 16,  -68,
    93,  17,  368, 95,  -68, 312, 75,  368, 74,  368, -68, -68, 130, 15,  -68,
    -68, 71,  88,  76,  52,  -68, -68, -68, -68, -8,  35,  35,  45,  24,  55,
    12,  18,  36,  92,  78,  368, 89,  -4,  40,  -68, -16, -68, 368, 91,  -68,
    368, -68, 97,  -68, -68, -68, -68, -49, -68, -16, -68, -68, -68, -68, 368,
    -68, 368, -68, -68, -68, -68, -3,  -68, -68, -68, -68, -68, -2,  -68, -68,
    -68, -33, -68, -68, 94,  -68, -68, -68, -68};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] = {
    0,   0,   0,   0,   0,   0,   0,   0,   46,  49,  0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   156, 155, 154, 161, 159, 157, 158, 166,
    150, 89,  86,  88,  87,  0,   136, 7,   0,   0,   0,   2,   3,   0,   0,
    9,   0,   0,   43,  0,   53,  10,  0,   93,  101, 107, 109, 111, 113, 116,
    118, 120, 122, 124, 126, 128, 130, 132, 134, 0,   0,   15,  16,  17,  18,
    14,  90,  152, 153, 160, 151, 107, 105, 102, 103, 0,   0,   24,  0,   20,
    0,   0,   0,   27,  0,   22,  0,   0,   165, 166, 0,   162, 32,  0,   34,
    0,   6,   0,   0,   139, 142, 0,   135, 0,   48,  92,  1,   4,   19,  8,
    45,  44,  55,  0,   54,  52,  104, 99,  100, 0,   137, 0,   65,  66,  67,
    68,  69,  70,  71,  72,  73,  74,  64,  0,   83,  84,  85,  0,   81,  82,
    0,   0,   0,   79,  80,  77,  78,  0,   75,  76,  0,   0,   0,   0,   0,
    0,   0,   13,  12,  164, 0,   26,  25,  21,  0,   143, 28,  23,  40,  35,
    31,  0,   0,   0,   145, 5,   0,   0,   0,   0,   11,  91,  0,   0,   50,
    51,  0,   160, 0,   138, 98,  97,  133, 110, 112, 114, 115, 117, 119, 121,
    123, 125, 127, 129, 0,   0,   0,   0,   0,   38,  0,   42,  0,   0,   30,
    0,   141, 0,   106, 140, 47,  56,  0,   60,  0,   63,  94,  95,  96,  0,
    108, 0,   148, 149, 144, 36,  0,   41,  29,  163, 33,  146, 0,   57,  62,
    131, 0,   37,  39,  0,   58,  61,  147, 59};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] = {
    -68, -68, -14, -68, -24, -68, -68, -68, -68, -68, -44, -68, -68,
    158, -68, -68, -68, -68, -68, -47, -68, -68, -68, -68, -68, -68,
    -68, -68, 3,   23,  57,  0,   51,  53,  48,  54,  47,  58,  46,
    -68, -27, -12, 5,   -68, -68, 84,  -36, -68, 2,   -68, -68, -68,
    -68, -68, 29,  6,   -68, -67, -9,  -68, -5};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] = {
    -1,  40, 41, 186, 42,  43,  44,  45,  179, 218, 219, 46,  47,
    48,  49, 50, 51,  194, 232, 233, 143, 160, 157, 150, 147, 52,
    53,  54, 55, 56,  57,  58,  59,  60,  61,  62,  63,  64,  65,
    66,  67, 68, 69,  113, 198, 110, 107, 70,  71,  72,  73,  74,
    244, 75, 76, 77,  78,  79,  220, 101, 80};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] = {
    86,  88,  90,  115, 81,   83,  84,  100, 103,  109, 97,  102, 104, 124, 125,
    91,  92,  117, 93,  96,   259, 111, 165, 105,  82,  29,  1,   98,  98,  153,
    120, 9,   154, 114, -164, 11,  85,  2,   3,    4,   106, 112, 122, 127, 128,
    98,  98,  98,  123, 25,   29,  151, 29,  29,   152, 81,  175, 252, 158, 159,
    253, 87,  197, 94,  200,  99,  99,  106, -164, 184, 20,  108, 22,  23,  24,
    126, 170, 25,  26,  27,   28,  29,  116, 99,   99,  169, 89,  117, 144, 145,
    146, 118, 185, 27,  28,   180, 106, 119, 183,  188, 176, 38,  257, 161, 31,
    164, 32,  260, 162, 33,   155, 156, 34,  167,  195, 36,  181, 95,  109, 166,
    129, 130, 131, 39,  231,  163, 201, 148, 149,  168, 132, 202, 171, 133, 196,
    134, 172, 135, 173, 136,  137, 138, 139, 140,  141, 245, 246, 174, 177, 178,
    81,  205, 206, 81,  81,   81,  182, 187, 189,  191, 81,  117, 192, 81,  81,
    81,  81,  81,  81,  193,  203, 214, 223, 221,  215, 217, 222, 229, 225, 216,
    226, 242, 236, 228, 234,  238, 239, 224, 235,  22,  23,  24,  227, 241, 25,
    26,  27,  28,  29,  237,  249, 251, 258, 263,  121, 261, 142, 204, 207, 209,
    211, 213, 255, 208, 199,  247, 210, 0,   81,   243, 262, 230, 212, 0,   0,
    0,   0,   248, 0,   254,  250, 0,   0,   0,    0,   0,   0,   1,   240, 0,
    0,   221, 81,  234, 0,    0,   256, 235, 2,    3,   4,   5,   0,   0,   0,
    6,   0,   7,   0,   0,    8,   9,   10,  11,   0,   0,   12,  13,  0,   0,
    14,  0,   15,  0,   0,    0,   16,  17,  0,    18,  19,  20,  21,  22,  23,
    24,  0,   0,   25,  26,   27,  28,  29,  0,    0,   30,  0,   0,   0,   0,
    0,   0,   0,   0,   0,    0,   0,   0,   0,    0,   0,   0,   0,   0,   0,
    31,  0,   32,  0,   0,    33,  0,   0,   34,   35,  1,   36,  0,   37,  190,
    0,   38,  0,   0,   39,   0,   2,   3,   4,    5,   0,   0,   0,   6,   0,
    7,   0,   0,   8,   9,    10,  11,  0,   0,    12,  13,  0,   0,   14,  0,
    15,  0,   0,   0,   16,   17,  0,   18,  19,   20,  21,  22,  23,  24,  0,
    0,   25,  26,  27,  28,   29,  1,   0,   30,   0,   0,   0,   0,   0,   0,
    0,   0,   2,   3,   4,    0,   0,   0,   0,    0,   0,   0,   0,   31,  0,
    32,  0,   0,   33,  0,    0,   34,  35,  0,    36,  0,   37,  0,   0,   38,
    0,   0,   39,  0,   0,    20,  0,   22,  23,   24,  0,   0,   25,  26,  27,
    28,  29,  1,   0,   0,    0,   0,   0,   0,    0,   0,   0,   0,   2,   3,
    0,   0,   0,   0,   0,    0,   0,   0,   0,    31,  0,   32,  0,   0,   33,
    0,   0,   34,  0,   0,    36,  0,   0,   0,    0,   0,   0,   0,   39,  0,
    0,   20,  0,   22,  23,   24,  0,   0,   25,   26,  27,  28,  29,  0,   0,
    0,   0,   0,   0,   0,    0,   0,   0,   0,    0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,    31,  0,   32,  0,    0,   33,  0,   0,   34,  0,
    0,   36,  0,   0,   0,    0,   0,   0,   0,    39};

static const yytype_int16 yycheck[] = {
    5,   6,   7,   39,  1,   2,   3,   16,  17,  21,  15,  16,  17,  49,  50,
    10,  11,  41,  12,  14,  22,  35,  15,  18,  1,   68,  13,  30,  30,  5,
    36,  37,  8,   38,  68,  39,  102, 24,  25,  26,  100, 36,  47,  24,  25,
    30,  30,  30,  108, 64,  68,  6,   68,  68,  9,   52,  92,  106, 3,   4,
    109, 104, 129, 42,  131, 68,  68,  100, 102, 105, 57,  102, 59,  60,  61,
    52,  85,  64,  65,  66,  67,  68,  0,   68,  68,  68,  104, 111, 96,  97,
    98,  104, 106, 66,  67,  100, 100, 104, 103, 108, 94,  107, 105, 91,  91,
    13,  93,  109, 90,  96,  86,  87,  99,  104, 123, 102, 102, 104, 130, 112,
    101, 102, 103, 110, 109, 89,  131, 92,  93,  104, 7,   143, 104, 10,  129,
    12,  104, 14,  104, 16,  17,  18,  19,  20,  21,  105, 106, 104, 104, 100,
    147, 151, 152, 150, 151, 152, 83,  55,  106, 109, 157, 185, 83,  160, 161,
    162, 163, 164, 165, 102, 147, 166, 181, 178, 109, 34,  83,  189, 83,  174,
    105, 217, 111, 109, 193, 109, 108, 182, 193, 59,  60,  61,  187, 104, 64,
    65,  66,  67,  68,  111, 109, 104, 246, 109, 46,  252, 83,  150, 157, 161,
    163, 165, 239, 160, 130, 220, 162, -1,  215, 217, 256, 192, 164, -1,  -1,
    -1,  -1,  222, -1,  234, 225, -1,  -1,  -1,  -1,  -1,  -1,  13,  215, -1,
    -1,  246, 239, 252, -1,  -1,  241, 252, 24,  25,  26,  27,  -1,  -1,  -1,
    31,  -1,  33,  -1,  -1,  36,  37,  38,  39,  -1,  -1,  42,  43,  -1,  -1,
    46,  -1,  48,  -1,  -1,  -1,  52,  53,  -1,  55,  56,  57,  58,  59,  60,
    61,  -1,  -1,  64,  65,  66,  67,  68,  -1,  -1,  71,  -1,  -1,  -1,  -1,
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
    91,  -1,  93,  -1,  -1,  96,  -1,  -1,  99,  100, 13,  102, -1,  104, 105,
    -1,  107, -1,  -1,  110, -1,  24,  25,  26,  27,  -1,  -1,  -1,  31,  -1,
    33,  -1,  -1,  36,  37,  38,  39,  -1,  -1,  42,  43,  -1,  -1,  46,  -1,
    48,  -1,  -1,  -1,  52,  53,  -1,  55,  56,  57,  58,  59,  60,  61,  -1,
    -1,  64,  65,  66,  67,  68,  13,  -1,  71,  -1,  -1,  -1,  -1,  -1,  -1,
    -1,  -1,  24,  25,  26,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  91,  -1,
    93,  -1,  -1,  96,  -1,  -1,  99,  100, -1,  102, -1,  104, -1,  -1,  107,
    -1,  -1,  110, -1,  -1,  57,  -1,  59,  60,  61,  -1,  -1,  64,  65,  66,
    67,  68,  13,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  24,  25,
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  91,  -1,  93,  -1,  -1,  96,
    -1,  -1,  99,  -1,  -1,  102, -1,  -1,  -1,  -1,  -1,  -1,  -1,  110, -1,
    -1,  57,  -1,  59,  60,  61,  -1,  -1,  64,  65,  66,  67,  68,  -1,  -1,
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
    -1,  -1,  -1,  -1,  -1,  91,  -1,  93,  -1,  -1,  96,  -1,  -1,  99,  -1,
    -1,  102, -1,  -1,  -1,  -1,  -1,  -1,  -1,  110};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] = {
    0,   13,  24,  25,  26,  27,  31,  33,  36,  37,  38,  39,  42,  43,  46,
    48,  52,  53,  55,  56,  57,  58,  59,  60,  61,  64,  65,  66,  67,  68,
    71,  91,  93,  96,  99,  100, 102, 104, 107, 110, 114, 115, 117, 118, 119,
    120, 124, 125, 126, 127, 128, 129, 138, 139, 140, 141, 142, 143, 144, 145,
    146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 160, 161, 162, 163, 164,
    166, 167, 168, 169, 170, 173, 141, 142, 141, 141, 102, 173, 104, 173, 104,
    173, 155, 155, 168, 42,  104, 155, 173, 30,  68,  171, 172, 173, 171, 173,
    155, 100, 159, 102, 154, 158, 115, 155, 156, 173, 159, 0,   117, 104, 104,
    36,  126, 173, 108, 159, 159, 142, 24,  25,  101, 102, 103, 7,   10,  12,
    14,  16,  17,  18,  19,  20,  21,  83,  133, 96,  97,  98,  137, 92,  93,
    136, 6,   9,   5,   8,   86,  87,  135, 3,   4,   134, 91,  90,  89,  13,
    15,  112, 104, 104, 68,  171, 104, 104, 104, 104, 159, 168, 104, 100, 121,
    173, 102, 83,  173, 159, 115, 116, 55,  171, 106, 105, 109, 83,  102, 130,
    171, 155, 170, 157, 158, 170, 173, 154, 142, 143, 144, 144, 145, 146, 147,
    148, 149, 150, 151, 155, 109, 155, 34,  122, 123, 171, 173, 83,  171, 155,
    83,  105, 155, 109, 154, 167, 109, 131, 132, 171, 173, 111, 111, 109, 108,
    142, 104, 159, 161, 165, 105, 106, 173, 155, 109, 155, 104, 106, 109, 173,
    153, 155, 105, 123, 22,  109, 132, 159, 109};

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] = {
    0,   113, 114, 115, 115, 116, 116, 117, 117, 117, 117, 117, 117, 117,
    117, 117, 117, 117, 117, 117, 117, 117, 117, 117, 117, 117, 117, 118,
    118, 119, 119, 119, 119, 119, 119, 120, 121, 121, 122, 122, 122, 123,
    123, 124, 124, 125, 125, 126, 126, 126, 127, 128, 129, 129, 129, 130,
    130, 130, 130, 130, 131, 131, 132, 132, 133, 133, 133, 133, 133, 133,
    133, 133, 133, 133, 133, 134, 134, 135, 135, 135, 135, 136, 136, 137,
    137, 137, 138, 138, 138, 138, 139, 139, 139, 140, 140, 140, 140, 140,
    140, 140, 140, 141, 141, 141, 141, 141, 141, 142, 142, 143, 143, 144,
    144, 145, 145, 145, 146, 146, 147, 147, 148, 148, 149, 149, 150, 150,
    151, 151, 152, 152, 153, 153, 154, 154, 155, 156, 156, 157, 157, 158,
    158, 159, 160, 161, 161, 162, 163, 164, 165, 165, 166, 167, 167, 167,
    167, 167, 167, 168, 168, 169, 169, 170, 171, 171, 172, 172, 173};

/* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] = {
    0, 2, 1, 1, 2, 1, 0, 1, 2, 1, 1, 3, 2, 2, 1, 1, 1, 1, 1, 2, 2, 3, 2, 3,
    2, 3, 3, 2, 3, 5, 4, 3, 2, 5, 2, 3, 3, 4, 1, 3, 0, 2, 1, 1, 2, 2, 1, 4,
    2, 1, 3, 3, 2, 1, 2, 0, 2, 3, 4, 5, 1, 3, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 2, 1, 4, 4,
    4, 3, 3, 2, 2, 1, 2, 2, 2, 2, 4, 1, 5, 1, 3, 1, 3, 1, 3, 3, 1, 3, 1, 3,
    1, 3, 1, 3, 1, 3, 1, 3, 1, 3, 1, 5, 1, 3, 1, 1, 0, 0, 1, 1, 3, 3, 2, 3,
    5, 3, 5, 7, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 4, 1, 1, 1};

#define yyerrok (yyerrstatus = 0)
#define yyclearin (yychar = YYEMPTY)
#define YYEMPTY (-2)
#define YYEOF 0

#define YYACCEPT goto yyacceptlab
#define YYABORT goto yyabortlab
#define YYERROR goto yyerrorlab

#define YYRECOVERING() (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                                 \
  do                                                                           \
    if (yychar == YYEMPTY) {                                                   \
      yychar = (Token);                                                        \
      yylval = (Value);                                                        \
      YYPOPSTACK(yylen);                                                       \
      yystate = *yyssp;                                                        \
      goto yybackup;                                                           \
    } else {                                                                   \
      yyerror(root, YY_("syntax error: cannot back up"));                      \
      YYERROR;                                                                 \
    }                                                                          \
  while (0)

/* Error token number */
#define YYTERROR 1
#define YYERRCODE 256

/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#ifndef YYLLOC_DEFAULT
#define YYLLOC_DEFAULT(Current, Rhs, N)                                        \
  do                                                                           \
    if (N) {                                                                   \
      (Current).first_line = YYRHSLOC(Rhs, 1).first_line;                      \
      (Current).first_column = YYRHSLOC(Rhs, 1).first_column;                  \
      (Current).last_line = YYRHSLOC(Rhs, N).last_line;                        \
      (Current).last_column = YYRHSLOC(Rhs, N).last_column;                    \
    } else {                                                                   \
      (Current).first_line = (Current).last_line = YYRHSLOC(Rhs, 0).last_line; \
      (Current).first_column = (Current).last_column =                         \
          YYRHSLOC(Rhs, 0).last_column;                                        \
    }                                                                          \
  while (0)
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K])

/* Enable debugging if requested.  */
#if YYDEBUG

#ifndef YYFPRINTF
#include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#define YYFPRINTF fprintf
#endif

#define YYDPRINTF(Args)                                                        \
  do {                                                                         \
    if (yydebug)                                                               \
      YYFPRINTF Args;                                                          \
  } while (0)

/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
#if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL

/* Print *YYLOCP on YYO.  Private, do not rely on its existence. */

YY_ATTRIBUTE_UNUSED
static unsigned yy_location_print_(FILE* yyo, YYLTYPE const* const yylocp) {
  unsigned res = 0;
  int end_col = 0 != yylocp->last_column ? yylocp->last_column - 1 : 0;
  if (0 <= yylocp->first_line) {
    res += YYFPRINTF(yyo, "%d", yylocp->first_line);
    if (0 <= yylocp->first_column)
      res += YYFPRINTF(yyo, ".%d", yylocp->first_column);
  }
  if (0 <= yylocp->last_line) {
    if (yylocp->first_line < yylocp->last_line) {
      res += YYFPRINTF(yyo, "-%d", yylocp->last_line);
      if (0 <= end_col)
        res += YYFPRINTF(yyo, ".%d", end_col);
    } else if (0 <= end_col && yylocp->first_column < end_col)
      res += YYFPRINTF(yyo, "-%d", end_col);
  }
  return res;
}

#define YY_LOCATION_PRINT(File, Loc) yy_location_print_(File, &(Loc))

#else
#define YY_LOCATION_PRINT(File, Loc) ((void)0)
#endif
#endif

#define YY_SYMBOL_PRINT(Title, Type, Value, Location)                          \
  do {                                                                         \
    if (yydebug) {                                                             \
      YYFPRINTF(stderr, "%s ", Title);                                         \
      yy_symbol_print(stderr, Type, Value, Location, root);                    \
      YYFPRINTF(stderr, "\n");                                                 \
    }                                                                          \
  } while (0)

/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void yy_symbol_value_print(FILE* yyoutput, int yytype,
                                  YYSTYPE const* const yyvaluep,
                                  YYLTYPE const* const yylocationp,
                                  ast_t** root) {
  FILE* yyo = yyoutput;
  YYUSE(yyo);
  YYUSE(yylocationp);
  YYUSE(root);
  if (!yyvaluep)
    return;
#ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT(yyoutput, yytoknum[yytype], *yyvaluep);
#endif
  YYUSE(yytype);
}

/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

static void yy_symbol_print(FILE* yyoutput, int yytype,
                            YYSTYPE const* const yyvaluep,
                            YYLTYPE const* const yylocationp, ast_t** root) {
  YYFPRINTF(yyoutput, "%s %s (", yytype < YYNTOKENS ? "token" : "nterm",
            yytname[yytype]);

  YY_LOCATION_PRINT(yyoutput, *yylocationp);
  YYFPRINTF(yyoutput, ": ");
  yy_symbol_value_print(yyoutput, yytype, yyvaluep, yylocationp, root);
  YYFPRINTF(yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void yy_stack_print(yytype_int16* yybottom, yytype_int16* yytop) {
  YYFPRINTF(stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++) {
    int yybot = *yybottom;
    YYFPRINTF(stderr, " %d", yybot);
  }
  YYFPRINTF(stderr, "\n");
}

#define YY_STACK_PRINT(Bottom, Top)                                            \
  do {                                                                         \
    if (yydebug)                                                               \
      yy_stack_print((Bottom), (Top));                                         \
  } while (0)

/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void yy_reduce_print(yytype_int16* yyssp, YYSTYPE* yyvsp, YYLTYPE* yylsp,
                            int yyrule, ast_t** root) {
  unsigned long int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF(stderr, "Reducing stack by rule %d (line %lu):\n", yyrule - 1,
            yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++) {
    YYFPRINTF(stderr, "   $%d = ", yyi + 1);
    yy_symbol_print(stderr, yystos[yyssp[yyi + 1 - yynrhs]],
                    &(yyvsp[(yyi + 1) - (yynrhs)]),
                    &(yylsp[(yyi + 1) - (yynrhs)]), root);
    YYFPRINTF(stderr, "\n");
  }
}

#define YY_REDUCE_PRINT(Rule)                                                  \
  do {                                                                         \
    if (yydebug)                                                               \
      yy_reduce_print(yyssp, yyvsp, yylsp, Rule, root);                        \
  } while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
#define YYDPRINTF(Args)
#define YY_SYMBOL_PRINT(Title, Type, Value, Location)
#define YY_STACK_PRINT(Bottom, Top)
#define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */

/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
#define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
#define YYMAXDEPTH 10000
#endif

#if YYERROR_VERBOSE

#ifndef yystrlen
#if defined __GLIBC__ && defined _STRING_H
#define yystrlen strlen
#else
/* Return the length of YYSTR.  */
static YYSIZE_T yystrlen(const char* yystr) {
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#endif
#endif

#ifndef yystpcpy
#if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#define yystpcpy stpcpy
#else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char* yystpcpy(char* yydest, const char* yysrc) {
  char* yyd = yydest;
  const char* yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#endif
#endif

#ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T yytnamerr(char* yyres, const char* yystr) {
  if (*yystr == '"') {
    YYSIZE_T yyn = 0;
    char const* yyp = yystr;

    for (;;)
      switch (*++yyp) {
      case '\'':
      case ',':
        goto do_not_strip_quotes;

      case '\\':
        if (*++yyp != '\\')
          goto do_not_strip_quotes;
      /* Fall through.  */
      default:
        if (yyres)
          yyres[yyn] = *yyp;
        yyn++;
        break;

      case '"':
        if (yyres)
          yyres[yyn] = '\0';
        return yyn;
      }
  do_not_strip_quotes:
    ;
  }

  if (!yyres)
    return yystrlen(yystr);

  return yystpcpy(yyres, yystr) - yyres;
}
#endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int yysyntax_error(YYSIZE_T* yymsg_alloc, char** yymsg,
                          yytype_int16* yyssp, int yytoken) {
  YYSIZE_T yysize0 = yytnamerr(YY_NULLPTR, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char* yyformat = YY_NULLPTR;
  /* Arguments of yyformat. */
  char const* yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY) {
    int yyn = yypact[*yyssp];
    yyarg[yycount++] = yytname[yytoken];
    if (!yypact_value_is_default(yyn)) {
      /* Start YYX at -YYN if negative to avoid negative indexes in
         YYCHECK.  In other words, skip the first -YYN actions for
         this state because they are default actions.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;
      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yyx;

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
        if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR &&
            !yytable_value_is_error(yytable[yyx + yyn])) {
          if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM) {
            yycount = 1;
            yysize = yysize0;
            break;
          }
          yyarg[yycount++] = yytname[yyx];
          {
            YYSIZE_T yysize1 = yysize + yytnamerr(YY_NULLPTR, yytname[yyx]);
            if (!(yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
              return 2;
            yysize = yysize1;
          }
        }
    }
  }

  switch (yycount) {
#define YYCASE_(N, S)                                                          \
  case N:                                                                      \
    yyformat = S;                                                              \
    break
    YYCASE_(0, YY_("syntax error"));
    YYCASE_(1, YY_("syntax error, unexpected %s"));
    YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
    YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
    YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
    YYCASE_(5,
            YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
  }

  {
    YYSIZE_T yysize1 = yysize + yystrlen(yyformat);
    if (!(yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
  }

  if (*yymsg_alloc < yysize) {
    *yymsg_alloc = 2 * yysize;
    if (!(yysize <= *yymsg_alloc && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
      *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
    return 1;
  }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char* yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount) {
        yyp += yytnamerr(yyp, yyarg[yyi++]);
        yyformat += 2;
      } else {
        yyp++;
        yyformat++;
      }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void yydestruct(const char* yymsg, int yytype, YYSTYPE* yyvaluep,
                       YYLTYPE* yylocationp, ast_t** root) {
  YYUSE(yyvaluep);
  YYUSE(yylocationp);
  YYUSE(root);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT(yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE(yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}

/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Location data for the lookahead symbol.  */
YYLTYPE yylloc
#if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
    = {1, 1, 1, 1}
#endif
;
/* Number of syntax errors so far.  */
int yynerrs;

/*----------.
| yyparse.  |
`----------*/

int yyparse(ast_t** root) {
  int yystate;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;

  /* The stacks and their tools:
     'yyss': related to states.
     'yyvs': related to semantic values.
     'yyls': related to locations.

     Refer to the stacks through separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  yytype_int16 yyssa[YYINITDEPTH];
  yytype_int16* yyss;
  yytype_int16* yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE* yyvs;
  YYSTYPE* yyvsp;

  /* The location stack.  */
  YYLTYPE yylsa[YYINITDEPTH];
  YYLTYPE* yyls;
  YYLTYPE* yylsp;

  /* The locations where the error started and ended.  */
  YYLTYPE yyerror_range[3];

  YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
  YYLTYPE yyloc;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char* yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N) (yyvsp -= (N), yyssp -= (N), yylsp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yylsp = yyls = yylsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  yylsp[0] = yylloc;
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp) {
    /* Get the current used size of the three stacks, in elements.  */
    YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
    {
      /* Give user a chance to reallocate the stack.  Use copies of
         these so that the &'s don't force the real ones into
         memory.  */
      YYSTYPE* yyvs1 = yyvs;
      yytype_int16* yyss1 = yyss;
      YYLTYPE* yyls1 = yyls;

      /* Each stack pointer address is followed by the size of the
         data in use in that stack, in bytes.  This used to be a
         conditional around just the two extra args, but that might
         be undefined if yyoverflow is a macro.  */
      yyoverflow(YY_("memory exhausted"), &yyss1, yysize * sizeof(*yyssp),
                 &yyvs1, yysize * sizeof(*yyvsp), &yyls1,
                 yysize * sizeof(*yylsp), &yystacksize);

      yyls = yyls1;
      yyss = yyss1;
      yyvs = yyvs1;
    }
#else /* no yyoverflow */
#ifndef YYSTACK_RELOCATE
    goto yyexhaustedlab;
#else
    /* Extend the stack our own way.  */
    if (YYMAXDEPTH <= yystacksize)
      goto yyexhaustedlab;
    yystacksize *= 2;
    if (YYMAXDEPTH < yystacksize)
      yystacksize = YYMAXDEPTH;

    {
      yytype_int16* yyss1 = yyss;
      union yyalloc* yyptr =
          (union yyalloc*)YYSTACK_ALLOC(YYSTACK_BYTES(yystacksize));
      if (!yyptr)
        goto yyexhaustedlab;
      YYSTACK_RELOCATE(yyss_alloc, yyss);
      YYSTACK_RELOCATE(yyvs_alloc, yyvs);
      YYSTACK_RELOCATE(yyls_alloc, yyls);
#undef YYSTACK_RELOCATE
      if (yyss1 != yyssa)
        YYSTACK_FREE(yyss1);
    }
#endif
#endif /* no yyoverflow */

    yyssp = yyss + yysize - 1;
    yyvsp = yyvs + yysize - 1;
    yylsp = yyls + yysize - 1;

    YYDPRINTF((stderr, "Stack size increased to %lu\n",
               (unsigned long int)yystacksize));

    if (yyss + yystacksize - 1 <= yyssp)
      YYABORT;
  }

  YYDPRINTF((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default(yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY) {
    YYDPRINTF((stderr, "Reading a token: "));
    yychar = yylex();
  }

  if (yychar <= YYEOF) {
    yychar = yytoken = YYEOF;
    YYDPRINTF((stderr, "Now at end of input.\n"));
  } else {
    yytoken = YYTRANSLATE(yychar);
    YY_SYMBOL_PRINT("Next token is", yytoken, &yylval, &yylloc);
  }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0) {
    if (yytable_value_is_error(yyn))
      goto yyerrlab;
    yyn = -yyn;
    goto yyreduce;
  }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END
  *++yylsp = yylloc;
  goto yynewstate;

/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;

/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1 - yylen];

  /* Default location.  */
  YYLLOC_DEFAULT(yyloc, (yylsp - yylen), yylen);
  YY_REDUCE_PRINT(yyn);
  switch (yyn) {
  case 2:
#line 162 "parser.y" /* yacc.c:1646  */
  {
    // printf("stmts addr: %p\n", $1);
    ast_position((yyvsp[0].node), (yylsp[0]), (yylsp[0]));
    ast_t* block = ast_mk_block((yyvsp[0].node));
    block->block.scope = AST_SCOPE_GLOBAL;
    block->block.uids = pool_new(sizeof(hash_t));
    hash_new(block->block.uids, 100);
    ast_position(block, (yylsp[0]), (yylsp[0]));
    (*root)->program.body = block;
    ast_position(*root, (yylsp[0]), (yylsp[0]));
  }
#line 1721 "parser.c" /* yacc.c:1646  */
  break;

  case 3:
#line 176 "parser.y" /* yacc.c:1646  */
  {
    // printf("create stmt list\n");
    // create and push
    (yyval.node) = ast_mk_list();
    ast_mk_list_push((yyval.node), (yyvsp[0].node));
    ast_position((yyvsp[0].node), (yylsp[0]), (yylsp[0]));
  }
#line 1733 "parser.c" /* yacc.c:1646  */
  break;

  case 4:
#line 183 "parser.y" /* yacc.c:1646  */
  {
    // printf("create stmt\n");
    // push
    if ((yyvsp[0].node)) { // do not push empty stmts
      ast_position((yyvsp[0].node), (yylsp[0]), (yylsp[0]));
      ast_mk_list_push((yyvsp[-1].node), (yyvsp[0].node));
    }
    (yyval.node) = (yyvsp[-1].node);
  }
#line 1747 "parser.c" /* yacc.c:1646  */
  break;

  case 6:
#line 200 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) = ast_mk_list();
  }
#line 1753 "parser.c" /* yacc.c:1646  */
  break;

  case 7:
#line 204 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) = 0; // empty stmt
  }
#line 1761 "parser.c" /* yacc.c:1646  */
  break;

  case 11:
#line 210 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) = ast_mk_block((yyvsp[-1].node));
  }
#line 1769 "parser.c" /* yacc.c:1646  */
  break;

  case 20:
#line 221 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) = ast_mk_continue(0);
    ast_position((yyval.node), (yylsp[-1]), (yylsp[0]));
  }
#line 1778 "parser.c" /* yacc.c:1646  */
  break;

  case 21:
#line 225 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) = ast_mk_continue((yyvsp[-1].node));
    ast_position((yyval.node), (yylsp[-2]), (yylsp[0]));
  }
#line 1787 "parser.c" /* yacc.c:1646  */
  break;

  case 22:
#line 229 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) = ast_mk_return(0);
    ast_position((yyval.node), (yylsp[-1]), (yylsp[0]));
  }
#line 1796 "parser.c" /* yacc.c:1646  */
  break;

  case 23:
#line 233 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) = ast_mk_return((yyvsp[-1].node));
    ast_position((yyval.node), (yylsp[-2]), (yylsp[0]));
  }
#line 1805 "parser.c" /* yacc.c:1646  */
  break;

  case 24:
#line 237 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) = ast_mk_break(0);
    ast_position((yyval.node), (yylsp[-1]), (yylsp[0]));
  }
#line 1814 "parser.c" /* yacc.c:1646  */
  break;

  case 25:
#line 241 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) = ast_mk_break((yyvsp[-1].node));
    ast_position((yyval.node), (yylsp[-2]), (yylsp[0]));
  }
#line 1823 "parser.c" /* yacc.c:1646  */
  break;

  case 26:
#line 245 "parser.y" /* yacc.c:1646  */
  {
    // TODO ensure ident start with '$' ??
    (yyval.node) = ast_mk_template((yyvsp[-1].node), 0);
    ast_position((yyval.node), (yylsp[-2]), (yylsp[0]));
  }
#line 1833 "parser.c" /* yacc.c:1646  */
  break;

  case 27:
#line 253 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) = ast_mk_import((yyvsp[0].node), false);
    ast_position((yyval.node), (yylsp[-1]), (yylsp[0]));
  }
#line 1842 "parser.c" /* yacc.c:1646  */
  break;

  case 28:
#line 257 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) = ast_mk_import((yyvsp[0].node), true);
    ast_position((yyval.node), (yylsp[-2]), (yylsp[0]));
  }
#line 1851 "parser.c" /* yacc.c:1646  */
  break;

  case 29:
#line 264 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) = ast_mk_list();
    ast_t* decl =
        ast_mk_var_decl((yyvsp[-3].node), (yyvsp[-2].node), AST_SCOPE_BLOCK);
    ast_position(decl, (yylsp[-4]), (yylsp[-2]));
    ast_mk_list_push((yyval.node), decl);
    ast_t* assignament =
        ast_mk_assignament(ast_clone(decl->var.id), '=', (yyvsp[0].node));
    ast_position(assignament, (yylsp[-2]), (yylsp[0]));
    ast_mk_list_push((yyval.node), assignament);
    ast_position((yyval.node), (yylsp[-4]), (yylsp[0]));
  }
#line 1866 "parser.c" /* yacc.c:1646  */
  break;

  case 30:
#line 274 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) = ast_mk_list();
    ast_t* decl = ast_mk_var_decl(0, (yyvsp[-2].node), AST_SCOPE_BLOCK);
    ast_position(decl, (yylsp[-3]), (yylsp[-2]));
    ast_mk_list_push((yyval.node), decl);
    ast_t* assignament =
        ast_mk_assignament(ast_clone(decl->var.id), '=', (yyvsp[0].node));
    ast_position(assignament, (yylsp[-1]), (yylsp[0]));
    ast_mk_list_push((yyval.node), assignament);
    ast_position((yyval.node), (yylsp[-3]), (yylsp[0]));
  }
#line 1881 "parser.c" /* yacc.c:1646  */
  break;

  case 31:
#line 284 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) =
        ast_mk_var_decl((yyvsp[-1].node), (yyvsp[0].node), AST_SCOPE_BLOCK);
    ast_position((yyval.node), (yylsp[-2]), (yylsp[0]));
  }
#line 1890 "parser.c" /* yacc.c:1646  */
  break;

  case 32:
#line 288 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) = ast_mk_var_decl(0, (yyvsp[0].node), AST_SCOPE_BLOCK);
    ast_position((yyval.node), (yylsp[-1]), (yylsp[0]));
  }
#line 1899 "parser.c" /* yacc.c:1646  */
  break;

  case 33:
#line 292 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) = ast_mk_list();
    ast_t* decl =
        ast_mk_var_decl((yyvsp[-3].node), (yyvsp[-2].node), AST_SCOPE_GLOBAL);
    ast_position(decl, (yylsp[-4]), (yylsp[-2]));
    ast_mk_list_push((yyval.node), decl);
    ast_t* assignament =
        ast_mk_assignament(ast_clone(decl->var.id), '=', (yyvsp[0].node));
    ast_position(assignament, (yylsp[-2]), (yylsp[0]));
    ast_mk_list_push((yyval.node), assignament);
    ast_position((yyval.node), (yylsp[-4]), (yylsp[0]));
  }
#line 1914 "parser.c" /* yacc.c:1646  */
  break;

  case 34:
#line 302 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) = ast_mk_var_decl(0, (yyvsp[0].node), AST_SCOPE_GLOBAL);
    ast_position((yyval.node), (yylsp[-1]), (yylsp[0]));
  }
#line 1923 "parser.c" /* yacc.c:1646  */
  break;

  case 35:
#line 309 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) = ast_mk_struct_decl((yyvsp[-1].node), (yyvsp[0].node));
    ast_position((yyval.node), (yylsp[-2]), (yylsp[0]));
  }
#line 1932 "parser.c" /* yacc.c:1646  */
  break;

  case 36:
#line 316 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) = (yyvsp[-1].node);
    ast_position((yyval.node), (yylsp[-2]), (yylsp[0]));
  }
#line 1938 "parser.c" /* yacc.c:1646  */
  break;

  case 37:
#line 317 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) = (yyvsp[-2].node);
    ast_position((yyval.node), (yylsp[-3]), (yylsp[0]));
  }
#line 1944 "parser.c" /* yacc.c:1646  */
  break;

  case 38:
#line 321 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) = ast_mk_list();
    ast_mk_list_push((yyval.node), (yyvsp[0].node));
    ast_position((yyval.node), (yylsp[0]), (yylsp[0]));
  }
#line 1954 "parser.c" /* yacc.c:1646  */
  break;

  case 39:
#line 326 "parser.y" /* yacc.c:1646  */
  {
    ast_mk_list_push((yyvsp[-2].node), (yyvsp[0].node));
    ast_position((yyvsp[0].node), (yylsp[0]), (yylsp[0]));
    (yyval.node) = (yyvsp[-2].node);
  }
#line 1964 "parser.c" /* yacc.c:1646  */
  break;

  case 40:
#line 331 "parser.y" /* yacc.c:1646  */
  {
    yyerror(root, "syntax error, empty struct declaration");
    YYERROR;
  }
#line 1970 "parser.c" /* yacc.c:1646  */
  break;

  case 41:
#line 335 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) = ast_mk_struct_decl_field((yyvsp[0].node), (yyvsp[-1].node));
    ast_position((yyval.node), (yylsp[-1]), (yylsp[0]));
  }
#line 1976 "parser.c" /* yacc.c:1646  */
  break;

  case 42:
#line 336 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) = ast_mk_struct_decl_field((yyvsp[0].node), 0);
    ast_position((yyval.node), (yylsp[0]), (yylsp[0]));
  }
#line 1982 "parser.c" /* yacc.c:1646  */
  break;

  case 43:
#line 340 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) = ast_mk_list();
    ast_mk_list_push((yyval.node), (yyvsp[0].node));
  }
#line 1991 "parser.c" /* yacc.c:1646  */
  break;

  case 44:
#line 344 "parser.y" /* yacc.c:1646  */
  {
    ast_mk_list_push((yyvsp[-1].node), (yyvsp[0].node));
    (yyval.node) = (yyvsp[-1].node);
  }
#line 2000 "parser.c" /* yacc.c:1646  */
  break;

  case 45:
#line 351 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) = (yyvsp[-1].node);
  }
#line 2006 "parser.c" /* yacc.c:1646  */
  break;

  case 46:
#line 352 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) = 0;
  }
#line 2012 "parser.c" /* yacc.c:1646  */
  break;

  case 47:
#line 357 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) = ast_mk_attribute((yyvsp[-2].node), (yyvsp[0].node));
    ast_position((yyval.node), (yylsp[-3]), (yylsp[0]));

    // TODO do not allow attributes to repeat
    // TODO id must have a lit_id literal
  }
#line 2024 "parser.c" /* yacc.c:1646  */
  break;

  case 48:
#line 364 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) = ast_mk_attribute((yyvsp[0].node), 0);
    ast_position((yyval.node), (yylsp[-1]), (yylsp[0]));
  }
#line 2033 "parser.c" /* yacc.c:1646  */
  break;

  case 49:
#line 368 "parser.y" /* yacc.c:1646  */
  {
    ast_t* id = ast_mk_lit_id(st_newc("ffi", st_enc_utf8), false);
    (yyval.node) = ast_mk_attribute(id, 0);
    ast_position(id, (yylsp[0]), (yylsp[0]));
    ast_position((yyval.node), (yylsp[0]), (yylsp[0]));
  }
#line 2044 "parser.c" /* yacc.c:1646  */
  break;

  case 50:
#line 377 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) = ast_mk_fn_decl((yyvsp[-1].node), (yyvsp[0].node), 0, 0,
                                  (yyvsp[-2].node));
    // XXX Hack!
    if ((yyvsp[0].node)->parent == (ast_t*)1) {
      (yyval.node)->func.varargs = true;
    }

    if ((yyvsp[-2].node)) {
      if (ast_get_attribute((yyvsp[-2].node), st_newc("ffi", st_enc_utf8))) {
        (yyval.node)->func.ffi = true;
      }
    }

    ast_position((yyval.node), (yylsp[-2]), (yylsp[0]));
  }
#line 2064 "parser.c" /* yacc.c:1646  */
  break;

  case 51:
#line 395 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) = (yyvsp[-2].node);
    (yyval.node)->func.ret_type = (yyvsp[0].node);
    ast_position((yyval.node), (yylsp[-2]), (yylsp[0]));
  }
#line 2074 "parser.c" /* yacc.c:1646  */
  break;

  case 52:
#line 403 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) = (yyvsp[-1].node);
    ast_mk_fn_decl_body((yyval.node), (yyvsp[0].node));

    // cannot be a ffi funcion with a body!
    if ((yyvsp[-1].node)->func.ffi) {
      yyerror(root, "syntax error, ffi cannot have a body");
      YYERROR;
    }

    ast_position((yyval.node), (yylsp[-1]), (yylsp[0]));
  }
#line 2090 "parser.c" /* yacc.c:1646  */
  break;

  case 53:
#line 414 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) = (yyvsp[0].node);
  }
#line 2098 "parser.c" /* yacc.c:1646  */
  break;

  case 54:
#line 417 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) = (yyvsp[-1].node);
    ast_mk_fn_decl_body((yyval.node), (yyvsp[0].node));

    if ((yyvsp[-1].node)->func.ffi) {
      yyerror(root, "syntax error, ffi cannot have a body and must have "
                    "declared return type");
      YYERROR;
    }

    ast_position((yyval.node), (yylsp[-1]), (yylsp[0]));
  }
#line 2113 "parser.c" /* yacc.c:1646  */
  break;

  case 55:
#line 430 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) = ast_mk_list();
    ast_position((yyval.node), (yylsp[0]), (yylsp[0]));
  }
#line 2119 "parser.c" /* yacc.c:1646  */
  break;

  case 56:
#line 431 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) = ast_mk_list();
    ast_position((yyval.node), (yylsp[-1]), (yylsp[0]));
  }
#line 2125 "parser.c" /* yacc.c:1646  */
  break;

  case 57:
#line 432 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) = (yyvsp[-1].node);
  }
#line 2131 "parser.c" /* yacc.c:1646  */
  break;

  case 58:
#line 433 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) = (yyvsp[-2].node);
  }
#line 2137 "parser.c" /* yacc.c:1646  */
  break;

  case 59:
#line 434 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) = (yyvsp[-3].node);
    // TODO find a more elegant way?
    // use parent as hack
    (yyval.node)->parent = (ast_t*)1;
  }
#line 2148 "parser.c" /* yacc.c:1646  */
  break;

  case 60:
#line 443 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) = ast_mk_list();
    ast_mk_list_push((yyval.node), (yyvsp[0].node));
    ast_position((yyval.node), (yylsp[0]), (yylsp[0]));
  }
#line 2158 "parser.c" /* yacc.c:1646  */
  break;

  case 61:
#line 448 "parser.y" /* yacc.c:1646  */
  {
    ast_mk_list_push((yyvsp[-2].node), (yyvsp[0].node));
    ast_position((yyvsp[0].node), (yylsp[0]), (yylsp[0]));
    (yyval.node) = (yyvsp[-2].node);
  }
#line 2168 "parser.c" /* yacc.c:1646  */
  break;

  case 62:
#line 457 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) = ast_mk_fn_param((yyvsp[0].node), (yyvsp[-1].node), 0);
    ast_position((yyval.node), (yylsp[-1]), (yylsp[0]));
  }
#line 2177 "parser.c" /* yacc.c:1646  */
  break;

  case 63:
#line 461 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) = ast_mk_fn_param((yyvsp[0].node), 0, 0);
    ast_position((yyval.node), (yylsp[0]), (yylsp[0]));
  }
#line 2186 "parser.c" /* yacc.c:1646  */
  break;

  case 64:
#line 468 "parser.y" /* yacc.c:1646  */
  {
    (yyval.token) = '=';
  }
#line 2192 "parser.c" /* yacc.c:1646  */
  break;

  case 65:
#line 469 "parser.y" /* yacc.c:1646  */
  {
    (yyval.token) = TK_SHLEQ;
  }
#line 2198 "parser.c" /* yacc.c:1646  */
  break;

  case 66:
#line 470 "parser.y" /* yacc.c:1646  */
  {
    (yyval.token) = TK_SHREQ;
  }
#line 2204 "parser.c" /* yacc.c:1646  */
  break;

  case 67:
#line 471 "parser.y" /* yacc.c:1646  */
  {
    (yyval.token) = TK_MINUSEQ;
  }
#line 2210 "parser.c" /* yacc.c:1646  */
  break;

  case 68:
#line 472 "parser.y" /* yacc.c:1646  */
  {
    (yyval.token) = TK_ANDEQ;
  }
#line 2216 "parser.c" /* yacc.c:1646  */
  break;

  case 69:
#line 473 "parser.y" /* yacc.c:1646  */
  {
    (yyval.token) = TK_OREQ;
  }
#line 2222 "parser.c" /* yacc.c:1646  */
  break;

  case 70:
#line 474 "parser.y" /* yacc.c:1646  */
  {
    (yyval.token) = TK_PLUSEQ;
  }
#line 2228 "parser.c" /* yacc.c:1646  */
  break;

  case 71:
#line 475 "parser.y" /* yacc.c:1646  */
  {
    (yyval.token) = TK_STAREQ;
  }
#line 2234 "parser.c" /* yacc.c:1646  */
  break;

  case 72:
#line 476 "parser.y" /* yacc.c:1646  */
  {
    (yyval.token) = TK_SLASHEQ;
  }
#line 2240 "parser.c" /* yacc.c:1646  */
  break;

  case 73:
#line 477 "parser.y" /* yacc.c:1646  */
  {
    (yyval.token) = TK_CARETEQ;
  }
#line 2246 "parser.c" /* yacc.c:1646  */
  break;

  case 74:
#line 478 "parser.y" /* yacc.c:1646  */
  {
    (yyval.token) = TK_PERCENTEQ;
  }
#line 2252 "parser.c" /* yacc.c:1646  */
  break;

  case 75:
#line 482 "parser.y" /* yacc.c:1646  */
  {
    (yyval.token) = TK_EQEQ;
  }
#line 2258 "parser.c" /* yacc.c:1646  */
  break;

  case 76:
#line 483 "parser.y" /* yacc.c:1646  */
  {
    (yyval.token) = TK_NE;
  }
#line 2264 "parser.c" /* yacc.c:1646  */
  break;

  case 77:
#line 487 "parser.y" /* yacc.c:1646  */
  {
    (yyval.token) = '<';
  }
#line 2270 "parser.c" /* yacc.c:1646  */
  break;

  case 78:
#line 488 "parser.y" /* yacc.c:1646  */
  {
    (yyval.token) = '>';
  }
#line 2276 "parser.c" /* yacc.c:1646  */
  break;

  case 79:
#line 489 "parser.y" /* yacc.c:1646  */
  {
    (yyval.token) = TK_LE;
  }
#line 2282 "parser.c" /* yacc.c:1646  */
  break;

  case 80:
#line 490 "parser.y" /* yacc.c:1646  */
  {
    (yyval.token) = TK_GE;
  }
#line 2288 "parser.c" /* yacc.c:1646  */
  break;

  case 81:
#line 493 "parser.y" /* yacc.c:1646  */
  {
    (yyval.token) = '+';
  }
#line 2294 "parser.c" /* yacc.c:1646  */
  break;

  case 82:
#line 494 "parser.y" /* yacc.c:1646  */
  {
    (yyval.token) = '-';
  }
#line 2300 "parser.c" /* yacc.c:1646  */
  break;

  case 83:
#line 498 "parser.y" /* yacc.c:1646  */
  {
    (yyval.token) = '*';
  }
#line 2306 "parser.c" /* yacc.c:1646  */
  break;

  case 84:
#line 499 "parser.y" /* yacc.c:1646  */
  {
    (yyval.token) = '/';
  }
#line 2312 "parser.c" /* yacc.c:1646  */
  break;

  case 85:
#line 500 "parser.y" /* yacc.c:1646  */
  {
    (yyval.token) = '%';
  }
#line 2318 "parser.c" /* yacc.c:1646  */
  break;

  case 86:
#line 504 "parser.y" /* yacc.c:1646  */
  {
    (yyval.token) = '-';
  }
#line 2324 "parser.c" /* yacc.c:1646  */
  break;

  case 87:
#line 505 "parser.y" /* yacc.c:1646  */
  {
    (yyval.token) = '!';
  }
#line 2330 "parser.c" /* yacc.c:1646  */
  break;

  case 88:
#line 506 "parser.y" /* yacc.c:1646  */
  {
    (yyval.token) = '*';
  }
#line 2336 "parser.c" /* yacc.c:1646  */
  break;

  case 89:
#line 507 "parser.y" /* yacc.c:1646  */
  {
    (yyval.token) = '&';
  }
#line 2342 "parser.c" /* yacc.c:1646  */
  break;

  case 90:
#line 511 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) = (yyvsp[0].node);
    ast_position((yyval.node), (yylsp[0]), (yylsp[0]));
  }
#line 2351 "parser.c" /* yacc.c:1646  */
  break;

  case 91:
#line 515 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) = (yyvsp[-1].node);
    ast_position((yyval.node), (yylsp[-2]),
                 (yylsp[0])); // override position to include parens
  }
#line 2360 "parser.c" /* yacc.c:1646  */
  break;

  case 92:
#line 519 "parser.y" /* yacc.c:1646  */
  {
    // TODO short function decl
    // TODO move. this prio?
    fl_fatal_error("%s", "not implemented");
  }
#line 2370 "parser.c" /* yacc.c:1646  */
  break;

  case 94:
#line 528 "parser.y" /* yacc.c:1646  */
  {
    /* TODO maybe_expr? */
    (yyval.node) = ast_mk_member((yyvsp[-3].node), (yyvsp[-1].node), true);
    ast_position((yyval.node), (yylsp[-3]), (yylsp[0]));
  }
#line 2380 "parser.c" /* yacc.c:1646  */
  break;

  case 95:
#line 533 "parser.y" /* yacc.c:1646  */
  {
    /* TODO maybe_expr? */
    (yyval.node) = ast_mk_member((yyvsp[-3].node), (yyvsp[-1].node), false);
    ast_position((yyval.node), (yylsp[-3]), (yylsp[0]));
  }
#line 2390 "parser.c" /* yacc.c:1646  */
  break;

  case 96:
#line 538 "parser.y" /* yacc.c:1646  */
  {
    // TODO this cannot be assignament lhs, right ?
    (yyval.node) = ast_mk_call_expr((yyvsp[-3].node), (yyvsp[-1].node));
    ast_position((yyval.node), (yylsp[-3]), (yylsp[0]));
  }
#line 2400 "parser.c" /* yacc.c:1646  */
  break;

  case 97:
#line 543 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) = ast_mk_member((yyvsp[-2].node), (yyvsp[0].node), false);
    ast_position((yyval.node), (yylsp[-2]), (yylsp[0]));
  }
#line 2409 "parser.c" /* yacc.c:1646  */
  break;

  case 98:
#line 547 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) = ast_mk_member((yyvsp[-2].node), (yyvsp[0].node), false);
    ast_position((yyval.node), (yylsp[-2]), (yylsp[0]));
  }
#line 2418 "parser.c" /* yacc.c:1646  */
  break;

  case 99:
#line 551 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) = ast_mk_runary((yyvsp[-1].node), TK_PLUSPLUS);
    ast_position((yyval.node), (yylsp[-1]), (yylsp[0]));
  }
#line 2424 "parser.c" /* yacc.c:1646  */
  break;

  case 100:
#line 552 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) = ast_mk_runary((yyvsp[-1].node), TK_MINUSMINUS);
    ast_position((yyval.node), (yylsp[-1]), (yylsp[0]));
  }
#line 2430 "parser.c" /* yacc.c:1646  */
  break;

  case 101:
#line 559 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) = (yyvsp[0].node);
  }
#line 2436 "parser.c" /* yacc.c:1646  */
  break;

  case 102:
#line 560 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) = ast_mk_lunary((yyvsp[0].node), TK_PLUSPLUS);
    ast_position((yyval.node), (yylsp[-1]), (yylsp[0]));
  }
#line 2442 "parser.c" /* yacc.c:1646  */
  break;

  case 103:
#line 561 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) = ast_mk_lunary((yyvsp[0].node), TK_MINUSMINUS);
    ast_position((yyval.node), (yylsp[-1]), (yylsp[0]));
  }
#line 2448 "parser.c" /* yacc.c:1646  */
  break;

  case 104:
#line 562 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) = ast_mk_lunary((yyvsp[0].node), (yyvsp[-1].token));
    ast_position((yyval.node), (yylsp[-1]), (yylsp[0]));
  }
#line 2454 "parser.c" /* yacc.c:1646  */
  break;

  case 105:
#line 563 "parser.y" /* yacc.c:1646  */
  {
    ast_t* lun = ast_mk_lunary((yyvsp[0].node), '&');
    ast_position(lun, (yylsp[-1]), (yylsp[0]));

    (yyval.node) = ast_mk_lunary(lun, '&');
    ast_position((yyval.node), (yylsp[-1]), (yylsp[0]));
  }
#line 2466 "parser.c" /* yacc.c:1646  */
  break;

  case 106:
#line 574 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) = ast_mk_sizeof((yyvsp[-1].node));
    ast_position((yyval.node), (yylsp[-3]), (yylsp[0]));
  }
#line 2475 "parser.c" /* yacc.c:1646  */
  break;

  case 108:
#line 582 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) = ast_mk_cast((yyvsp[-2].node), (yyvsp[0].node));
    ast_position((yyval.node), (yylsp[-4]), (yylsp[0]));
  }
#line 2484 "parser.c" /* yacc.c:1646  */
  break;

  case 110:
#line 590 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) =
        ast_mk_binop((yyvsp[-2].node), (yyvsp[-1].token), (yyvsp[0].node));
    ast_position((yyval.node), (yylsp[-2]), (yylsp[0]));
  }
#line 2493 "parser.c" /* yacc.c:1646  */
  break;

  case 112:
#line 598 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) =
        ast_mk_binop((yyvsp[-2].node), (yyvsp[-1].token), (yyvsp[0].node));
    ast_position((yyval.node), (yylsp[-2]), (yylsp[0]));
  }
#line 2502 "parser.c" /* yacc.c:1646  */
  break;

  case 114:
#line 606 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) = ast_mk_binop((yyvsp[-2].node), TK_SHL, (yyvsp[0].node));
    ast_position((yyval.node), (yylsp[-2]), (yylsp[0]));
  }
#line 2511 "parser.c" /* yacc.c:1646  */
  break;

  case 115:
#line 610 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) = ast_mk_binop((yyvsp[-2].node), TK_SHR, (yyvsp[0].node));
    ast_position((yyval.node), (yylsp[-2]), (yylsp[0]));
  }
#line 2520 "parser.c" /* yacc.c:1646  */
  break;

  case 117:
#line 617 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) =
        ast_mk_binop((yyvsp[-2].node), (yyvsp[-1].token), (yyvsp[0].node));
    ast_position((yyval.node), (yylsp[-2]), (yylsp[0]));
  }
#line 2529 "parser.c" /* yacc.c:1646  */
  break;

  case 119:
#line 625 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) =
        ast_mk_binop((yyvsp[-2].node), (yyvsp[-1].token), (yyvsp[0].node));
    ast_position((yyval.node), (yylsp[-2]), (yylsp[0]));
  }
#line 2538 "parser.c" /* yacc.c:1646  */
  break;

  case 121:
#line 633 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) = ast_mk_binop((yyvsp[-2].node), '&', (yyvsp[0].node));
    ast_position((yyval.node), (yylsp[-2]), (yylsp[0]));
  }
#line 2547 "parser.c" /* yacc.c:1646  */
  break;

  case 123:
#line 641 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) = ast_mk_binop((yyvsp[-2].node), '^', (yyvsp[0].node));
    ast_position((yyval.node), (yylsp[-2]), (yylsp[0]));
  }
#line 2556 "parser.c" /* yacc.c:1646  */
  break;

  case 125:
#line 649 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) = ast_mk_binop((yyvsp[-2].node), '|', (yyvsp[0].node));
    ast_position((yyval.node), (yylsp[-2]), (yylsp[0]));
  }
#line 2565 "parser.c" /* yacc.c:1646  */
  break;

  case 127:
#line 657 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) = ast_mk_binop((yyvsp[-2].node), TK_ANDAND, (yyvsp[0].node));
    ast_position((yyval.node), (yylsp[-2]), (yylsp[0]));
  }
#line 2574 "parser.c" /* yacc.c:1646  */
  break;

  case 129:
#line 665 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) = ast_mk_binop((yyvsp[-2].node), TK_OROR, (yyvsp[0].node));
    ast_position((yyval.node), (yylsp[-2]), (yylsp[0]));
  }
#line 2583 "parser.c" /* yacc.c:1646  */
  break;

  case 131:
#line 673 "parser.y" /* yacc.c:1646  */
  {
    printf("TODO logical_or_expression");
    exit(99);
  }
#line 2592 "parser.c" /* yacc.c:1646  */
  break;

  case 133:
#line 681 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) = ast_mk_assignament((yyvsp[-2].node), (yyvsp[-1].token),
                                      (yyvsp[0].node));
    ast_position((yyval.node), (yylsp[-2]), (yylsp[0]));
  }
#line 2601 "parser.c" /* yacc.c:1646  */
  break;

  case 136:
#line 694 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) = 0;
  }
#line 2607 "parser.c" /* yacc.c:1646  */
  break;

  case 137:
#line 703 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) = ast_mk_list();
  }
#line 2613 "parser.c" /* yacc.c:1646  */
  break;

  case 139:
#line 708 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) = ast_mk_list();
    ast_mk_list_push((yyval.node), (yyvsp[0].node));
  }
#line 2622 "parser.c" /* yacc.c:1646  */
  break;

  case 140:
#line 712 "parser.y" /* yacc.c:1646  */
  {
    ast_mk_list_push((yyvsp[-2].node), (yyvsp[0].node));
    (yyval.node) = (yyvsp[-2].node);
  }
#line 2631 "parser.c" /* yacc.c:1646  */
  break;

  case 141:
#line 719 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) = ast_mk_block((yyvsp[-1].node));
  }
#line 2639 "parser.c" /* yacc.c:1646  */
  break;

  case 142:
#line 725 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) = ast_mk_log((yyvsp[0].node));
  }
#line 2647 "parser.c" /* yacc.c:1646  */
  break;

  case 143:
#line 731 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) = ast_mk_if((yyvsp[-1].node), (yyvsp[0].node), 0);
    ast_position((yyval.node), (yylsp[-2]), (yylsp[0]));
  }
#line 2656 "parser.c" /* yacc.c:1646  */
  break;

  case 144:
#line 735 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) =
        ast_mk_if((yyvsp[-3].node), (yyvsp[-2].node), (yyvsp[0].node));
    ast_position((yyval.node), (yylsp[-4]), (yylsp[0]));
  }
#line 2665 "parser.c" /* yacc.c:1646  */
  break;

  case 145:
#line 742 "parser.y" /* yacc.c:1646  */
  {
    /* ast_mk_loop(init, pre_cond, update, block, post_cond) */
    (yyval.node) = ast_mk_loop(0, (yyvsp[-1].node), 0, (yyvsp[0].node), 0);
    ast_position((yyval.node), (yylsp[-2]), (yylsp[0]));
  }
#line 2675 "parser.c" /* yacc.c:1646  */
  break;

  case 146:
#line 750 "parser.y" /* yacc.c:1646  */
  {
    /* ast_mk_loop(init, pre_cond, update, block, post_cond) */
    (yyval.node) = ast_mk_loop(0, 0, 0, (yyvsp[-3].node), (yyvsp[-1].node));
    ast_position((yyval.node), (yylsp[-4]), (yylsp[0]));
  }
#line 2685 "parser.c" /* yacc.c:1646  */
  break;

  case 147:
#line 758 "parser.y" /* yacc.c:1646  */
  {
    /* ast_mk_loop(init, pre_cond, update, block, post_cond) */
    (yyval.node) = ast_mk_loop((yyvsp[-5].node), (yyvsp[-3].node),
                               (yyvsp[-1].node), (yyvsp[0].node), 0);
    ast_position((yyval.node), (yylsp[-6]), (yylsp[0]));
  }
#line 2695 "parser.c" /* yacc.c:1646  */
  break;

  case 150:
#line 771 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) = ast_mk_comment(st_newc(yytext, st_enc_utf8));
    ast_position((yyval.node), (yylsp[0]), (yylsp[0]));
  }
#line 2704 "parser.c" /* yacc.c:1646  */
  break;

  case 151:
#line 782 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) = (yyvsp[0].node);
  }
#line 2712 "parser.c" /* yacc.c:1646  */
  break;

  case 154:
#line 787 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) = ast_mk_lit_null();
    ast_position((yyval.node), (yylsp[0]), (yylsp[0]));
  }
#line 2721 "parser.c" /* yacc.c:1646  */
  break;

  case 155:
#line 791 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) = ast_mk_lit_boolean(true);
    ast_position((yyval.node), (yylsp[0]), (yylsp[0]));
  }
#line 2730 "parser.c" /* yacc.c:1646  */
  break;

  case 156:
#line 795 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) = ast_mk_lit_boolean(false);
    ast_position((yyval.node), (yylsp[0]), (yylsp[0]));
  }
#line 2739 "parser.c" /* yacc.c:1646  */
  break;

  case 157:
#line 802 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) = ast_mk_lit_string(yytext, false);
    ast_position((yyval.node), (yylsp[0]), (yylsp[0]));
  }
#line 2748 "parser.c" /* yacc.c:1646  */
  break;

  case 158:
#line 806 "parser.y" /* yacc.c:1646  */
  {
    /* TODO what is the difference? */
    (yyval.node) = ast_mk_lit_string(yytext, false);
    ast_position((yyval.node), (yylsp[0]), (yylsp[0]));
  }
#line 2758 "parser.c" /* yacc.c:1646  */
  break;

  case 159:
#line 814 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) = ast_mk_lit_float(yytext);
    ast_position((yyval.node), (yylsp[0]), (yylsp[0]));
  }
#line 2767 "parser.c" /* yacc.c:1646  */
  break;

  case 161:
#line 822 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) = ast_mk_lit_integer(yytext);
    ast_position((yyval.node), (yylsp[0]), (yylsp[0]));
  }
#line 2776 "parser.c" /* yacc.c:1646  */
  break;

  case 162:
#line 830 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) = (yyvsp[0].node);
    ast_position((yyval.node), (yylsp[0]), (yylsp[0]));
  }
#line 2785 "parser.c" /* yacc.c:1646  */
  break;

  case 163:
#line 834 "parser.y" /* yacc.c:1646  */
  {
    // printf("ty_primitive<ty_primitive>\n");
    (yyvsp[-3].node)->ty.child = (yyvsp[-1].node);
    ast_position((yyvsp[-3].node), (yylsp[-3]), (yylsp[0]));
    (yyval.node) = (yyvsp[-3].node);
  }
#line 2796 "parser.c" /* yacc.c:1646  */
  break;

  case 164:
#line 843 "parser.y" /* yacc.c:1646  */
  {
    // printf("ty_primitive\n");
    (yyval.node) = ast_mk_type(array_unshift(identifiers), 0);
    ast_position((yyval.node), (yylsp[0]), (yylsp[0]));
  }
#line 2806 "parser.c" /* yacc.c:1646  */
  break;

  case 165:
#line 848 "parser.y" /* yacc.c:1646  */
  {
    (yyval.node) = ast_mk_type(0, 0); // TODO set type ?
    ast_position((yyval.node), (yylsp[0]), (yylsp[0]));
  }
#line 2815 "parser.c" /* yacc.c:1646  */
  break;

  case 166:
#line 855 "parser.y" /* yacc.c:1646  */
  {
    // printf("ident\n");
    (yyval.node) = ast_mk_lit_id(array_unshift(identifiers), true);
    ast_position((yyval.node), (yylsp[0]), (yylsp[0]));
  }
#line 2825 "parser.c" /* yacc.c:1646  */
  break;

#line 2829 "parser.c" /* yacc.c:1646  */
  default:
    break;
  }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK(yylen);
  yylen = 0;
  YY_STACK_PRINT(yyss, yyssp);

  *++yyvsp = yyval;
  *++yylsp = yyloc;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;

/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE(yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus) {
    ++yynerrs;
#if !YYERROR_VERBOSE
    yyerror(root, YY_("syntax error"));
#else
#define YYSYNTAX_ERROR yysyntax_error(&yymsg_alloc, &yymsg, yyssp, yytoken)
    {
      char const* yymsgp = YY_("syntax error");
      int yysyntax_error_status;
      yysyntax_error_status = YYSYNTAX_ERROR;
      if (yysyntax_error_status == 0)
        yymsgp = yymsg;
      else if (yysyntax_error_status == 1) {
        if (yymsg != yymsgbuf)
          YYSTACK_FREE(yymsg);
        yymsg = (char*)YYSTACK_ALLOC(yymsg_alloc);
        if (!yymsg) {
          yymsg = yymsgbuf;
          yymsg_alloc = sizeof yymsgbuf;
          yysyntax_error_status = 2;
        } else {
          yysyntax_error_status = YYSYNTAX_ERROR;
          yymsgp = yymsg;
        }
      }
      yyerror(root, yymsgp);
      if (yysyntax_error_status == 2)
        goto yyexhaustedlab;
    }
#undef YYSYNTAX_ERROR
#endif
  }

  yyerror_range[1] = yylloc;

  if (yyerrstatus == 3) {
    /* If just tried and failed to reuse lookahead token after an
       error, discard it.  */

    if (yychar <= YYEOF) {
      /* Return failure if at end of input.  */
      if (yychar == YYEOF)
        YYABORT;
    } else {
      yydestruct("Error: discarding", yytoken, &yylval, &yylloc, root);
      yychar = YYEMPTY;
    }
  }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;

/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
    goto yyerrorlab;

  yyerror_range[1] = yylsp[1 - yylen];
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK(yylen);
  yylen = 0;
  YY_STACK_PRINT(yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;

/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3; /* Each real token shifted decrements this.  */

  for (;;) {
    yyn = yypact[yystate];
    if (!yypact_value_is_default(yyn)) {
      yyn += YYTERROR;
      if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR) {
        yyn = yytable[yyn];
        if (0 < yyn)
          break;
      }
    }

    /* Pop the current state because it cannot handle the error token.  */
    if (yyssp == yyss)
      YYABORT;

    yyerror_range[1] = *yylsp;
    yydestruct("Error: popping", yystos[yystate], yyvsp, yylsp, root);
    YYPOPSTACK(1);
    yystate = *yyssp;
    YY_STACK_PRINT(yyss, yyssp);
  }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  yyerror_range[2] = yylloc;
  /* Using YYLLOC is tempting, but would change the location of
     the lookahead.  YYLOC is available though.  */
  YYLLOC_DEFAULT(yyloc, yyerror_range, 2);
  *++yylsp = yyloc;

  /* Shift the error token.  */
  YY_SYMBOL_PRINT("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;

/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror(root, YY_("memory exhausted"));
  yyresult = 2;
/* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY) {
    /* Make sure we have latest lookahead translation.  See comments at
       user semantic actions for why this is necessary.  */
    yytoken = YYTRANSLATE(yychar);
    yydestruct("Cleanup: discarding lookahead", yytoken, &yylval, &yylloc,
               root);
  }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK(yylen);
  YY_STACK_PRINT(yyss, yyssp);
  while (yyssp != yyss) {
    yydestruct("Cleanup: popping", yystos[*yyssp], yyvsp, yylsp, root);
    YYPOPSTACK(1);
  }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE(yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE(yymsg);
#endif
  return yyresult;
}
#line 862 "parser.y" /* yacc.c:1906  */
