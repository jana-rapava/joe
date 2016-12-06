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

//  --------------------------------------------------------------------------
//  Self test of this class

void
joe_server_test (bool verbose)
{
    printf (" * joe_server: ");

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
