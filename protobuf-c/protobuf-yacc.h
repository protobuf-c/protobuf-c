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
#line 74 "protobuf-c/protobuf-yacc.y" /* yacc.c:1909  */

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

#line 105 "protobuf-c/protobuf-yacc.h" /* yacc.c:1909  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_PROTOBUF_C_PROTOBUF_YACC_H_INCLUDED  */
