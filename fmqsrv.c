#include <czmq.h>
#include <malloc.h>
#include <string.h>

static void serverActor(
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
                zsys_debug("%s: quit");

                zmsg_t* msg_ = zmsg_new();
                zmsg_addstr(msg_, "OK");
                zmsg_send(&msg_, pipe_);

                zstr_free(&command_);
                break;
            }
            zstr_free(&command_);
        }
        else if(which_ == server_) {
            // server - receive the REQUEST
            zmsg_t *msg2 = zmsg_recv(server_);
            zmsg_print(msg2);
            zframe_t *routing_id = zmsg_pop (msg2);
            char* command = zmsg_popstr(msg2);
            zmsg_destroy(&msg2);

            // server response
            zmsg_t *response = zmsg_new ();
            zmsg_add (response, routing_id);
            if(strcmp(command, "HELLO") == 0) {
                zmsg_addstr(response, "READY");
            }
            else {
                zmsg_addstr (response, "ERROR");
                zmsg_addstr (response, "Invalid protocol command");
            }
            zmsg_send (&response, server_);
            zstr_free(&command);
        }
    }

    zpoller_destroy(&poller_);
    zsock_destroy(&server_);
    free(name_);
}

int main () {
    zsys_init();

    zactor_t *server_ = zactor_new(serverActor, "server1");
    zclock_sleep(10000);
    zstr_sendx(server_, "QUIT", NULL);
    char* message_ = zstr_recv(server_);
    zstr_free(&message_);
    zsys_debug("Process ended");
    zactor_destroy(&server_);

    return 0;
}
