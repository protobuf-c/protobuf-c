
typedef struct _ProtobufC_ParseResult ProtobufC_ParseResult;
struct _ProtobufC_ParseResult
{
  ...
};

struct _ProtobufC_ParserPackage
{
  char *name;
  unsigned n_messages;
  ProtobufCMessageDescriptor *message_descriptors;
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

