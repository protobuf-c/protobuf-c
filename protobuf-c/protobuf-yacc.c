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
#line 1 "protobuf-c/protobuf-yacc.y" /* yacc.c:339  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "protobuf-c.h"
#include "protobuf-c-list.h"

extern int yylex(void);
extern int yylineno;
extern ProtobufCAllocator* _allocator;
extern ProtobufCFileDescriptor* _file_descriptor;

/* make a full name including package name */
static char* make_full_name(char*);

/* lookup the type based on name */
static void* lookup_type(char*, int*);

typedef struct EnumDefinition
{
    /* the fully qualified name of the message */
    const char* name;
    /* the short name/type of the message */
    const char* short_name;              
    /* the list of enum values */
    _List enum_values_list;
    /* temporary enum descriptor */
    ProtobufCEnumDescriptor* enum_descriptor; 
} EnumDefinition;

/* contains all the definitions within one proto file */
typedef struct FileDefinition {
    /* the package name for this proto file */
    const char *package_name;            
    /* the list of ProtobufCEnumDescriptors */
    _List enum_definition_list;          
    /* the list of MessageDefinition */
    _List message_definition_list;       
    /* the list of ServiceDefinition */
    _List service_definition_list;       
} FileDefinition;

/* contains all field definitions within one message */
typedef struct MessageDefinition 
{
    /* the fully qualified name of the message */
    const char* name;                    
    /* the short name/type of the message */
    const char* short_name;              
    /* the list of ProtobufCFieldDescriptors */
    _List field_definition_list;         
    /* temporary message descriptor */
    ProtobufCMessageDescriptor* message_descriptor; 
} MessageDefinition;

/* default file defintion */
FileDefinition file_definition = { 
                                    NULL, 
                                    LIST_INITIALIZER, 
                                    LIST_INITIALIZER, 
                                    LIST_INITIALIZER 
                                 };

DEFINE_LIST(enum_values_list);

/* this contains a temporary list of message objects as they are recursively parsed */
DEFINE_LIST(message_definition_tree);

/* holds either a enum descriptor or a message definition */
static void * tmp_descriptor = NULL;

#line 138 "protobuf-c/protobuf-yacc.c" /* yacc.c:339  */

# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* In a future release of Bison, this section will be replaced
   by #include "y.tab.h".  */
#ifndef YY_YY_PROTOBUF_C_PROTOBUF_YACC_H_INCLUDED
# define YY_YY_PROTOBUF_C_PROTOBUF_YACC_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    KEYWORD_MESSAGE = 258,
    KEYWORD_PACKAGE = 259,
    KEYWORD_IMPORT = 260,
    KEYWORD_SERVICE = 261,
    KEYWORD_SYNTAX = 262,
    KEYWORD_OPTION = 263,
    KEYWORD_EXTENSIONS = 264,
    KEYWORD_ENUM = 265,
    KEYWORD_ONEOF = 266,
    KEYWORD_RPC = 267,
    KEYWORD_RETURNS = 268,
    FIELDLABEL = 269,
    FIELDTYPE = 270,
    FIELDFLAG = 271,
    INT_LITERAL = 272,
    IDENT = 273,
    BOOL_LITERAL = 274
  };
#endif
/* Tokens.  */
#define KEYWORD_MESSAGE 258
#define KEYWORD_PACKAGE 259
#define KEYWORD_IMPORT 260
#define KEYWORD_SERVICE 261
#define KEYWORD_SYNTAX 262
#define KEYWORD_OPTION 263
#define KEYWORD_EXTENSIONS 264
#define KEYWORD_ENUM 265
#define KEYWORD_ONEOF 266
#define KEYWORD_RPC 267
#define KEYWORD_RETURNS 268
#define FIELDLABEL 269
#define FIELDTYPE 270
#define FIELDFLAG 271
#define INT_LITERAL 272
#define IDENT 273
#define BOOL_LITERAL 274

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 74 "protobuf-c/protobuf-yacc.y" /* yacc.c:355  */

  int d;
  char* s;
  int b;
  ProtobufCType type;
  ProtobufCLabel label;
  ProtobufCFieldFlag flag;
  ProtobufCEnumValue *ev;
  struct EnumDefinition *ed;
  struct MessageDefinition* md;
  ProtobufCFieldDescriptor* fd;

