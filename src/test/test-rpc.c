#include <assert.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include "generated-code/test.pb-c.h"
#include <google/protobuf-c/protobuf-c-rpc.h>

static void
message (const char *format, ...)
{
  va_list args;
  va_start (args, format);
  vfprintf (stderr, format, args);
  va_end (args);
  fputc ('\n', stderr);
}

/* --- A local service --- */
static void
test__by_name (Foo__DirLookup_Service *service,
               const Foo__Name *name,
               Foo__LookupResult_Closure closure,
               void *closure_data)
{
  Foo__LookupResult result = FOO__LOOKUP_RESULT__INIT;
  Foo__Person__PhoneNumber pn = FOO__PERSON__PHONE_NUMBER__INIT;
  Foo__Person__PhoneNumber *pns[] = { &pn };
  Foo__Person person = FOO__PERSON__INIT;
  char *number = NULL;
  Foo__Person__PhoneType type = FOO__PERSON__PHONE_TYPE__MOBILE;
  char *email = NULL;
  (void) service;
  if (name->name == NULL)
    closure (NULL, closure_data);
  else if (strcmp (name->name, "dave") == 0)
    {
      number = "555-1212";
      type = FOO__PERSON__PHONE_TYPE__MOBILE;
    }
  else if (strcmp (name->name, "joe the plumber") == 0)
    {
      number = "976-PIPE";
      type = FOO__PERSON__PHONE_TYPE__WORK;
      email = "joe@the-plumber.com";
    }

  if (number != NULL)
    {
      pn.number = number;
      pn.has_type = 1;
      pn.type = type;
      person.n_phone = 1;
      person.phone = pns;
      person.email = email;
      person.name = name->name;
      result.person = &person;
    }
  closure (&result, closure_data);
}

static Foo__DirLookup_Service the_dir_lookup_service =
  FOO__DIR_LOOKUP__INIT(test__);

static void
test_dave_closure (const Foo__LookupResult *result,
                   void *closure_data)
{
  assert (result);
  assert (result->person != NULL);
  assert (result->person->name != NULL);
  assert (strcmp (result->person->name, "dave") == 0);
  assert (result->person->n_phone == 1);
  assert (strcmp (result->person->phone[0]->number, "555-1212") == 0);
  assert (result->person->phone[0]->type == FOO__PERSON__PHONE_TYPE__MOBILE);
  * (protobuf_c_boolean *) closure_data = 1;
}
static void
test_joe_the_plumber_closure (const Foo__LookupResult *result,
                              void *closure_data)
{
  assert (result);
  assert (result->person != NULL);
  assert (result->person->name != NULL);
  assert (strcmp (result->person->name, "joe the plumber") == 0);
  assert (result->person->n_phone == 1);
  assert (strcmp (result->person->phone[0]->number, "976-PIPE") == 0);
  assert (result->person->phone[0]->type == FOO__PERSON__PHONE_TYPE__WORK);
  * (protobuf_c_boolean *) closure_data = 1;
}
static void
test_not_found_closure (const Foo__LookupResult *result,
                        void *closure_data)
{
  assert (result);
  assert (result->person == NULL);
  * (protobuf_c_boolean *) closure_data = 1;
}

static void
test_defunct_closure (const Foo__LookupResult *result,
                        void *closure_data)
{
  assert (result == NULL);
  * (protobuf_c_boolean *) closure_data = 1;
}


static void
test_service (ProtobufCService *service)
{
  Foo__Name name = FOO__NAME__INIT;
  protobuf_c_boolean is_done;

  name.name = "dave";
  is_done = 0;
  foo__dir_lookup__by_name (service, &name, test_dave_closure, &is_done);
  while (!is_done)
    protobuf_c_dispatch_run (protobuf_c_dispatch_default ());

  name.name = "joe the plumber";
  is_done = 0;
  foo__dir_lookup__by_name (service, &name, test_joe_the_plumber_closure, &is_done);
  while (!is_done)
    protobuf_c_dispatch_run (protobuf_c_dispatch_default ());

  name.name = "asdfvcvzxsa";
  is_done = 0;
  foo__dir_lookup__by_name (service, &name, test_not_found_closure, &is_done);
  while (!is_done)
    protobuf_c_dispatch_run (protobuf_c_dispatch_default ());
}
static void
test_defunct_client (ProtobufCService *service)
{
  Foo__Name name = FOO__NAME__INIT;
  protobuf_c_boolean is_done;

  name.name = "dave";
  is_done = 0;
  foo__dir_lookup__by_name (service, &name, test_defunct_closure, &is_done);
  while (!is_done)
    protobuf_c_dispatch_run (protobuf_c_dispatch_default ());

  name.name = "joe the plumber";
  is_done = 0;
  foo__dir_lookup__by_name (service, &name, test_defunct_closure, &is_done);
  while (!is_done)
    protobuf_c_dispatch_run (protobuf_c_dispatch_default ());

  name.name = "asdfvcvzxsa";
  is_done = 0;
  foo__dir_lookup__by_name (service, &name, test_defunct_closure, &is_done);
  while (!is_done)
    protobuf_c_dispatch_run (protobuf_c_dispatch_default ());
}

