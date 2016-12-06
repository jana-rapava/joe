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


void
joes_server (zsock_t *pipe, void *args)
{
    //    zsock_t *svr = zsock_new_router ("inproc://test");
    zsock_t *svr = zsock_new_router ("tcp://192.168.1.144:7777");
    
    // joe's message
    joe_proto_t *joes = joe_proto_new ();
    
    char *name = strdup ((char*) args);
    zpoller_t *poller = zpoller_new (pipe, svr, NULL);

    // to signal to runtime it should spawn the thread
    zsock_signal (pipe, 0);
    zsys_debug ("\t%s:started", name);

    while (!zsys_interrupted) {

        void *which = zpoller_wait (poller, -1);

        if (!which)
            break;
        
        if (which == pipe)
        {
            zmsg_t *msg = zmsg_recv (pipe);
            char *command = zmsg_popstr (msg);
            zsys_info ("\tGot API command=%s", command);

            if (streq (command, "$TERM"))
            {                
                zstr_free (&command);
                zmsg_destroy (&msg);
                break;
            }                       
        }

        if (which == svr)
        {
            int recv = joe_proto_recv (joes, svr);
            zsys_info ("\t%s receiving message", name);
            
            if (recv == -1)
                zsys_debug ("%s receive error", name);

            joe_proto_print (joes);

            zframe_t *routing_id = joe_proto_routing_id (joes);
            int command_subj = joe_proto_id (joes);
            
            zsys_info ("%s Got API command=%s", name , joe_proto_command (joes));

            if (command_subj == JOE_PROTO_HELLO)
            {
                joe_proto_t *send_joe = joe_proto_new ();
                joe_proto_set_id (send_joe, JOE_PROTO_READY);
                joe_proto_set_routing_id (send_joe, routing_id);
                int send = joe_proto_send (send_joe, svr);
                
                if (send != 0)
                    zsys_debug ("error while sending %s", name);
                zsys_info ("\t%s sending READY", name);
                joe_proto_print (send_joe);
                joe_proto_destroy (&send_joe);                                    
            }                
            else
            {
                joe_proto_t *send_joe = joe_proto_new ();
                joe_proto_set_id (send_joe, JOE_PROTO_ERROR);
                joe_proto_set_routing_id (send_joe,routing_id);

                int send = joe_proto_send (send_joe, svr);
                if (send != 0)
                    zsys_debug ("error while sending %s", name);
                zsys_info ("sending error");
                
                joe_proto_print (send_joe);
                joe_proto_destroy (&send_joe);                                   
            }
        }
        zclock_sleep (1000);                
    }
    joe_proto_destroy(&joes);
    zsock_destroy (&svr);
    zpoller_destroy (&poller);
    zstr_free (&name);
}

// client
void
test_client (zsock_t *pipe, void *args)
{
    zsock_t *client = zsock_new_dealer ("inproc://test");
    char *name = strdup ((char*) args);
    zpoller_t *poller = zpoller_new (pipe, client, NULL);

    // to signal to runtime it should spawn the thread
    zsock_signal (pipe, 0);
    zsys_debug ("\t%s:started", name);

    // first hello message
    joe_proto_t *joes = joe_proto_new ();
    joe_proto_set_id (joes, JOE_PROTO_HELLO);
    joe_proto_set_filename (joes, "/etc/password");
    
    int send = joe_proto_send (joes, client);
    zsys_info("\t%s sending HELLO message", name);
    
    if (send != 0)
        zsys_debug ("%s not send", name);

    while (!zsys_interrupted)
    {
        void *which = zpoller_wait (poller, -1);

        if (!which)
            break;
        
        if (which == pipe)
        {
            zmsg_t *msg = zmsg_recv (pipe);
            char *command = zmsg_popstr (msg);
            zsys_info ("Got API command=%s", command);

            if (streq (command, "$TERM"))
            {                
                zstr_free (&command);
                zmsg_destroy (&msg);
                break;
            }                       
        }     
        if (which == client)
        {
            joe_proto_t *recv = joe_proto_new ();
            int rec = joe_proto_recv (recv, client);
            if (rec != 0)
                zsys_debug ("%s receive not performed\n", name);
            
            int command_subj = joe_proto_id (recv);            
            zsys_info ("%s Got API command=%i", name, joe_proto_command (joes));

            //everything is ok so send the file
            if (command_subj == JOE_PROTO_READY)
            {               
                printf("CHUNK\n");

            }
            else if (command_subj == JOE_PROTO_ERROR)
            {               

                printf (" I am not going to send anything\n");
                break;
            }
            else
            {
                zsys_debug ("%s invalid subject ", name);
            }
                zclock_sleep (2000);                
        }  
    }
    joe_proto_destroy(&joes); 
    zsock_destroy (&client);
    zpoller_destroy (&poller);
    zstr_free (&name);
}





//  --------------------------------------------------------------------------
//  Self test of this class

void
joe_server_test (bool verbose)
{
       
    printf (" * joe_server: \n");

    //  @selftest
    //  Simple create/destroy test
    joe_server_t *self = joe_server_new ();
    assert (self);

    zactor_t *server = zactor_new (joes_server, "joes_server");
    //    zactor_t *client = zactor_new (test_client, "test_client");


    //    zactor_destroy (&client);
    zactor_destroy (&server);
    joe_server_destroy (&self);
    //  @end
    printf ("OK\n");
}