#line 229 "protobuf-c/protobuf-yacc.c" /* yacc.c:355  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_PROTOBUF_C_PROTOBUF_YACC_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 246 "protobuf-c/protobuf-yacc.c" /* yacc.c:358  */

#ifdef short
# undef short
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
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

#ifndef YY_ATTRIBUTE
# if (defined __GNUC__                                               \
      && (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)))  \
     || defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#  define YY_ATTRIBUTE(Spec) __attribute__(Spec)
# else
#  define YY_ATTRIBUTE(Spec) /* empty */
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# define YY_ATTRIBUTE_PURE   YY_ATTRIBUTE ((__pure__))
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE ((__unused__))
#endif

#if !defined _Noreturn \
     && (!defined __STDC_VERSION__ || __STDC_VERSION__ < 201112)
# if defined _MSC_VER && 1200 <= _MSC_VER
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn YY_ATTRIBUTE ((__noreturn__))
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif


#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  20
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   76

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  30
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  17
/* YYNRULES -- Number of rules.  */
#define YYNRULES  30
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  78

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   275

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    23,     2,     2,     2,     2,     2,
      28,    29,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    20,
       2,    22,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    26,     2,    27,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    24,     2,    25,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    21
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint8 yyrline[] =
{
       0,   115,   115,   116,   117,   118,   119,   120,   123,   129,
     131,   139,   150,   156,   164,   165,   167,   179,   180,   185,
     186,   189,   202,   203,   211,   212,   214,   229,   230,   236,
     247
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "KEYWORD_MESSAGE", "KEYWORD_PACKAGE",
  "KEYWORD_IMPORT", "KEYWORD_SERVICE", "KEYWORD_SYNTAX", "KEYWORD_OPTION",
  "KEYWORD_EXTENSIONS", "KEYWORD_ENUM", "KEYWORD_ONEOF", "KEYWORD_RPC",
  "KEYWORD_RETURNS", "FIELDLABEL", "FIELDTYPE", "FIELDFLAG", "INT_LITERAL",
  "IDENT", "BOOL_LITERAL", "';'", "\".\"", "'='", "'\"'", "'{'", "'}'",
  "'['", "']'", "'('", "')'", "$accept", "proto_file_entry",
  "package_declaration", "package_name", "FULL_IDENT",
  "syntax_declaration", "message_definition", "optional_semicolon",
  "message_definition_start", "message_body", "field_definition",
  "field_type", "field_options", "enum_definition", "enum_values",
  "enum_value_entry", "service_definition", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
      59,   275,    61,    34,   123,   125,    91,    93,    40,    41
};
# endif

#define YYPACT_NINF -25

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-25)))

