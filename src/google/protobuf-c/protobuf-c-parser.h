
typedef struct _ProtobufC_ParseResult ProtobufC_ParseResult;
struct _ProtobufC_ParseResult
{
  const char *message;
  const char *filename;
  unsigned line_no;
};

struct _ProtobufC_ParserPackage
{
  char *name;
  unsigned n_messages;
  ProtobufCMessageDescriptor *message_descriptors;
  unsigned n_enums;
  ProtobufCEnumDescriptor *enum_descriptors;
};

ProtobufC_Parser *protobuf_c_parser_new (void);

/* If non-NULL, return-value should be freed with default allocator */
typedef char *(*ProtobufC_ImportHandlerFunc) (const char *package_name,
                                              void       *func_data);

void protobuf_c_parser_set_import_handler (ProtobufC_Parser *parser,
                                           ProtobufC_ImportHandlerFunc func,
                                           void *func_data);

ProtobufC_ParserPackage *
               protobuf_c_parser_parse_file(ProtobufC_Parser *parser,
                                            const char       *filename,
                                            ProtobufC_ParseResult *error);
ProtobufC_ParserPackage *
               protobuf_c_parser_import    (ProtobufC_Parser *parser,
                                            const char       *package_name,
                                            ProtobufC_ParseResult *error);

void           protobuf_c_parser_free      (ProtobufC_Parser *parser);


typedef enum
{
  PROTOBUF_C_CODEGEN_MODE_H,
  PROTOBUF_C_CODEGEN_MODE_C
} ProtobufC_CodegenMode;

struct _ProtobufC_CodegenInfo
{
  const char *package_name;
  ProtobufC_CodegenMode mode;
  const char *output_filename;
};
#define PROTOBUF_C_CODEGEN_INFO_INIT { NULL, PROTOBUF_C_CODEGEN_MODE_H, NULL }

protobuf_c_boolean protobuf_c_parser_codegen(ProtobufC_Parser *parser,
                                             ProtobufC_CodegenInfo *codegen);
