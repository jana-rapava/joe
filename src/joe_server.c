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

#include <czmq.h>
#include <stdlib.h>
#include <stdio.h>

#include "joe_proto.h"

void joe_server_actor(
        zsock_t* pipe_,
        void* udata_) {
    JoeServerActorParams* params_ = (JoeServerActorParams*) udata_;
    char* name_ = strdup(params_ -> actor_name);
    char* url_ = strdup(params_ -> bind_url);

    zsock_t* server_ = zsock_new_router(url_);
    zpoller_t* poller_ = zpoller_new(pipe_, server_, NULL);

    // to signal to runtime it should spawn the thread
    zsock_signal(pipe_, 0);
    zsys_debug("%s: started", name_);

    while(!zsys_interrupted) {
        /* -- wait for an event */
        void* which_ = zpoller_wait(poller_, -1);
        if(!which_)
            break;

        if(which_ == pipe_) {
            zmsg_t* msg_ = zmsg_recv(pipe_);
            zmsg_print(msg_);
            char* command_ = zmsg_popstr(msg_);
            if(strcmp(command_, "QUIT") == 0) {
                zsys_debug("%s: quit", name_);
                zstr_free(&command_);
                zmsg_destroy(&msg_);
                break;
            }
            zstr_free(&command_);
            zmsg_destroy(&msg_);
        }
        else if(which_ == server_) {
            joe_proto_t* message_ = joe_proto_new();
            joe_proto_recv(message_, server_);
            joe_proto_print(message_);

            joe_proto_t* response_ = joe_proto_new();
            joe_proto_set_routing_id(response_, joe_proto_routing_id(message_));
            if(joe_proto_id(message_) == JOE_PROTO_HELLO) {
                joe_proto_set_id(response_, JOE_PROTO_READY);
            }
            else if(joe_proto_id(message_) == JOE_PROTO_CHUNK) {
                zsys_info("  filename: %s", joe_proto_filename(message_));
                zsys_info("  offset: %lld", joe_proto_offset(message_));
                zsys_info("  size: %lld", joe_proto_size(message_));
                zsys_info("  checksum: %llx", joe_proto_checksum(message_));
                zchunk_t* data_ = joe_proto_data(message_);

                FILE* file_ = fopen(joe_proto_filename(message_), "wb");
                if(file_ != NULL) {
                    if(fseek(file_, joe_proto_offset(message_), SEEK_SET) == joe_proto_offset(message_)) {
                        zchunk_write(data_, file_);
                        fclose(file_);
                        joe_proto_set_id(response_, JOE_PROTO_READY);
                    }
                    else {
                        joe_proto_set_id(response_, JOE_PROTO_ERROR);
                        joe_proto_set_reason(response_, "Cannot write into the file.");
                    }
                }
                else {
                    joe_proto_set_id(response_, JOE_PROTO_ERROR);
                    joe_proto_set_reason(response_, "Cannot open the file.");
                }
//                zchunk_destroy(&data_);
            }
            else {
                joe_proto_set_id(response_, JOE_PROTO_ERROR);
                joe_proto_set_reason(response_, "Invalid protocol command");
            }
            joe_proto_send(response_, server_);

            joe_proto_destroy(&message_);
            joe_proto_destroy(&response_);
        }
    }

    zpoller_destroy(&poller_);
    zsock_destroy(&server_);
    free(name_);
    free(url_);
    zsock_signal(pipe_, 0);
}


void
joes_server (zsock_t *pipe, void *args)
{
    //    zsock_t *svr = zsock_new_router ("inproc://test");
    zsock_t *svr = NULL;
    
    // joe's message
    joe_proto_t *joes = joe_proto_new ();
    
    char *name = strdup ((char*) args);
    zpoller_t *poller = zpoller_new (pipe, NULL);

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
            else
            if (streq (command, "BIND")) {
                if (svr) {
                    zsys_warning ("Already connected, nothing to do");
                }
                else
                {
                    char *endpoint = zmsg_popstr (msg);
                    zsys_debug ("endpoint=%s", endpoint);
                    svr = zsock_new_router (endpoint);
                    zpoller_add (poller, svr);
                    zstr_free (&endpoint);
                }
            }
            zmsg_destroy (&msg);
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

    JoeServerActorParams params_ = {"server1", JOE_SERVER_TEST_SERVICE_URL};
    zactor_t *server_ = zactor_new(joe_server_actor, &params_);
    assert(server_ != NULL);

    zsock_t *client_ = zsock_new_dealer(JOE_SERVER_TEST_SERVICE_URL);

    /* -- test the HELLO message */
    joe_proto_t* message_ = joe_proto_new();
    joe_proto_set_id(message_, JOE_PROTO_HELLO);
    joe_proto_set_filename(message_, "/etc/passwd");
    joe_proto_send(message_, client_);
    joe_proto_destroy(&message_);
    joe_proto_t* response_ = joe_proto_new();
    joe_proto_recv(response_, client_);
    joe_proto_print(response_);
    assert(joe_proto_id(response_) == JOE_PROTO_READY);
    joe_proto_destroy(&response_);

    /* -- invalid command */
    message_ = joe_proto_new();
    joe_proto_set_id(message_, JOE_PROTO_READY);
    joe_proto_send(message_, client_);
    joe_proto_destroy(&message_);
    response_ = joe_proto_new();
    joe_proto_recv(response_, client_);
    joe_proto_print(response_);
    assert(joe_proto_id(response_) == JOE_PROTO_ERROR);
    joe_proto_destroy(&response_);

    zsock_destroy(&client_);

    /* -- finish the server */
    zstr_sendx(server_, "QUIT", NULL);
    zsock_wait(server_);
    zactor_destroy(&server_);

    printf ("OK\n");
}
