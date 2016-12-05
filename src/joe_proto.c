/*  =========================================================================
    joe_proto - joe example protocol

    Codec class for joe_proto.

    ** WARNING *************************************************************
    THIS SOURCE FILE IS 100% GENERATED. If you edit this file, you will lose
    your changes at the next build cycle. This is great for temporary printf
    statements. DO NOT MAKE ANY CHANGES YOU WISH TO KEEP. The correct places
    for commits are:

     * The XML model used for this code generation: joe_proto.xml, or
     * The code generation script that built this file: zproto_codec_c
    ************************************************************************
    =========================================================================
*/

/*
@header
    joe_proto - joe example protocol
@discuss
@end
*/

#include "joe_proto.h"

//  Structure of our class

struct _joe_proto_t {
    zframe_t *routing_id;               //  Routing_id from ROUTER, if any
    int id;                             //  joe_proto message ID
    byte *needle;                       //  Read/write pointer for serialization
    byte *ceiling;                      //  Valid upper limit for read pointer
    char filename [256];                //  filename
    zhash_t *aux;                       //  aux
    size_t aux_bytes;                   //  Size of hash content
    char reason [256];                  //  reason
};

//  --------------------------------------------------------------------------
//  Network data encoding macros

//  Put a block of octets to the frame
#define PUT_OCTETS(host,size) { \
    memcpy (self->needle, (host), size); \
    self->needle += size; \
}

//  Get a block of octets from the frame
#define GET_OCTETS(host,size) { \
    if (self->needle + size > self->ceiling) { \
        zsys_warning ("joe_proto: GET_OCTETS failed"); \
        goto malformed; \
    } \
    memcpy ((host), self->needle, size); \
    self->needle += size; \
}

//  Put a 1-byte number to the frame
#define PUT_NUMBER1(host) { \
    *(byte *) self->needle = (byte) (host); \
    self->needle++; \
}

//  Put a 2-byte number to the frame
#define PUT_NUMBER2(host) { \
    self->needle [0] = (byte) (((host) >> 8)  & 255); \
    self->needle [1] = (byte) (((host))       & 255); \
    self->needle += 2; \
}

//  Put a 4-byte number to the frame
#define PUT_NUMBER4(host) { \
    self->needle [0] = (byte) (((host) >> 24) & 255); \
    self->needle [1] = (byte) (((host) >> 16) & 255); \
    self->needle [2] = (byte) (((host) >> 8)  & 255); \
    self->needle [3] = (byte) (((host))       & 255); \
    self->needle += 4; \
}

//  Put a 8-byte number to the frame
#define PUT_NUMBER8(host) { \
    self->needle [0] = (byte) (((host) >> 56) & 255); \
    self->needle [1] = (byte) (((host) >> 48) & 255); \
    self->needle [2] = (byte) (((host) >> 40) & 255); \
    self->needle [3] = (byte) (((host) >> 32) & 255); \
    self->needle [4] = (byte) (((host) >> 24) & 255); \
    self->needle [5] = (byte) (((host) >> 16) & 255); \
    self->needle [6] = (byte) (((host) >> 8)  & 255); \
    self->needle [7] = (byte) (((host))       & 255); \
    self->needle += 8; \
}

//  Get a 1-byte number from the frame
#define GET_NUMBER1(host) { \
    if (self->needle + 1 > self->ceiling) { \
        zsys_warning ("joe_proto: GET_NUMBER1 failed"); \
        goto malformed; \
    } \
    (host) = *(byte *) self->needle; \
    self->needle++; \
}

//  Get a 2-byte number from the frame
#define GET_NUMBER2(host) { \
    if (self->needle + 2 > self->ceiling) { \
        zsys_warning ("joe_proto: GET_NUMBER2 failed"); \
        goto malformed; \
    } \
    (host) = ((uint16_t) (self->needle [0]) << 8) \
           +  (uint16_t) (self->needle [1]); \
    self->needle += 2; \
}

//  Get a 4-byte number from the frame
#define GET_NUMBER4(host) { \
    if (self->needle + 4 > self->ceiling) { \
        zsys_warning ("joe_proto: GET_NUMBER4 failed"); \
        goto malformed; \
    } \
    (host) = ((uint32_t) (self->needle [0]) << 24) \
           + ((uint32_t) (self->needle [1]) << 16) \
           + ((uint32_t) (self->needle [2]) << 8) \
           +  (uint32_t) (self->needle [3]); \
    self->needle += 4; \
}

