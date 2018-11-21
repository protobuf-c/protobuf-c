%{
#include "protobuf-c.h"
#include "protobuf-yacc.h"
%}

%option yylineno

%%

"package"    { return KEYWORD_PACKAGE; }
"import"     { return KEYWORD_IMPORT; }
"service"    { return KEYWORD_SERVICE; }
"option"     { return KEYWORD_OPTION; }
"message"    { return KEYWORD_MESSAGE; }
"enum"       { return KEYWORD_ENUM; }
"oneof"      { return KEYWORD_ONEOF; }
"syntax"     { return KEYWORD_SYNTAX; }
"extensions" { return KEYWORD_EXTENSIONS; }
"rpc"        { return KEYWORD_RPC; }
"returns"    { return KEYWORD_RETURNS; }

"default"    { yylval.flag = 0; return FIELDFLAG; }
"packed"     { yylval.flag = PROTOBUF_C_FIELD_FLAG_PACKED; return FIELDFLAG; }
"deprecated" { yylval.flag = PROTOBUF_C_FIELD_FLAG_DEPRECATED; return FIELDFLAG; } 

"optional"   { yylval.label = PROTOBUF_C_LABEL_OPTIONAL; return FIELDLABEL; }
"required"   { yylval.label= PROTOBUF_C_LABEL_REQUIRED; return FIELDLABEL; }
"repeated"   { yylval.label= PROTOBUF_C_LABEL_REPEATED; return FIELDLABEL; }

"double"     { yylval.type = PROTOBUF_C_TYPE_DOUBLE; return FIELDTYPE; }
"float"      { yylval.type = PROTOBUF_C_TYPE_FLOAT; return FIELDTYPE; }
"int32"      { yylval.type = PROTOBUF_C_TYPE_INT32; return FIELDTYPE; }
"int64"      { yylval.type = PROTOBUF_C_TYPE_INT64; return FIELDTYPE; }
"uint32"     { yylval.type = PROTOBUF_C_TYPE_UINT32; return FIELDTYPE; }
"uint64"     { yylval.type = PROTOBUF_C_TYPE_UINT64; return FIELDTYPE; }
"sint32"     { yylval.type = PROTOBUF_C_TYPE_SINT32; return FIELDTYPE; }
"sint64"     { yylval.type = PROTOBUF_C_TYPE_SINT64; return FIELDTYPE; }
"fixed32"    { yylval.type = PROTOBUF_C_TYPE_FIXED32; return FIELDTYPE; }
"fixed64"    { yylval.type = PROTOBUF_C_TYPE_FIXED64; return FIELDTYPE; }
"sfixed32"   { yylval.type = PROTOBUF_C_TYPE_SFIXED32; return FIELDTYPE; }
"sfixed64"   { yylval.type = PROTOBUF_C_TYPE_SFIXED64; return FIELDTYPE; }
"bool"       { yylval.type = PROTOBUF_C_TYPE_BOOL; return FIELDTYPE; }
"string"     { yylval.type = PROTOBUF_C_TYPE_STRING; return FIELDTYPE; }

[a-zA-Z][a-zA-Z0-9\-_]*     { yylval.s=strdup(yytext); return IDENT; }


[1-9][0-9]*                 { yylval.d=atoi(yytext); return INT_LITERAL; }
0[0-7]*                     { yylval.d=atoi(yytext); return INT_LITERAL; }
0[xX][0-9A-Fa-f]+           { yylval.d=atoi(yytext); return INT_LITERAL; }

"true"                        { yylval.b = 1; return BOOL_LITERAL; }
"false"                       { yylval.b = 0; return BOOL_LITERAL; }

"{" |
"}" |
"=" |
";" |
"[" |
"]" |
"(" |
")" |
\"            { return yytext[0]; }

\/\/.*        { /* ingore comment */ }
[ \t]*        { /* ignore whitespace */ }

.             { printf("bad input character '%s' at line %d\n", yytext, yylineno); }
%%

