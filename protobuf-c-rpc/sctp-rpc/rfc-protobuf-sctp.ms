.pl 10.0i
.po 0
.ll 7.2i
.lt 7.2i
.nr LL 7.2i
.nr LT 7.2i
.ds LF Benson
.ds RF FORMFEED[Page %]
.ds CF
.ds LH PROPOSAL
.ds RH 7 December 2008
.ds CH Protobuf over SCTP
.hy 0
.ad l
.in 0
.ce
Proposal for Implementing Protocol-Buffer RPC over SCTP

.ti 0
Status of this Memo

.fi
.in 3
This memo describes a proposed method for the encapsulation of
Protocol-Buffer Datagrams via the Stream Control Transmission Protocol.
This is an experimental, not recommended standard.
Distribution of this memo is unlimited.

.ti 0
Overview and Rationale

Google's Protobuf Buffer package (or "protobuf", for short)
provides a language for
specifying the format binary-data.  In that language, a "message"
in that language defines a specific binary-data format.
The language also defines a "service":  a service has
an array of named "methods", each of which takes a specific
type of message as input, and gives a specific 
type of message as output.

SCTP is a reliable datagram-oriented protocol,
parallel to UDP or TCP, and riding atop the IP layer
(either IPv4 or IPv6).

Protocol-Buffer RPC over SCTP (or protobuf-sctp for short),
defines a way of implementing RPCs to one of a set of named services.

The general inplementation of the server and client
is very similar, since both the server and client can
implement local services and invoke remote services.
Usually, a server provides a local service and the client 
accesses it, but we provide a more symmetric model.

.ti 0
Why Services and not Messages?

One possible alternate scheme for using protobuf is to encapsulate messages
without any regard to the request/response cycle.
This could be quite beneficial at times, especially when the RPC does not
need to get a response message.  Despite the possibility of eliminating
many round-trips, we do not offer any protocol but the rpc-method technique.

Services are the fundamental unit of RPC in protocol buffers,
so it makes sense to use them as the basis of a protocol.
It is possible that a string name is not the most efficient way
to identify the service, but many languages are well-optimized 
for string handling, so it's unclear if base-128 encoding is really
better.  In any event, strings are sent on every request for both
the name of the service, and the name of the method.

.ti 0
Why SCTP and not a generalized reliable datagram layer?

In principle, this specification could be generalized
to cover any reliable datagram passing mechanism.
We chose to limit our attention to SCTP for
the following reasons:
.IP
.RS
.IP *
We can analyze the performance of our design choices.
.RE
.RS
.IP *
SCTP boasts a rich set of features, like the ability to 
specify ordered or unordered delivery.
.RE

.ti 0
Definitions

.KS
.IP
Datagram
.RS
.IP
a piece of binary-data, an array of bytes.
.RE
.KE

.KS
.IP
SCTP Connection
.RS
.IP
a single stream of datagrams.
.RE
.KE

.KS
.IP
Handshake
.RS
.IP
the sequence of datagrams to establish client/server validity.
.RE
.KE

.KS
.IP
Uninitialized Connection
.RS
.IP
an SCTP connection before the handshake has completed.
.RE
.KE

.KS
.IP
Request ID
.RS
.IP
a 64-bit number allocated by the end that begins an RPC request.
It is recommend that the allocation strategy simply use an incrementing 64-bit
counter.
.RE
.KE

.KS
.IP
Datagram Type
.RS
.IP
is a single byte that is the first byte in every message.
It can be used to distinguish the type of message.
.RE
.KE

.ti 0
Overview of the Protocol

The session always begins with ordered delivery of three
messages, the handshake.  Only after the handshake is completed
may unordered messages be sent.

For each remote-procedure call, the caller must allocate a request_id.
The request_id is necessary because we want
to support unordered delivery and because we would like to support
concurrent backend service invocations.

.ti 0
Initial handshake

The first packet is sent by the client,
the active end of the connection.

