#include <string.h>
#include <stdio.h>
#include "protobuf-c.h"
#include "../test.pb-c.h"

int find_message_descriptor(const ProtobufCFileDescriptor*, const ProtobufCMessageDescriptor*);
int compare_message_descriptor(const ProtobufCMessageDescriptor*, const ProtobufCMessageDescriptor*);
int find_enum_descriptor(const ProtobufCFileDescriptor*, const ProtobufCEnumDescriptor*);
int compare_enum_descriptor(const ProtobufCEnumDescriptor*, const ProtobufCEnumDescriptor*);
int compare_field_descriptor(const ProtobufCFieldDescriptor*, const ProtobufCFieldDescriptor*);
int compare_enum_value(const ProtobufCEnumValue*, const ProtobufCEnumValue*);
int compare_enum_index(const ProtobufCEnumValueIndex*, const ProtobufCEnumValueIndex*);

int compare_field_descriptor(const ProtobufCFieldDescriptor* parsed, const ProtobufCFieldDescriptor* generated)
{
    int retval = 1;
    do
    {
        if( strcmp(parsed->name, generated->name) != 0 )
        {
            printf("field name doesnt match parser->%s generated->%s\n", parsed->name, generated->name);
            break;
        }

        if( parsed->id != generated->id )
        {
            printf("field id doesnt match parser->%d generated->%d\n", parsed->id, generated->id);
            break;
        }

        if( parsed->label != generated->label )
        {
            printf("field label doesnt match parser->%d generated->%d\n", parsed->label, generated->label);
            break;
        }

        if( parsed->type != generated->type )
        {
            printf("field type doesnt match parser->%d generated->%d\n", parsed->type, generated->type);
            break;
        }

        retval = 0;
    }while(0);
    return retval;
}

int compare_enum_value(const ProtobufCEnumValue* parsed, const ProtobufCEnumValue* generated)
{
    int retval = 1;

    do
    {
        if( strcmp(parsed->name, generated->name) != 0 )
        {
            printf("enum name doesnt match parser->%s generated->%s\n", parsed->name, generated->name);
            break;
        }
        if( parsed->value != generated->value )
        {
            printf("enum value doesnt match parser->%d generated->%d\n", 
                    parsed->value, generated->value );
            break;
        }

        retval = 0;
    }while(0);

    return retval;
}

int compare_enum_index(const ProtobufCEnumValueIndex* parsed, const ProtobufCEnumValueIndex* generated)
{
    int retval = 1;

    do
    {
        if( strcmp(parsed->name, generated->name) != 0 )
        {
            printf("index name doesnt match parser->%s generated->%s\n", parsed->name, generated->name);
            break;
        }
        if( parsed->index != generated->index )
        {
            printf("enum value index doesnt match parser->%d generated->%d\n", 
                    parsed->index, generated->index );
            break;
        }

        retval = 0;
        break;
    }while(0);

    return retval;
}

int compare_message_descriptor(const ProtobufCMessageDescriptor* parsed, const ProtobufCMessageDescriptor* generated)
{
    int retval = 1;
    do
    {
        if( strcmp(parsed->name, generated->name) != 0 )
        {
            printf("message name doesnt match parser->%s generated->%s\n", parsed->name, generated->name);
            break;
        }
        if( strcmp(parsed->short_name, generated->short_name) != 0 )
        {
            printf("message short_name doesnt match parser->%s generated->%s\n", 
                    parsed->short_name, generated->short_name );
            break;
        }

        if( strcmp(parsed->package_name, generated->package_name) != 0 )
        {
            printf("message package_name doesnt match parser->%s generated->%s\n", 
                    parsed->package_name, generated->package_name );
            break;
        }

        if( parsed->n_fields != generated->n_fields )
        {
            printf("# of fields differ parsed-%d generated-%d\n", parsed->n_fields, generated->n_fields);
            break;
        }

        for( unsigned i = 0; i < parsed->n_fields; i++)
        {
            if( compare_field_descriptor(&parsed->fields[i], &generated->fields[i]) != 0 )
            {
                retval = -1;
                break;
            }
        }

        if( retval == -1 )
        {
            retval = 1;
            break;
        }

        for( unsigned i = 0; i < parsed->n_fields; i++)
        {
            if( parsed->fields_sorted_by_name[i] != generated->fields_sorted_by_name[i] ) 
            {
                printf("field index doesnt match parsed-%d generated-%d\n", 
                            parsed->fields_sorted_by_name[i],
                            generated->fields_sorted_by_name[i] );
                retval = -1;
                break;
            }
        }

        if( retval == -1 )
        {
            retval = 1;
            break;
        }

        retval = 0;
    }while(0);
    return retval;
}