#define YYTABLE_NINF -1

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int8 yypact[] =
{
      14,   -12,   -11,     1,     6,     4,    23,    14,    14,    14,
       8,    14,    14,   -25,    15,    17,   -25,    11,    16,    19,
     -25,   -25,   -25,   -25,     0,   -25,   -25,   -11,   -25,    26,
      22,   -25,   -13,     0,    20,     0,     0,   -25,    24,    21,
     -17,   -25,   -25,    28,   -25,    27,   -25,   -25,    13,    29,
      30,   -25,   -25,    31,   -25,   -25,    32,   -25,    34,    37,
      33,    35,    38,    43,   -25,    41,    39,    40,    36,   -25,
      42,    45,    44,    47,    46,   -25,    50,   -25
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       2,     0,     0,     0,     0,     0,     0,     2,     2,     2,
       0,     2,     2,    16,    10,     0,     9,     0,     0,     0,
       1,     3,     4,     5,    17,     7,     6,     0,     8,     0,
       0,    27,     0,    17,     0,    17,    17,    11,     0,     0,
       0,    22,    23,     0,    20,    14,    18,    19,     0,     0,
       0,    26,    28,     0,    15,    13,     0,    12,     0,     0,
       0,     0,    24,     0,    29,     0,     0,     0,     0,    21,
       0,     0,     0,     0,     0,    25,     0,    30
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -25,    18,   -25,   -25,    49,   -25,   -24,   -25,   -25,    -2,
     -25,   -25,   -25,   -20,   -25,   -25,   -25
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,     6,     7,    15,    16,     8,     9,    55,    10,    34,
      35,    43,    66,    11,    40,    52,    12
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint8 yytable[] =
{
      33,    50,    41,     1,    36,    42,    13,    14,    51,    33,
       5,    33,    33,    36,    32,    36,    36,     1,     2,    17,
       3,     4,    19,    20,     5,    21,    22,    23,    18,    25,
      26,    44,    24,    46,    47,    29,    27,    28,    38,    30,
      39,    56,    48,    31,    49,    45,    53,    54,     0,    57,
      60,    61,    58,    59,    62,    64,    67,    68,    71,    69,
      72,     0,    63,    73,    65,     0,    76,     0,    70,     0,
       0,     0,     0,    74,    75,    77,    37
};

static const yytype_int8 yycheck[] =
{
      24,    18,    15,     3,    24,    18,    18,    18,    25,    33,
      10,    35,    36,    33,    14,    35,    36,     3,     4,    18,
       6,     7,    18,     0,    10,     7,     8,     9,    22,    11,
      12,    33,    24,    35,    36,    24,    21,    20,    12,    23,
      18,    28,    18,    24,    23,    25,    18,    20,    -1,    20,
      18,    17,    22,    22,    17,    20,    13,    16,    22,    20,
      18,    -1,    29,    18,    26,    -1,    20,    -1,    28,    -1,
      -1,    -1,    -1,    29,    27,    25,    27
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     3,     4,     6,     7,    10,    31,    32,    35,    36,
      38,    43,    46,    18,    18,    33,    34,    18,    22,    18,
       0,    31,    31,    31,    24,    31,    31,    21,    20,    24,
      23,    24,    14,    36,    39,    40,    43,    34,    12,    18,
      44,    15,    18,    41,    39,    25,    39,    39,    18,    23,
      18,    25,    45,    18,    20,    37,    28,    20,    22,    22,
      18,    17,    17,    29,    20,    26,    42,    13,    16,    20,
      28,    22,    18,    18,    29,    27,    20,    25
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    30,    31,    31,    31,    31,    31,    31,    32,    33,
      34,    34,    35,    36,    37,    37,    38,    39,    39,    39,
      39,    40,    41,    41,    42,    42,    43,    44,    44,    45,
      46
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     2,     2,     2,     2,     2,     3,     1,
       1,     3,     6,     5,     0,     1,     2,     0,     2,     2,
       2,     7,     1,     1,     0,     5,     5,     0,     2,     4,
      14
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
do                                                              \
  if (yychar == YYEMPTY)                                        \
    {                                                           \
      yychar = (Token);                                         \
      yylval = (Value);                                         \
      YYPOPSTACK (yylen);                                       \
      yystate = *yyssp;                                         \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;                                                  \
    }                                                           \
while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256



/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)

/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  YYUSE (yytype);
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, int yyrule)
{
  unsigned long int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[yyssp[yyi + 1 - yynrhs]],
                       &(yyvsp[(yyi + 1) - (yynrhs)])
                                              );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