//  Get a 8-byte number from the frame
#define GET_NUMBER8(host) { \
    if (self->needle + 8 > self->ceiling) { \
        zsys_warning ("joe_proto: GET_NUMBER8 failed"); \
        goto malformed; \
    } \
    (host) = ((uint64_t) (self->needle [0]) << 56) \
           + ((uint64_t) (self->needle [1]) << 48) \
           + ((uint64_t) (self->needle [2]) << 40) \
           + ((uint64_t) (self->needle [3]) << 32) \
           + ((uint64_t) (self->needle [4]) << 24) \
           + ((uint64_t) (self->needle [5]) << 16) \
           + ((uint64_t) (self->needle [6]) << 8) \
           +  (uint64_t) (self->needle [7]); \
    self->needle += 8; \
}

//  Put a string to the frame
#define PUT_STRING(host) { \
    size_t string_size = strlen (host); \
    PUT_NUMBER1 (string_size); \
    memcpy (self->needle, (host), string_size); \
    self->needle += string_size; \
}

//  Get a string from the frame
#define GET_STRING(host) { \
    size_t string_size; \
    GET_NUMBER1 (string_size); \
    if (self->needle + string_size > (self->ceiling)) { \
        zsys_warning ("joe_proto: GET_STRING failed"); \
        goto malformed; \
    } \
    memcpy ((host), self->needle, string_size); \
    (host) [string_size] = 0; \
    self->needle += string_size; \
}

//  Put a long string to the frame
#define PUT_LONGSTR(host) { \
    size_t string_size = strlen (host); \
    PUT_NUMBER4 (string_size); \
    memcpy (self->needle, (host), string_size); \
    self->needle += string_size; \
}

//  Get a long string from the frame
#define GET_LONGSTR(host) { \
    size_t string_size; \
    GET_NUMBER4 (string_size); \
    if (self->needle + string_size > (self->ceiling)) { \
        zsys_warning ("joe_proto: GET_LONGSTR failed"); \
        goto malformed; \
    } \
    free ((host)); \
    (host) = (char *) malloc (string_size + 1); \
    memcpy ((host), self->needle, string_size); \
    (host) [string_size] = 0; \
    self->needle += string_size; \
}


//  --------------------------------------------------------------------------
//  Create a new joe_proto

joe_proto_t *
joe_proto_new (void)
{
    joe_proto_t *self = (joe_proto_t *) zmalloc (sizeof (joe_proto_t));
    return self;
}


//  --------------------------------------------------------------------------
//  Destroy the joe_proto

void
joe_proto_destroy (joe_proto_t **self_p)
{
    assert (self_p);
    if (*self_p) {
        joe_proto_t *self = *self_p;

        //  Free class properties
        zframe_destroy (&self->routing_id);
        zhash_destroy (&self->aux);

        //  Free object itself
        free (self);
        *self_p = NULL;
    }
}


//  --------------------------------------------------------------------------
//  Receive a joe_proto from the socket. Returns 0 if OK, -1 if
//  the recv was interrupted, or -2 if the message is malformed.
//  Blocks if there is no message waiting.

int
joe_proto_recv (joe_proto_t *self, zsock_t *input)
{
    assert (input);
    int rc = 0;

    if (zsock_type (input) == ZMQ_ROUTER) {
        zframe_destroy (&self->routing_id);
        self->routing_id = zframe_recv (input);
        if (!self->routing_id || !zsock_rcvmore (input)) {
            zsys_warning ("joe_proto: no routing ID");
            rc = -1;            //  Interrupted
            goto malformed;
        }
    }
    zmq_msg_t frame;
    zmq_msg_init (&frame);
    int size = zmq_msg_recv (&frame, zsock_resolve (input), 0);
    if (size == -1) {
        zsys_warning ("joe_proto: interrupted");
        rc = -1;                //  Interrupted
        goto malformed;
    }
    //  Get and check protocol signature
    self->needle = (byte *) zmq_msg_data (&frame);
    self->ceiling = self->needle + zmq_msg_size (&frame);

    uint16_t signature;
    GET_NUMBER2 (signature);
    if (signature != (0xAAA0 | 0)) {
        zsys_warning ("joe_proto: invalid signature");
        rc = -2;                //  Malformed
        goto malformed;
    }
    //  Get message id and parse per message type
    GET_NUMBER1 (self->id);

    switch (self->id) {
        case JOE_PROTO_HELLO:
            GET_STRING (self->filename);
            {
                size_t hash_size;
                GET_NUMBER4 (hash_size);
                self->aux = zhash_new ();
                zhash_autofree (self->aux);
                while (hash_size--) {
                    char key [256];
                    char *value = NULL;
                    GET_STRING (key);
                    GET_LONGSTR (value);
                    zhash_insert (self->aux, key, value);
                    free (value);
                }
            }
            break;

        case JOE_PROTO_READY:
            GET_STRING (self->filename);
            break;

        case JOE_PROTO_ERROR:
            GET_STRING (self->reason);
            break;

        default:
            zsys_warning ("joe_proto: bad message ID");
            rc = -2;            //  Malformed
            goto malformed;
    }
    //  Successful return
    zmq_msg_close (&frame);
    return rc;

    //  Error returns
    malformed:
        zmq_msg_close (&frame);
        return rc;              //  Invalid message
}