/* --- main() --- */
static void
set_boolean_true (ProtobufCDispatch *dispatch,
                  void              *func_data)
{
  (void) dispatch;
  * (protobuf_c_boolean *) func_data = 1;
}

int main()
{
  protobuf_c_boolean is_done;
  ProtobufCService *local_service = (ProtobufCService *) &the_dir_lookup_service;
  ProtobufCService *remote_service;
  ProtobufC_RPC_Client *client;
  ProtobufC_RPC_Server *server;

  message ("testing local service");
  test_service (local_service);

  message ("creating client");
  /* Create a client with no server.  Verify that
     the client returns a failure immediately */
  remote_service = protobuf_c_rpc_client_new (PROTOBUF_C_RPC_ADDRESS_LOCAL,
                                       "test.socket",
                                       &foo__dir_lookup__descriptor,
                                       NULL);
  assert (remote_service != NULL);
  client = (ProtobufC_RPC_Client *) remote_service;
  protobuf_c_rpc_client_set_autoreconnect_period (client, 10);
  is_done = 0;
  protobuf_c_dispatch_add_timer_millis (protobuf_c_dispatch_default (),
                                        250, set_boolean_true, &is_done);
  message ("verify client cannot connect");
  while (!is_done)
    {
      protobuf_c_dispatch_run (protobuf_c_dispatch_default ());
      assert (!protobuf_c_rpc_client_is_connected (client));
    }
  message ("testing unconnected client");
  test_defunct_client (remote_service);

  message ("creating server");
  /* Create a server and wait for the client to connect. */
  server = protobuf_c_rpc_server_new (PROTOBUF_C_RPC_ADDRESS_LOCAL,
                                      "test.socket",
                                      local_service,
                                      NULL);
  assert (server != NULL);
  message ("waiting to connect");
  is_done = 0;

  /* technically, there's no way to know how long it'll take to connect
     if the machine is heavily loaded.  We give 250 millis,
     which should be ample on an unloaded system. */
  protobuf_c_dispatch_add_timer_millis (protobuf_c_dispatch_default (),
                                        250, set_boolean_true, &is_done);
  while (!is_done && !protobuf_c_rpc_client_is_connected (client))
    protobuf_c_dispatch_run (protobuf_c_dispatch_default ());
  if (!protobuf_c_rpc_client_is_connected (client))
    assert(0);          /* operation timed-out; could be high machine load */

  /* wait for the timer to elapse, since that's laziest way to handle it. */
  while (!is_done)
    protobuf_c_dispatch_run (protobuf_c_dispatch_default ());

  /* Test the client */
  message ("testing client");
  test_service (remote_service);

  /* Destroy the server and ensure that a request is failed in
     a timely fashion. */
  message ("destroying server");
  protobuf_c_rpc_server_destroy (server, 0);
  server = NULL;
  message ("test client has no data");
  test_defunct_client (remote_service);

  /* Create a server again and wait for the client to reconnect. */
  message ("creating server again");
  server = protobuf_c_rpc_server_new (PROTOBUF_C_RPC_ADDRESS_LOCAL,
                                      "test.socket",
                                      local_service,
                                      NULL);
  assert (server != NULL);
  is_done = 0;
  protobuf_c_dispatch_add_timer_millis (protobuf_c_dispatch_default (),
                                        250, set_boolean_true, &is_done);
  while (!is_done)
    protobuf_c_dispatch_run (protobuf_c_dispatch_default ());
  assert (protobuf_c_rpc_client_is_connected (client));

  /* Test the client again, for kicks. */
  message ("testing client again");
  test_service (remote_service);

  /* Destroy the client */
  message ("destroying client");
  protobuf_c_service_destroy (remote_service);

  /* Destroy the server */
  message ("destroying server");
  protobuf_c_rpc_server_destroy (server, 0);

  protobuf_c_dispatch_destroy_default ();

  return 0;
}

