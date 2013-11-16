#include "example-word-funcs-service.h"

static void
word_closure_to_ptr_word (const Example__Word *message,
                          void *closure_data)
{
  const char *rv = message == NULL || message->word == NULL
                 ? "*error*" : message->word;
  * (char **) closure_data = strdup (rv);
}

int main (int argc, char **argv)
{
  Example__WordFuncs_Service *service = NULL;
  const char *name = "word_funcs";
  const char *addr = NULL;
   
  dispatch = protobuf_c_sctp_dispatch_new ();

  /* Create service (either local or remote) */
  for (i = 1; i < argc; i++)
    {
      if (strcmp (argv[i], "--local") == 0)
        {
          example_word_funcs_service_init ();
	  service = example_word_funcs_service;
          name = "<local word_funcs>";
	}
      else if (strncmp (argv[i], "--addr=", 7) == 0)
        addr = argv[i] + 7;
      else if (strncmp (argv[i], "--name=", 7) == 0)
        name = argv[i] + 7;
      else
        usage (argv[0]);
    }
  if (service == NULL && addr == NULL)
    die ("missing --addr=");
  if (service == NULL && addr != NULL)
    {
      channel = protobuf_c_sctp_channel_ipv4_connect_by_hostport (addr, dispatch);
      service = protobuf_c_sctp_channel_add_remote_service (channel,
                                                            name,
                                          &example__word_funcs__descriptor);
    }

  /* main loop: invoke service methods (either local or remote) */
  for (;;)
    {
      fprintf (stderr, "%s >> ", name);
      if (fgets (buf, sizeof (buf), stdin) == NULL)
        break;
      /* parse command */
      at = buf;
      while (*at && isspace (*at))
        at++;
      if (*at == 0)
        continue;
      cmd = at;
      while (*at && !isspace (*at))
        at++;
      if (*at != 0)
	{
	  *at++ = 0;
	  while (*at && *isspace (*at))
	    at++;
        }

      if (strcmp (cmd, "uppercase") == 0
       || strcmp (cmd, "lowercase") == 0)
        {
          /* functions mapping string => string can be handled via this
             mechanism */
          char *rv = NULL;
          Example__Word input = EXAMPLE__WORD__INIT;
          input.word = at;
          if (strcmp (cmd, "uppercase") == 0)
            example__word_funcs__uppercase (service,
                                            &input,
                                            word_closure_to_ptr_word,
                                            &rv);
          else if (strcmp (cmd, "lowercase") == 0)
            example__word_funcs__lowercase (service,
                                            &input,
                                            word_closure_to_ptr_word,
                                            &rv);
          else
            assert (0);
	  while (rv == NULL)
            protobuf_c_sctp_dispatch_run (dispatch);

          printf ("%s\n", rv);
          fflush (stdout);
          free (rv);
        }
    }

  return 0;
}