yystrlen (const char *yystr)
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
        switch (*++yyp)
          {
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
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
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
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
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

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
{
  YYUSE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
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

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        YYSTYPE *yyvs1 = yyvs;
        yytype_int16 *yyss1 = yyss;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * sizeof (*yyssp),
                    &yyvs1, yysize * sizeof (*yyvsp),
                    &yystacksize);

        yyss = yyss1;
        yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yytype_int16 *yyss1 = yyss;
        union yyalloc *yyptr =
          (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
                  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

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
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

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
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 8:
#line 124 "protobuf-c/protobuf-yacc.y" /* yacc.c:1646  */
    { 
                        file_definition.package_name = (yyvsp[-1].s);
                    }
#line 1372 "protobuf-c/protobuf-yacc.c" /* yacc.c:1646  */
    break;

  case 10:
#line 132 "protobuf-c/protobuf-yacc.y" /* yacc.c:1646  */
    {
                int len = strlen((yyvsp[0].s));
                char* var = _allocator->alloc(_allocator->allocator_data, (len+1)*sizeof(char));
                if(!var) YYABORT;
                strncpy(var,(yyvsp[0].s), len);
                var[len]='\0';
            }
#line 1384 "protobuf-c/protobuf-yacc.c" /* yacc.c:1646  */
    break;

  case 11:
#line 140 "protobuf-c/protobuf-yacc.y" /* yacc.c:1646  */
    {
                int len = strlen((yyvsp[-2].s))+strlen((yyvsp[0].s))+1;
                char* var = _allocator->alloc(_allocator->allocator_data, (len+1)*sizeof(char));                
                if(!var) YYABORT;
                sprintf(var,"%s.%s",(yyvsp[-2].s),(yyvsp[0].s));                
                var[len]='\0';
                _allocator->free(_allocator->allocator_data,(yyvsp[0].s));                                        
            }
#line 1397 "protobuf-c/protobuf-yacc.c" /* yacc.c:1646  */
    break;

  case 12:
#line 151 "protobuf-c/protobuf-yacc.y" /* yacc.c:1646  */
    { 
                        printf("syntax %s\n",(yyvsp[-2].s));
                    }
#line 1405 "protobuf-c/protobuf-yacc.c" /* yacc.c:1646  */
    break;

  case 13:
#line 157 "protobuf-c/protobuf-yacc.y" /* yacc.c:1646  */
    { 
                              (yyval.md)=(yyvsp[-4].md);
                              if(!list_add(&file_definition.message_definition_list, (yyval.md))) YYABORT;
                              list_remove(&message_definition_tree);
                          }
#line 1415 "protobuf-c/protobuf-yacc.c" /* yacc.c:1646  */
    break;

  case 16:
#line 168 "protobuf-c/protobuf-yacc.y" /* yacc.c:1646  */
    { 
                              MessageDefinition* m = _allocator->alloc(_allocator->allocator_data, sizeof(MessageDefinition));                              
                              if(!m) YYABORT;
                              m->short_name = (yyvsp[0].s);
                              m->name = make_full_name((yyvsp[0].s));
                              if(!m->name) YYABORT;
                              (yyval.md) = m;
                              list_add(&message_definition_tree, m);
                          }
#line 1429 "protobuf-c/protobuf-yacc.c" /* yacc.c:1646  */
    break;

  case 18:
#line 181 "protobuf-c/protobuf-yacc.y" /* yacc.c:1646  */
    { 
                    MessageDefinition* m = (MessageDefinition*)message_definition_tree.last->data;
                    if(!list_add(&m->field_definition_list, (yyvsp[-1].fd))) YYABORT;
                }
#line 1438 "protobuf-c/protobuf-yacc.c" /* yacc.c:1646  */
    break;

  case 21:
#line 190 "protobuf-c/protobuf-yacc.y" /* yacc.c:1646  */
    { 
                      ProtobufCFieldDescriptor* desc = _allocator->alloc(_allocator->allocator_data, sizeof(ProtobufCFieldDescriptor));
                      if(!desc) YYABORT;
                      desc->name=(yyvsp[-4].s);
                      desc->id=(yyvsp[-2].d);
                      desc->label=(yyvsp[-6].label);
                      desc->type = (yyvsp[-5].type);
                      desc->descriptor = tmp_descriptor;
                      (yyval.fd)=desc;
                  }
#line 1453 "protobuf-c/protobuf-yacc.c" /* yacc.c:1646  */
    break;

  case 22:
#line 202 "protobuf-c/protobuf-yacc.y" /* yacc.c:1646  */
    { tmp_descriptor = NULL; (yyval.type) = (yyvsp[0].type); }
#line 1459 "protobuf-c/protobuf-yacc.c" /* yacc.c:1646  */
    break;

  case 23:
#line 204 "protobuf-c/protobuf-yacc.y" /* yacc.c:1646  */
    { 
                int is_enum = 0;
                tmp_descriptor = lookup_type((yyvsp[0].s), &is_enum);
                if(!tmp_descriptor) YYABORT;
                (yyval.type) = is_enum ? PROTOBUF_C_TYPE_ENUM : PROTOBUF_C_TYPE_MESSAGE;
            }
#line 1470 "protobuf-c/protobuf-yacc.c" /* yacc.c:1646  */
    break;

  case 26:
#line 215 "protobuf-c/protobuf-yacc.y" /* yacc.c:1646  */
    {
                      EnumDefinition* def = _allocator->alloc(_allocator->allocator_data, sizeof(EnumDefinition));
                      if(!def) YYABORT;
                      def->short_name=(yyvsp[-3].s);
                      def->name = make_full_name((yyvsp[-3].s));
                      if(!def->name) YYABORT;
                      def->enum_values_list = enum_values_list;
                      enum_values_list.first = enum_values_list.last = 0;
                      enum_values_list.length = 0;
                      (yyval.ed)=def;
                      if(!list_add(&file_definition.enum_definition_list, def)) YYABORT;
                  }
#line 1487 "protobuf-c/protobuf-yacc.c" /* yacc.c:1646  */
    break;

  case 28:
#line 231 "protobuf-c/protobuf-yacc.y" /* yacc.c:1646  */
    { 
                    if(!list_add(&enum_values_list, (yyvsp[0].ev))) YYABORT;
                }
#line 1495 "protobuf-c/protobuf-yacc.c" /* yacc.c:1646  */
    break;

  case 29:
#line 237 "protobuf-c/protobuf-yacc.y" /* yacc.c:1646  */
    {
                ProtobufCEnumValue* value = _allocator->alloc(_allocator->allocator_data, sizeof(ProtobufCEnumValue));
                if(!value) YYABORT;
                value->name = (yyvsp[-3].s);
                value->c_name = (yyvsp[-3].s);
                value->value = (yyvsp[-1].d);
                (yyval.ev) = value;
            }
#line 1508 "protobuf-c/protobuf-yacc.c" /* yacc.c:1646  */
    break;

  case 30:
#line 248 "protobuf-c/protobuf-yacc.y" /* yacc.c:1646  */
    {
                        printf("got service %s\n", (yyvsp[-12].s));
                    }
#line 1516 "protobuf-c/protobuf-yacc.c" /* yacc.c:1646  */
    break;


#line 1520 "protobuf-c/protobuf-yacc.c" /* yacc.c:1646  */
      default: break;
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
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

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
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
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

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYTERROR;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

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
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  return yyresult;
}
#line 253 "protobuf-c/protobuf-yacc.y" /* yacc.c:1906  */


static char* make_full_name(char *val) 
{
    int len = 0;                                
    if(message_definition_tree.last!=NULL) 
    {      
        MessageDefinition* def = message_definition_tree.last->data; 
        len+=strlen(def->name);                   
        len+=1;                                   
    } 
    else if(file_definition.package_name!=NULL) 
    {      
        len+=strlen(file_definition.package_name); 
        len+=1; /* for the dot */                  
    }                                             

    if(len==0)
    {
        return val;
    } 
    len+=strlen(val);                           
    char* tmp = (char*)_allocator->alloc(_allocator->allocator_data,len*sizeof(char));
    if( !tmp ) return NULL;
    char* tmp1=tmp;
    if(message_definition_tree.last!=NULL) 
    {      
        MessageDefinition* def = message_definition_tree.last->data; 
        tmp1+=sprintf(tmp1, "%s.", def->name);                   
    }
    else if(file_definition.package_name!=NULL) {    
            tmp1+=sprintf(tmp1, "%s.", file_definition.package_name);
    }

    sprintf(tmp1,"%s", val);
    return tmp;
}                              

/**
 *  Lookup type(enum or message) based on name. This will
 *  return an enum definition or a message definition object
 */
static void* lookup_type(char* name, int* is_enum)
{    
    _List* list = &file_definition.enum_definition_list;
    _ListNode* node = list->first;
    *is_enum = 0;
    while(node)
    {
        EnumDefinition* desc = node->data;
        if( strcmp(desc->short_name,name) == 0 )
        {
            *is_enum = 1;
            return desc;
        }
        node=node->next;
    }
    list = &file_definition.message_definition_list;
    node = list->first;
    while(node)
    {
        MessageDefinition* desc = node->data;
        if( strcmp(desc->short_name,name) == 0 )
        {
            return desc;
        }
        node=node->next;
    }

    return NULL;
}

void yyerror(char* s)
{
   printf("Error in parsing %s at line %d\n", s, yylineno);
   exit(1);
}

static int qsort_compare_field_names(const void *a, const void *b)
{
    return strcmp((*(ProtobufCFieldDescriptor**)a)->name, (*(ProtobufCFieldDescriptor**)b)->name);
}

static int qsort_compare_field_tags(const void *a, const void *b)
{
    return (*(ProtobufCFieldDescriptor**)a)->id - (*(ProtobufCFieldDescriptor**)b)->id;
}

static void build_message_descriptor(MessageDefinition* mdef, ProtobufCFileDescriptor* file_desc, ProtobufCMessageDescriptor* message)
{
    unsigned i = 0;

    message->name = mdef->name;
    message->short_name = mdef->short_name;
    message->package_name = file_desc->package_name;
    message->n_fields = 0;
    message->fields = NULL;

    void **array = list_flatten(&mdef->field_definition_list);

    while(1)
    {
        if( !array )
        {
            break;
        }

        /* sort the fields by tags */
        qsort(array, mdef->field_definition_list.length, sizeof(void*), qsort_compare_field_tags);

        ProtobufCFieldDescriptor* fields = _allocator->alloc(_allocator->allocator_data, 
                                mdef->field_definition_list.length*sizeof(ProtobufCFieldDescriptor));

        if(!fields) 
        {
            break;
        }

        for( ; i < mdef->field_definition_list.length; i++)
        {
            *(&fields[i]) = *((ProtobufCFieldDescriptor*)array[i]);
            if(fields[i].type == PROTOBUF_C_TYPE_MESSAGE)
            {
                fields[i].descriptor = ((MessageDefinition*)fields[i].descriptor)->message_descriptor;
            }
            else if(fields[i].type == PROTOBUF_C_TYPE_ENUM)
            {
                fields[i].descriptor = ((EnumDefinition*)fields[i].descriptor)->enum_descriptor;
            }
        }   
        message->n_fields = mdef->field_definition_list.length;
        message->fields = fields;

        /* sort the fields by tags */
        qsort(array, mdef->field_definition_list.length, sizeof(void*), qsort_compare_field_names);

        unsigned* indexes = _allocator->alloc(_allocator->allocator_data, 
                                        mdef->field_definition_list.length*sizeof(unsigned));
        if( !indexes )
        {
            break;
        }

        for( i = 0; i < mdef->field_definition_list.length; i++)
        {
            ProtobufCFieldDescriptor* tmp_desc = (ProtobufCFieldDescriptor*)array[i];
            for( int j = 0; j < mdef->field_definition_list.length; j++)
            {
                ProtobufCFieldDescriptor* val = (ProtobufCFieldDescriptor*)&message->fields[j];
                if( tmp_desc->id == val->id )
                {
                    indexes[i] = (unsigned)j;
                    break;
                }
            }
        }   
        message->fields_sorted_by_name = indexes;

        break;
    }
    if(array)
    {
        _allocator->free(_allocator->allocator_data, array);
    }
}

static int qsort_compare_enum_values(const void *a, const void *b)
{
    return (*(ProtobufCEnumValue**)a)->value - (*(ProtobufCEnumValue**)b)->value;
}

static int qsort_compare_enum_names(const void *a, const void *b)
{
    return strcmp((*(ProtobufCEnumValue**)a)->name,(*(ProtobufCEnumValue**)b)->name);
}

static void build_enum_descriptor(ProtobufCFileDescriptor* file_desc, ProtobufCEnumDescriptor* desc, EnumDefinition* def)
{
    desc->name = def->name;
    desc->short_name = def->short_name;
    desc->package_name = file_desc->package_name;
    desc->n_values = 0;
    desc->values = NULL;
    desc->n_value_names = 0;
    desc->values_by_name = NULL;

    /* flatten the linked list to array of pointers so it is easier to sort */
    void ** array = list_flatten(&def->enum_values_list);
    while(1)
    {
        if( !array )
        {
            break;            
        }

        /* first sort the array by value */
        qsort(array, def->enum_values_list.length, sizeof(void*), qsort_compare_enum_values);
        
        /* by defintion the array is of lenth atleast 1 */
        ProtobufCEnumValue* last = array[0];
        int len = 1;

        /* check for duplicates */
        for(int i = 1; i < def->enum_values_list.length; i++)
        {
            ProtobufCEnumValue* val = (ProtobufCEnumValue*)array[i];
            if( last->value != val->value )
            {
                len++;
            }
            last = val;
        }

        ProtobufCEnumValue* values = _allocator->alloc(_allocator->allocator_data, len*sizeof(ProtobufCEnumValue));
        if( !values )
        {
            break;
        }

        *(&values[0]) = *(ProtobufCEnumValue*)array[0];
        last = array[0];
        unsigned j = 1;
        for(int i = 1; i < def->enum_values_list.length; i++)
        {
            ProtobufCEnumValue* val = (ProtobufCEnumValue*)array[i];
            if( last->value == val->value )
            {
                continue;
            }
            else
            {
                *(&values[j++]) = *(ProtobufCEnumValue*)array[i];
            }
            last = val;
        }
        desc->n_values = len;
        desc->values = values;

        qsort(array, def->enum_values_list.length, sizeof(void*), qsort_compare_enum_names);
        
        ProtobufCEnumValueIndex* indexes = _allocator->alloc(_allocator->allocator_data, def->enum_values_list.length*sizeof(ProtobufCEnumValueIndex));        
        if(!indexes)
        {
            break;
        }

        for(int i = 0; i < def->enum_values_list.length; i++)
        {
            ProtobufCEnumValue* val = (ProtobufCEnumValue*)array[i];
            indexes[i].name = val->name;
            for( j = 0; j < desc->n_values; j++ )
            {
                if( desc->values[j].value == val->value )
                {
                    indexes[i].index = j;
                    break;
                }
            }            
        }
        desc->n_value_names = def->enum_values_list.length;
        desc->values_by_name = indexes;

        break;
    }   
    if( array ) 
    {
        _allocator->free(_allocator->allocator_data, array);
    }
}

static ProtobufCFileDescriptor* build_file_descriptor(const FileDefinition* def)
{
    _file_descriptor->package_name = file_definition.package_name;
    _file_descriptor->n_enums = file_definition.enum_definition_list.length;
    _file_descriptor->n_messages = file_definition.message_definition_list.length;
    _file_descriptor->enums = NULL;
    _file_descriptor->messages = NULL;

    printf("n_message %d\n", _file_descriptor->n_messages);
    printf("n_enums %d\n", _file_descriptor->n_enums);

    /* first pass. Consruct all Protobuf[Enum/Message]Descriptor pointers */
    /* for enums we can construct the full struct without any dependencies */
    if( _file_descriptor->n_enums )
    {
        ProtobufCEnumDescriptor* enums = _allocator->alloc(_allocator->allocator_data, 
                            _file_descriptor->n_enums*sizeof(ProtobufCEnumDescriptor));
        unsigned i = 0;
        _ListNode* node = file_definition.enum_definition_list.first;
        if(!enums) return NULL;
        for( i = 0; i < _file_descriptor->n_enums; i++)                            
        {
            build_enum_descriptor(_file_descriptor, &enums[i], (EnumDefinition*)node->data);
            ((EnumDefinition*)node->data)->enum_descriptor = &enums[i];
            node = node->next;
        }
        _file_descriptor->enums = enums;
    }

    /* first pass for messages */
    if( _file_descriptor->n_messages )
    {
        ProtobufCMessageDescriptor* messages = _allocator->alloc(_allocator->allocator_data, 
                            _file_descriptor->n_messages*sizeof(ProtobufCMessageDescriptor));
        unsigned i = 0;
        _ListNode* node = file_definition.message_definition_list.first;

        // first pass just save pointer to message. 
        for( i = 0; i < _file_descriptor->n_messages; i++)                            
        {
            // save the pointer
            ((MessageDefinition*)node->data)->message_descriptor = &messages[i];
            node = node->next;
        }
        _file_descriptor->messages = messages;
    }

    if( _file_descriptor->n_messages )
    {
        ProtobufCMessageDescriptor* messages = _file_descriptor->messages;
        unsigned i = 0;
        _ListNode* node = file_definition.message_definition_list.first;

        // first pass just save pointer to message. 
        for( i = 0; i < _file_descriptor->n_messages; i++)                            
        {
            build_message_descriptor(node->data, _file_descriptor, &messages[i]);
            node = node->next;
        }
    }

    list_free(&file_definition.enum_definition_list);            
    list_free(&file_definition.message_definition_list);
}

int yywrap()
{
    build_file_descriptor(&file_definition);
    return 1;
}
