#include "fmqsrv.h"

#include <czmq.h>
#include <malloc.h>
#include <string.h>

#include "fmq_proto.h"
#include "ocontrol.h"

void serverActor(
        zsock_t* pipe_,
        void* udata_) {
    char* name_ = strdup((char*) udata_);

    zsock_t* server_ = zsock_new_router("tcp://0.0.0.0:5555");
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
                break;
            }
            zstr_free(&command_);
        }
        else if(which_ == server_) {
            fmq_proto_t* message_ = fmq_proto_new();
            fmq_proto_recv(message_, server_);
            fmq_proto_print(message_);

            fmq_proto_t* response_ = fmq_proto_new();
            fmq_proto_set_routing_id(response_, fmq_proto_routing_id(message_));
            if(fmq_proto_id(message_) == FMQ_PROTO_HELLO) {
                fmq_proto_set_id(response_, FMQ_PROTO_READY);
                fmq_proto_set_filename(response_, fmq_proto_filename(message_));
            }
            else {
                fmq_proto_set_id(response_, FMQ_PROTO_ERROR);
                fmq_proto_set_reason(response_, "Invalid protocol command");
            }
            fmq_proto_send(response_, server_);

            fmq_proto_destroy(&message_);
            fmq_proto_destroy(&response_);
        }
    }

    zpoller_destroy(&poller_);
    zsock_destroy(&server_);
    free(name_);
    zsock_signal(pipe_, 0);
}

