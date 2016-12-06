/*  =========================================================================
    joe_server - class description

    Copyright (c) the Contributors as noted in the AUTHORS file.       
    This file is part of CZMQ, the high-level C binding for 0MQ:       
    http://czmq.zeromq.org.                                            
                                                                       
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.           
    =========================================================================
*/

/*
@header
    joe_server - 
@discuss
@end
*/

#include "joe_classes.h"

//  Structure of our class

struct _joe_server_t {
    int filler;     //  Declare class properties here
};


//  --------------------------------------------------------------------------
//  Create a new joe_server

joe_server_t *
joe_server_new (void)
{
    joe_server_t *self = (joe_server_t *) zmalloc (sizeof (joe_server_t));
    assert (self);
    //  Initialize class properties here
    return self;
}


//  --------------------------------------------------------------------------
//  Destroy the joe_server

void
joe_server_destroy (joe_server_t **self_p)
{
    assert (self_p);
    if (*self_p) {
        joe_server_t *self = *self_p;
        //  Free class properties here
        //  Free object itself
        free (self);
        *self_p = NULL;
    }
}

//  --------------------------------------------------------------------------
//  Self test of this class

void
joe_server_test (bool verbose)
{
    printf (" * joe_server: ");

    //  @selftest
    //  Simple create/destroy test
    joe_server_t *self = joe_server_new ();
    assert (self);
    joe_server_destroy (&self);
    //  @end
    printf ("OK\n");
}