.KS
.TS
tab(:);
l s
| c | c |
| l |  l | .
HANDSHAKE_REQUEST
=
format:name
_
byte:datagram_type (HANDSHAKE_REQUEST=1)
HandshakeRequest:request
_
.TE
.KE

The HandshakeRequest message format is defined
by the following protobuf file fragment:
.DS L
  message HandshakeService
  {
    required string name = 1;
    optional string service_type_name = 2;
  }
  message HandshakeRequest
  {
    // lists the services provided by the client to the server
    repeated HandshakeService services = 1;
  }
.DE

Once the request has been received,
the server should respond with a message
.KS
.TS
tab(:);
l s
| c | c | 
| l |  l | .
HANDSHAKE_RESPONSE
=
format:name
_
byte:datagram_type (HANDSHAKE_RESPONSE=2)
HandshakeResponse:request
_
.TE
.KE
where HandshakeResponse is defined by the
following fragment, which uses the definition of
HandshakeService above:
.DS L
  message HandshakeResponse
  {
    // lists the services provided by the client to the server
    repeated HandshakeService services = 1;
  }
.DE
This message gives a list of services provided by the
server (which can be called by the client).

Once the response has been received,
the client should respond with the final handshake message:
.KS
.TS
tab(:);
l s
| c | c |
| l |  l | .
HANDSHAKE_COMPLETED
=
format:name
_
byte:datagram_type (HANDSHAKE_COMPLETED=3)
_
.TE
.KE

As far as the client is concerned, the handshake is completed
when it received the HandshakeRespond message;
for the server, the handshake is completed slightly later:
once the HandshakeCompleted message is received.

For pipelineing purposes, it is nice to be able to send
requests before completing the handshake.
But until the handshake is complete, all packets
must be sent in ordered fashion, to ensure that the handshake
is the first packet processed.  (This is the point of the HandshakeCompleted
message: to ensure that all sends will be ordered until
the client has received the respond.)


.ti 0
The RPC Protocol

The format of a remote-procedure call (RPC) transaction
is that a request is sent from the caller to
the caller.  Eventually, the called resource
trasmits a response packet.  The protocol specified here
permits responses to be received out of order.

Out-of-order receipt is implemented by requiring the caller
(or, more precisely, its RPC implementation)
to allocate a request_id for each request.  
The caller which has multiple outstanding requests must then
match the request id to the appropriate response.
The caller may throw an error if a response with an invalid request_id
is encountered.

The remaining sections will describe the wire-format of the messages
sent for a request/response pair.

.ti 0
Request Format

.KS
.TS
tab(:);
l s
| c | c |
| l |  l | .
REQUEST
=
format:name
_
byte:datagram_type (REQUEST=4)
uint64:request_id
NUL-terminated string:service_name
NUL-terminated string:method_name
protobuf:request
_
.TE
.KE

The format of "request" is specified by the service-definition.

.ti 0
Response Format

.KS
.TS
tab(:);
l s
| c | c |
| l |  l | .
RESPONSE
=
format:name
_
byte:datagram_type (RESPONSE=5)
uint64:request_id
protobuf:response
_
.TE
.KE


.ti 0
Discussion

Multiple types of service can be provided with a prioritized pecking
order.  An additional property is built-in worm detection and
eradication.  Because IP only guarantees best effort delivery, loss of
a carrier can be tolerated.  With time, the carriers are
self-regenerating.  While broadcasting is not specified, storms can
cause data loss.  There is persistent delivery retry, until the
carrier drops.  Audit trails are automatically generated, and can
often be found on logs and cable trays.

.ti 0
Security Considerations

.in 3
Messages are sent unencrypted, so this encapsulation cannot
be used safely on the broader internet.

.KS
.ti 0
Author's Address

.nf
David Benson

EMail: daveb@ffem.org
.KE

.KS
.ti 0
Appendix: table of datagram types

.TS
tab(:);
c | c
| l |  l | .
value:datagram type
=
1:HANDSHAKE_REQUEST
2:HANDSHAKE_RESPONSE
3:HANDSHAKE_COMPLETED
4:REQUEST
5:RESPONSE
_
.TE
.KE

