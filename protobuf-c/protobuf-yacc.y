%{
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
%}

%union
{
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
}

%token KEYWORD_MESSAGE 
%token KEYWORD_PACKAGE 
%token KEYWORD_IMPORT 
%token KEYWORD_SERVICE 
%token KEYWORD_SYNTAX 
%token KEYWORD_OPTION 
%token KEYWORD_EXTENSIONS 
%token KEYWORD_ENUM 
%token KEYWORD_ONEOF 
%token KEYWORD_RPC
%token KEYWORD_RETURNS
%token <label> FIELDLABEL
%token <type> FIELDTYPE
%token <flag> FIELDFLAG
%token <d> INT_LITERAL 
%token <s> IDENT
%token <b> BOOL_LITERAL
%type <s> FULL_IDENT
%type <s> package_name
%type <ev> enum_value_entry
%type <ed> enum_definition
%type <md> message_definition
%type <md> message_definition_start
%type <fd> field_definition
%type <type> field_type

%%

proto_file_entry : /* nothing */
                | package_declaration proto_file_entry
                | syntax_declaration proto_file_entry
                | message_definition proto_file_entry
                | service_definition proto_file_entry
                | enum_definition proto_file_entry
                ;

package_declaration: KEYWORD_PACKAGE package_name ';' 
                    { 
                        file_definition.package_name = $2;
                    }
                    ;

package_name: FULL_IDENT

FULL_IDENT: IDENT 
            {
                int len = strlen($1);
                char* var = _allocator->alloc(_allocator->allocator_data, (len+1)*sizeof(char));
                if(!var) YYABORT;
                strncpy(var,$1, len);
                var[len]='\0';
            }
          | IDENT "." FULL_IDENT
            {
                int len = strlen($1)+strlen($3)+1;
                char* var = _allocator->alloc(_allocator->allocator_data, (len+1)*sizeof(char));                
                if(!var) YYABORT;
                sprintf(var,"%s.%s",$1,$3);                
                var[len]='\0';
                _allocator->free(_allocator->allocator_data,$3);                                        
            }


syntax_declaration: KEYWORD_SYNTAX '=' '"' IDENT '"' ';' 
                    { 
                        printf("syntax %s\n",$4);
                    }
                    ;

message_definition: message_definition_start '{' message_body '}' optional_semicolon
                          { 
                              $$=$1;
                              if(!list_add(&file_definition.message_definition_list, $$)) YYABORT;
                              list_remove(&message_definition_tree);
                          }                          
                          ;

optional_semicolon: /* nothing */
                  | ';'

message_definition_start: KEYWORD_MESSAGE IDENT 
                          { 
                              MessageDefinition* m = _allocator->alloc(_allocator->allocator_data, sizeof(MessageDefinition));                              
                              if(!m) YYABORT;
                              m->short_name = $2;
                              m->name = make_full_name($2);
                              if(!m->name) YYABORT;
                              $$ = m;
                              list_add(&message_definition_tree, m);
                          }
                          ;

message_body: /* nothing */
            | field_definition message_body  
                { 
                    MessageDefinition* m = (MessageDefinition*)message_definition_tree.last->data;
                    if(!list_add(&m->field_definition_list, $1)) YYABORT;
                }
            | enum_definition message_body
            | message_definition message_body 
            ;

field_definition: FIELDLABEL field_type IDENT '=' INT_LITERAL field_options ';' 
                  { 
                      ProtobufCFieldDescriptor* desc = _allocator->alloc(_allocator->allocator_data, sizeof(ProtobufCFieldDescriptor));
                      if(!desc) YYABORT;
                      desc->name=$3;
                      desc->id=$5;
                      desc->label=$1;
                      desc->type = $2;
                      desc->descriptor = tmp_descriptor;
                      $$=desc;
                  }
                  ; 

field_type:  FIELDTYPE { tmp_descriptor = NULL; $$ = $1; }
          |  IDENT  
            { 
                int is_enum = 0;
                tmp_descriptor = lookup_type($1, &is_enum);
                if(!tmp_descriptor) YYABORT;
                $$ = is_enum ? PROTOBUF_C_TYPE_ENUM : PROTOBUF_C_TYPE_MESSAGE;
            }

field_options: /* nothing */
              | '[' FIELDFLAG '=' IDENT ']'

enum_definition: KEYWORD_ENUM IDENT '{' enum_values '}'
                  {
                      EnumDefinition* def = _allocator->alloc(_allocator->allocator_data, sizeof(EnumDefinition));
                      if(!def) YYABORT;
                      def->short_name=$2;
                      def->name = make_full_name($2);
                      if(!def->name) YYABORT;
                      def->enum_values_list = enum_values_list;
                      enum_values_list.first = enum_values_list.last = 0;
                      enum_values_list.length = 0;
                      $$=def;
                      if(!list_add(&file_definition.enum_definition_list, def)) YYABORT;
                  }
                  ;

enum_values: /* nothing */
            | enum_values enum_value_entry 
                { 
                    if(!list_add(&enum_values_list, $2)) YYABORT;
                }
            ;

enum_value_entry: IDENT '=' INT_LITERAL ';' 
            {
                ProtobufCEnumValue* value = _allocator->alloc(_allocator->allocator_data, sizeof(ProtobufCEnumValue));
                if(!value) YYABORT;
                value->name = $1;
                value->c_name = $1;
                value->value = $3;
                $$ = value;
            }
            ;

service_definition: KEYWORD_SERVICE IDENT '{' KEYWORD_RPC IDENT '(' IDENT ')' KEYWORD_RETURNS '(' IDENT ')' ';' '}'
                    {
                        printf("got service %s\n", $2);
                    }
                    ;

%%

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
