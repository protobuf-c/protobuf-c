/* Checks streaming flags in descriptors and streaming dispatch via invoke. */
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "t/streaming/streaming.pb-c.h"

/* A trivial ProtobufCInputStream "derived" type that sums its inputs. */
typedef struct {
	ProtobufCInputStream base;
	int sum;
	int ended;
	int destroyed;
	ProtobufCClosure closure;
	void *closure_data;
} SumStream;

static SumStream the_stream;

/* NULL ends the stream: reply, and release the stream (here: record it). */
static protobuf_c_boolean
sum_send(ProtobufCInputStream *s, const ProtobufCMessage *m)
{
	SumStream *ss = (SumStream *) s;
	if (m == NULL) {
		Streaming__Rsp rsp = STREAMING__RSP__INIT;
		ss->ended = 1;
		rsp.n = ss->sum;
		ss->closure(&rsp.base, ss->closure_data);
		ss->destroyed = 1;
		return 1;
	}
	ss->sum += ((const Streaming__Req *) m)->n;
	return 1;
}

static void
my_unary(Streaming__Counter_Service *s, const Streaming__Req *in,
	 Streaming__Rsp_Closure cl, void *d)
{
	Streaming__Rsp rsp = STREAMING__RSP__INIT;
	(void) s;
	rsp.n = in->n + 1;
	cl(&rsp, d);
}

/* server streaming: n, n-1, ..., 1 then end of stream */
static void
my_down(Streaming__Counter_Service *s, const Streaming__Req *in,
	Streaming__Rsp_Closure cl, void *d)
{
	int i;
	(void) s;
	for (i = in->n; i > 0; i--) {
		Streaming__Rsp rsp = STREAMING__RSP__INIT;
		rsp.n = i;
		cl(&rsp, d);
	}
	cl(NULL, d);
}

/* client streaming: response is sent when the input stream ends */
static ProtobufCInputStream *
my_up(Streaming__Counter_Service *s, const Streaming__Req *in,
      Streaming__Rsp_Closure cl, void *d)
{
	(void) s;
	memset(&the_stream, 0, sizeof the_stream);
	the_stream.base.send = sum_send;
	the_stream.base.input_descriptor = &streaming__req__descriptor;
	the_stream.closure = (ProtobufCClosure) cl;
	the_stream.closure_data = d;
	if (in != NULL)
		the_stream.sum = in->n;
	return &the_stream.base;
}

/* bidi: echoes each input back (the first message is `in`, if given) */
static ProtobufCInputStream *
my_both(Streaming__Counter_Service *s, const Streaming__Req *in,
	Streaming__Rsp_Closure cl, void *d)
{
	return my_up(s, in, cl, d);
}

static int got_n[8];
static int got_count;
static int got_end;

static void
collect(const Streaming__Rsp *r, void *d)
{
	(void) d;
	if (r == NULL)
		got_end++;
	else
		got_n[got_count++] = r->n;
}

int
main(void)
{
	const ProtobufCServiceDescriptor *d = &streaming__counter__descriptor;
	Streaming__Counter_Service svc =
		STREAMING__COUNTER__INIT(my_);
	Streaming__Req req = STREAMING__REQ__INIT;
	Streaming__Req__InputStream *in;

	assert(d->n_methods == 4);
	assert(d->methods[0].flags == 0);
	assert(d->methods[1].flags == PROTOBUF_C_METHOD_OUTPUT_STREAMING);
	assert(d->methods[2].flags == PROTOBUF_C_METHOD_INPUT_STREAMING);
	assert(d->methods[3].flags == (PROTOBUF_C_METHOD_INPUT_STREAMING |
				       PROTOBUF_C_METHOD_OUTPUT_STREAMING));
	assert(protobuf_c_service_descriptor_get_method_by_name(d, "Both") ==
	       &d->methods[3]);

	/* unary */
	req.n = 41;
	streaming__counter__unary(&svc.base, &req, collect, NULL);
	assert(got_count == 1 && got_n[0] == 42 && got_end == 0);

	/* server streaming: many callbacks then NULL */
	got_count = 0;
	req.n = 3;
	streaming__counter__down(&svc.base, &req, collect, NULL);
	assert(got_count == 3 && got_n[0] == 3 && got_n[2] == 1 && got_end == 1);

	/* client streaming, no first message */
	got_count = 0; got_end = 0;
	in = streaming__counter__up(&svc.base, NULL, collect, NULL);
	assert(in != NULL && in->base.send == sum_send);
	req.n = 5;
	assert(streaming__req__input_stream__send(in, &req));	/* typed */
	req.n = 7;
	assert(protobuf_c_input_stream_send(&in->base, &req.base));	/* generic */
	assert(got_count == 0);

	/* The generic send rejects a message of the wrong type (the typed
	 * one cannot be given one). */
	{
		Streaming__Rsp wrong = STREAMING__RSP__INIT;
		assert(!protobuf_c_input_stream_send(&in->base, &wrong.base));
		assert(got_count == 0);
	}

	assert(!the_stream.ended);
	assert(streaming__req__input_stream__send(in, NULL));	/* end of stream */
	assert(got_count == 1 && got_n[0] == 12);
	assert(the_stream.ended && the_stream.destroyed);

	/* client streaming, first message supplied to invoke */
	got_count = 0;
	req.n = 100;
	in = streaming__counter__up(&svc.base, &req, collect, NULL);
	req.n = 1;
	streaming__req__input_stream__send(in, &req);
	streaming__req__input_stream__send(in, NULL);
	assert(got_count == 1 && got_n[0] == 101);

	/* bidi uses the same return convention */
	got_count = 0;
	in = streaming__counter__both(&svc.base, NULL, collect, NULL);
	assert(in != NULL);
	streaming__req__input_stream__send(in, NULL);

	puts("streaming: ok");
	return 0;
}
