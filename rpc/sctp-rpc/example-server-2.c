#include "protobuf-c-sctp.h"

void usage (const char *prog_name)
{
  die ("usage: %s --port=PORT [--name=NAME]\n"
       "Run an example word-funcs service.\n", prog_name);
}

int main (int argc, char **argv)
{
  int port = 0;
  ProtobufC_SCTP_LocalService local_services[1] = { { "word_funcs", NULL } };
  for (i = 1; i < argc; i++)
    {
      if (strncmp (argv[i], "--port=", 7) == 0)
        port = atoi (argv[i] + 7);
      else if (strncmp (argv[i], "--name=", 7) == 0)
        local_services[0].name = argv[i] + 7;
      else
        usage (argv[0]);
    }
  if (port == 0)
    die ("--port=PORT is required");

  /* initialize our our actual service implementation */
  example_word_funcs_service_init ();
  local_services[0].service = example_word_funcs_service;

  /* run the server */
  protobuf_c_sctp_server_run (1, local_services, 0, NULL, port);

  return 0;
}