//  --------------------------------------------------------------------------
//  Send the joe_proto to the socket. Does not destroy it. Returns 0 if
//  OK, else -1.

int
joe_proto_send (joe_proto_t *self, zsock_t *output)
{
    assert (self);
    assert (output);

    if (zsock_type (output) == ZMQ_ROUTER)
        zframe_send (&self->routing_id, output, ZFRAME_MORE + ZFRAME_REUSE);

    size_t frame_size = 2 + 1;          //  Signature and message ID
    switch (self->id) {
        case JOE_PROTO_HELLO:
            frame_size += 1 + strlen (self->filename);
            frame_size += 4;            //  Size is 4 octets
            if (self->aux) {
                self->aux_bytes = 0;
                char *item = (char *) zhash_first (self->aux);
                while (item) {
                    self->aux_bytes += 1 + strlen (zhash_cursor (self->aux));
                    self->aux_bytes += 4 + strlen (item);
                    item = (char *) zhash_next (self->aux);
                }
            }
            frame_size += self->aux_bytes;
            break;
        case JOE_PROTO_READY:
            frame_size += 1 + strlen (self->filename);
            break;
        case JOE_PROTO_ERROR:
            frame_size += 1 + strlen (self->reason);
            break;
    }
    //  Now serialize message into the frame
    zmq_msg_t frame;
    zmq_msg_init_size (&frame, frame_size);
    self->needle = (byte *) zmq_msg_data (&frame);
    PUT_NUMBER2 (0xAAA0 | 0);
    PUT_NUMBER1 (self->id);
    size_t nbr_frames = 1;              //  Total number of frames to send

    switch (self->id) {
        case JOE_PROTO_HELLO:
            PUT_STRING (self->filename);
            if (self->aux) {
                PUT_NUMBER4 (zhash_size (self->aux));
                char *item = (char *) zhash_first (self->aux);
                while (item) {
                    PUT_STRING (zhash_cursor (self->aux));
                    PUT_LONGSTR (item);
                    item = (char *) zhash_next (self->aux);
                }
            }
            else
                PUT_NUMBER4 (0);    //  Empty hash
            break;

        case JOE_PROTO_READY:
            PUT_STRING (self->filename);
            break;

        case JOE_PROTO_ERROR:
            PUT_STRING (self->reason);
            break;

    }
    //  Now send the data frame
    zmq_msg_send (&frame, zsock_resolve (output), --nbr_frames? ZMQ_SNDMORE: 0);

    return 0;
}


//  --------------------------------------------------------------------------
//  Print contents of message to stdout

void
joe_proto_print (joe_proto_t *self)
{
    assert (self);
    switch (self->id) {
        case JOE_PROTO_HELLO:
            zsys_debug ("JOE_PROTO_HELLO:");
            zsys_debug ("    filename='%s'", self->filename);
            zsys_debug ("    aux=");
            if (self->aux) {
                char *item = (char *) zhash_first (self->aux);
                while (item) {
                    zsys_debug ("        %s=%s", zhash_cursor (self->aux), item);
                    item = (char *) zhash_next (self->aux);
                }
            }
            else
                zsys_debug ("(NULL)");
            break;

        case JOE_PROTO_READY:
            zsys_debug ("JOE_PROTO_READY:");
            zsys_debug ("    filename='%s'", self->filename);
            break;

        case JOE_PROTO_ERROR:
            zsys_debug ("JOE_PROTO_ERROR:");
            zsys_debug ("    reason='%s'", self->reason);
            break;

    }
}


//  --------------------------------------------------------------------------
//  Get/set the message routing_id

zframe_t *
joe_proto_routing_id (joe_proto_t *self)
{
    assert (self);
    return self->routing_id;
}

void
joe_proto_set_routing_id (joe_proto_t *self, zframe_t *routing_id)
{
    if (self->routing_id)
        zframe_destroy (&self->routing_id);
    self->routing_id = zframe_dup (routing_id);
}


//  --------------------------------------------------------------------------
//  Get/set the joe_proto id

int
joe_proto_id (joe_proto_t *self)
{
    assert (self);
    return self->id;
}

void
joe_proto_set_id (joe_proto_t *self, int id)
{
    self->id = id;
}

//  --------------------------------------------------------------------------
//  Return a printable command string

