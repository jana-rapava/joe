/*  =========================================================================
    ocontrol - server control protocol

    Codec header for ocontrol.

    ** WARNING *************************************************************
    THIS SOURCE FILE IS 100% GENERATED. If you edit this file, you will lose
    your changes at the next build cycle. This is great for temporary printf
    statements. DO NOT MAKE ANY CHANGES YOU WISH TO KEEP. The correct places
    for commits are:

     * The XML model used for this code generation: ocontrol.xml, or
     * The code generation script that built this file: zproto_codec_c
    ************************************************************************
    =========================================================================
*/

#ifndef OCONTROL_H_INCLUDED
#define OCONTROL_H_INCLUDED

/*  These are the ocontrol messages:

    QUIT - 
*/


#define OCONTROL_QUIT                       1

#include <czmq.h>

#ifdef __cplusplus
extern "C" {
#endif

//  Opaque class structure
#ifndef OCONTROL_T_DEFINED
typedef struct _ocontrol_t ocontrol_t;
#define OCONTROL_T_DEFINED
#endif

//  @interface
//  Create a new empty ocontrol
ocontrol_t *
    ocontrol_new (void);

//  Destroy a ocontrol instance
void
    ocontrol_destroy (ocontrol_t **self_p);

//  Receive a ocontrol from the socket. Returns 0 if OK, -1 if
//  the read was interrupted, or -2 if the message is malformed.
//  Blocks if there is no message waiting.
int
    ocontrol_recv (ocontrol_t *self, zsock_t *input);

//  Send the ocontrol to the output socket, does not destroy it
int
    ocontrol_send (ocontrol_t *self, zsock_t *output);


//  Print contents of message to stdout
void
    ocontrol_print (ocontrol_t *self);

//  Get/set the message routing id
zframe_t *
    ocontrol_routing_id (ocontrol_t *self);
void
    ocontrol_set_routing_id (ocontrol_t *self, zframe_t *routing_id);

//  Get the ocontrol id and printable command
int
    ocontrol_id (ocontrol_t *self);
void
    ocontrol_set_id (ocontrol_t *self, int id);
const char *
    ocontrol_command (ocontrol_t *self);

//  Self test of this class
void
    ocontrol_test (bool verbose);
//  @end

//  For backwards compatibility with old codecs
#define ocontrol_dump       ocontrol_print

#ifdef __cplusplus
}
#endif

#endif