int compare_enum_descriptor(const ProtobufCEnumDescriptor* parsed, const ProtobufCEnumDescriptor* generated)
{
    int retval = 1;
    do
    {
        if( strcmp(parsed->name, generated->name) != 0 )
        {
            printf("enum name doesnt match parser->%s generated->%s\n", parsed->name, generated->name);
            break;
        }
        if( strcmp(parsed->short_name, generated->short_name) != 0 )
        {
            printf("enum short_name doesnt match parser->%s generated->%s\n", 
                    parsed->short_name, generated->short_name );
            break;
        }

        if( strcmp(parsed->package_name, generated->package_name) != 0 )
        {
            printf("enum package_name doesnt match parser->%s generated->%s\n", 
                    parsed->package_name, generated->package_name );
            break;
        }

        if( parsed->n_values != generated->n_values )
        {
            printf("# of values differ parsed-%d generated-%d\n", parsed->n_values, generated->n_values);
            break;
        }

        for( unsigned i = 0; i < parsed->n_values; i++)
        {
            if( compare_enum_value(&parsed->values[i], &generated->values[i]) != 0 )
            {
                retval = -1;
                break;
            }
        }

        if( retval == -1 )
        {
            retval = 1;
            break;
        }

        if( parsed->n_value_names != generated->n_value_names )
        {
            printf("# of value names differ parsed-%d generated-%d\n", 
                    parsed->n_value_names, generated->n_value_names);
            break;
        }

        for( unsigned i = 0; i < parsed->n_value_names; i++)
        {
            if( compare_enum_index(&parsed->values_by_name[i], &generated->values_by_name[i]) != 0 )
            {
                retval = -1;
                break;
            }
        }

        if( retval == -1 )
        {
            retval = 1;
            break;
        }

        retval = 0;
    }while(0);
    return retval;
}

int find_message_descriptor(const ProtobufCFileDescriptor* desc, const ProtobufCMessageDescriptor* mdesc)
{   
    int retval = 1;
    for( unsigned i = 0; i < desc->n_messages; i++)
    {
        if( strcmp(desc->messages[i].name, mdesc->name) == 0 )
        {
            printf("Found message %s\n", mdesc->name);            
            if( compare_message_descriptor(&desc->messages[i], mdesc) == 0 )
            {
                retval = 0;
            }
            break;
        }
    }
    return retval;
}

int find_enum_descriptor(const ProtobufCFileDescriptor* desc, const ProtobufCEnumDescriptor* edesc)
{   
    int retval = 1;
    for( unsigned i = 0; i < desc->n_enums; i++)
    {
        if( strcmp(desc->enums[i].name, edesc->name) == 0 )
        {
            printf("Found enum %s\n", edesc->name);            
            retval = compare_enum_descriptor(&desc->enums[i], edesc);
            break;
        }
    }
    return retval;
}

int main(int argc, char* argv[])
{   
    int retval = 1;
    const ProtobufCFileDescriptor* desc = protobuf_c_proto_load_file("t/test-parser.proto", NULL);
    const ProtobufCMessageDescriptor* mdesc[] = 
    {
        &foo__person__descriptor,
        &foo__person__phone_number__descriptor,
        &foo__person__phone_number__comment__descriptor,
        &foo__lookup_result__descriptor,
        &foo__name__descriptor
    };
    const ProtobufCEnumDescriptor* edesc[] = 
    {
        &foo__person__phone_type__descriptor
    };

    do
    {
        for( unsigned i = 0; i < sizeof(mdesc)/sizeof(mdesc[0]); i++ )
        {
            if(find_message_descriptor(desc, mdesc[i]) != 0 )
            {    
                retval = -1;
                break;
            }
        }
        if( retval == -1 )
        {
            break;
        }

        for( unsigned i = 0; i < sizeof(edesc)/sizeof(edesc[0]); i++ )
        {
            if(find_enum_descriptor(desc, edesc[i]) != 0)
            {    
                retval = -1;
                break;
            }
        }
        if( retval == -1 )
        {
            break;
        }

        retval = 0;
    }while(0);
   return retval;
}