const char *
joe_proto_command (joe_proto_t *self)
{
    assert (self);
    switch (self->id) {
        case JOE_PROTO_HELLO:
            return ("HELLO");
            break;
        case JOE_PROTO_READY:
            return ("READY");
            break;
        case JOE_PROTO_ERROR:
            return ("ERROR");
            break;
    }
    return "?";
}

//  --------------------------------------------------------------------------
//  Get/set the filename field

const char *
joe_proto_filename (joe_proto_t *self)
{
    assert (self);
    return self->filename;
}

void
joe_proto_set_filename (joe_proto_t *self, const char *value)
{
    assert (self);
    assert (value);
    if (value == self->filename)
        return;
    strncpy (self->filename, value, 255);
    self->filename [255] = 0;
}


//  --------------------------------------------------------------------------
//  Get the aux field without transferring ownership

zhash_t *
joe_proto_aux (joe_proto_t *self)
{
    assert (self);
    return self->aux;
}

//  Get the aux field and transfer ownership to caller

zhash_t *
joe_proto_get_aux (joe_proto_t *self)
{
    zhash_t *aux = self->aux;
    self->aux = NULL;
    return aux;
}

//  Set the aux field, transferring ownership from caller

void
joe_proto_set_aux (joe_proto_t *self, zhash_t **aux_p)
{
    assert (self);
    assert (aux_p);
    zhash_destroy (&self->aux);
    self->aux = *aux_p;
    *aux_p = NULL;
}


//  --------------------------------------------------------------------------
//  Get/set the reason field

const char *
joe_proto_reason (joe_proto_t *self)
{
    assert (self);
    return self->reason;
}

void
joe_proto_set_reason (joe_proto_t *self, const char *value)
{
    assert (self);
    assert (value);
    if (value == self->reason)
        return;
    strncpy (self->reason, value, 255);
    self->reason [255] = 0;
}



//  --------------------------------------------------------------------------
//  Selftest

void
joe_proto_test (bool verbose)
{
    printf (" * joe_proto: ");

    if (verbose)
        printf ("\n");

    //  @selftest
    //  Simple create/destroy test
    joe_proto_t *self = joe_proto_new ();
    assert (self);
    joe_proto_destroy (&self);
    //  Create pair of sockets we can send through
    //  We must bind before connect if we wish to remain compatible with ZeroMQ < v4
    zsock_t *output = zsock_new (ZMQ_DEALER);
    assert (output);
    int rc = zsock_bind (output, "inproc://selftest-joe_proto");
    assert (rc == 0);

    zsock_t *input = zsock_new (ZMQ_ROUTER);
    assert (input);
    rc = zsock_connect (input, "inproc://selftest-joe_proto");
    assert (rc == 0);


    //  Encode/send/decode and verify each message type
    int instance;
    self = joe_proto_new ();
    joe_proto_set_id (self, JOE_PROTO_HELLO);

    joe_proto_set_filename (self, "Life is short but Now lasts for ever");
    zhash_t *hello_aux = zhash_new ();
    zhash_insert (hello_aux, "Name", "Brutus");
    joe_proto_set_aux (self, &hello_aux);
    //  Send twice
    joe_proto_send (self, output);
    joe_proto_send (self, output);

    for (instance = 0; instance < 2; instance++) {
        joe_proto_recv (self, input);
        assert (joe_proto_routing_id (self));
        assert (streq (joe_proto_filename (self), "Life is short but Now lasts for ever"));
        zhash_t *aux = joe_proto_get_aux (self);
        assert (zhash_size (aux) == 2);
        assert (streq ((char *) zhash_first (aux), "Brutus"));
        assert (streq ((char *) zhash_cursor (aux), "Name"));
        zhash_destroy (&aux);
        if (instance == 1)
            zhash_destroy (&hello_aux);
    }
    joe_proto_set_id (self, JOE_PROTO_READY);

    joe_proto_set_filename (self, "Life is short but Now lasts for ever");
    //  Send twice
    joe_proto_send (self, output);
    joe_proto_send (self, output);

    for (instance = 0; instance < 2; instance++) {
        joe_proto_recv (self, input);
        assert (joe_proto_routing_id (self));
        assert (streq (joe_proto_filename (self), "Life is short but Now lasts for ever"));
    }
    joe_proto_set_id (self, JOE_PROTO_ERROR);

    joe_proto_set_reason (self, "Life is short but Now lasts for ever");
    //  Send twice
    joe_proto_send (self, output);
    joe_proto_send (self, output);

    for (instance = 0; instance < 2; instance++) {
        joe_proto_recv (self, input);
        assert (joe_proto_routing_id (self));
        assert (streq (joe_proto_reason (self), "Life is short but Now lasts for ever"));
    }

    joe_proto_destroy (&self);
    zsock_destroy (&input);
    zsock_destroy (&output);
    //  @end

    printf ("OK\n");
}
