#include "example-word-funcs-service.h"

/* note: the implementations here only handle ascii characters--
   this is a tutorial about RPC,
   not utf8 encoding or unicode normalization */

Example__WordFuncs_Service *example_word_funcs_service;

static Example__WordFuncs_Service word_funcs_service;

static void
implement_uppercase (Example__WordFuncs_Service *service,
                     const Example__Word *input,
                     Example__Word_Closure closure,
                     void *closure_data)
{
  char *rv = strdup (input->word);
  Example__Word output = EXAMPLE__WORD__INIT;
  char *at;
  for (at = rv; *at; at++)
    if ('a' <= *at && *at <= 'z')
      *at -= ('a' - 'A');
  output.word = rv;
  closure (&output, closure_data);
  free (rv);
}

static void
implement_lowercase (Example__WordFuncs_Service *service,
                     const Example__Word *input,
                     Example__Word_Closure closure,
                     void *closure_data)
{
  char *rv = strdup (input->word);
  Example__Word output = EXAMPLE__WORD__INIT;
  char *at;
  for (at = rv; *at; at++)
    if ('A' <= *at && *at <= 'Z')
      *at += ('a' - 'A');
  output.word = rv;
  closure (&output, closure_data);
  free (rv);
}

#if 0           /* you could consider an init function like this instead */
void example_word_funcs_service_init (void)
{
  example_word_funcs_service = &word_funcs_service;
  example__word_funcs__init (&word_funcs_service);
  word_funcs_service.uppercase = implement_uppercase;
  word_funcs_service.lowercase = implement_lowercase;
}
#endif

/* TODO: provide a c99 example */
Example__WordFuncs_Service example__word_funcs__service__global =
{
  EXAMPLE__WORD_FUNCS__SERVICE__BASE_INIT,
  implement_uppercase,
  implement_lowercase
};
