/*  =========================================================================
    joe_proto - joe example protocol

    Codec header for joe_proto.

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

#ifndef JOE_PROTO_H_INCLUDED
#define JOE_PROTO_H_INCLUDED

/*  These are the joe_proto messages:

    HELLO - 
        filename            string      
        aux                 hash        

    READY - 
        filename            string      

    ERROR - 
        reason              string      
*/


#define JOE_PROTO_HELLO                     1
#define JOE_PROTO_READY                     2
#define JOE_PROTO_ERROR                     3

#include <czmq.h>

#ifdef __cplusplus
extern "C" {
#endif

//  Opaque class structure
#ifndef JOE_PROTO_T_DEFINED
typedef struct _joe_proto_t joe_proto_t;
#define JOE_PROTO_T_DEFINED
#endif

//  @interface
//  Create a new empty joe_proto
joe_proto_t *
    joe_proto_new (void);

//  Destroy a joe_proto instance
void
    joe_proto_destroy (joe_proto_t **self_p);

//  Receive a joe_proto from the socket. Returns 0 if OK, -1 if
//  the read was interrupted, or -2 if the message is malformed.
//  Blocks if there is no message waiting.
int
    joe_proto_recv (joe_proto_t *self, zsock_t *input);

//  Send the joe_proto to the output socket, does not destroy it
int
    joe_proto_send (joe_proto_t *self, zsock_t *output);


//  Print contents of message to stdout
void
    joe_proto_print (joe_proto_t *self);

//  Get/set the message routing id
zframe_t *
    joe_proto_routing_id (joe_proto_t *self);
void
    joe_proto_set_routing_id (joe_proto_t *self, zframe_t *routing_id);

//  Get the joe_proto id and printable command
int
    joe_proto_id (joe_proto_t *self);
void
    joe_proto_set_id (joe_proto_t *self, int id);
const char *
    joe_proto_command (joe_proto_t *self);

//  Get/set the filename field
const char *
    joe_proto_filename (joe_proto_t *self);
void
    joe_proto_set_filename (joe_proto_t *self, const char *value);

//  Get a copy of the aux field
zhash_t *
    joe_proto_aux (joe_proto_t *self);
//  Get the aux field and transfer ownership to caller
zhash_t *
    joe_proto_get_aux (joe_proto_t *self);
//  Set the aux field, transferring ownership from caller
void
    joe_proto_set_aux (joe_proto_t *self, zhash_t **hash_p);

//  Get/set the reason field
const char *
    joe_proto_reason (joe_proto_t *self);
void
    joe_proto_set_reason (joe_proto_t *self, const char *value);

//  Self test of this class
void
    joe_proto_test (bool verbose);
//  @end

//  For backwards compatibility with old codecs
#define joe_proto_dump      joe_proto_print

#ifdef __cplusplus
}
#endif

#endif
