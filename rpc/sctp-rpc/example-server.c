#include "protobuf-c-sctp.h"

typedef struct _StringFuncsImpl StringFuncsImpl;
struct _StringFuncsImpl
{
  Example__WordFuncs_Service base_service;
};

void usage (const char *prog_name)
{
  die ("usage: %s --port=PORT [--name=NAME]\n"
       "Run an example word-funcs service.\n", prog_name);
}

typedef struct _ChannelConfigData ChannelConfigData;
struct _ChannelConfigData
{
  const char *name;
  ProtobufCService *service;
};

/* Set up newly-accepted connection */
static void
init_channel (ProtobufC_SCTP_Channel *channel,
              void                   *data)
{
  ChannelConfigData *config = data;
  protobuf_c_sctp_channel_add_local_service (channel,
                                             config->name,
                                             config->service);
}

int main (int argc, char **argv)
{
  int port = 0;
  StringFuncsImpl impl;
  ChannelConfigData config = { "word_funcs", &impl.base_service.base };
  for (i = 1; i < argc; i++)
    {
      if (strncmp (argv[i], "--port=", 7) == 0)
        port = atoi (argv[i] + 7);
      else if (strncmp (argv[i], "--name=", 7) == 0)
        config.name = argv[i] + 7;
      else
        usage (argv[0]);
    }
  if (port == 0)
    die ("--port=PORT is required");

  /* initialize our our actual service implementation */
  example__word_funcs__init (&impl.base_service);
  impl.base_service.uppercase = implement_uppercase;

  listener = protobuf_c_sctp_listener_new_ipv4 (NULL, port, dispatch,
                                                init_channel, &config);
  for (;;)
    protobuf_c_sctp_dispatch_run (dispatch, -1);

  return 0;
}
