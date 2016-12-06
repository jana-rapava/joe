#include <czmq.h>
#include <stdlib.h>

#include "../include/joe_proto.h"

static
void s_server_actor (zsock_t *pipe, void *args)
{
    char *name = strdup ((char*) args);
    zsock_t *server = zsock_new_router ("inproc://fmq");
    zpoller_t *poller = zpoller_new (pipe, server, NULL);
    const char *filename;
    int state = 0;

    // to signal to runtime it should spawn the thread
    zsock_signal (pipe, 0);
    zsys_debug ("%s:\tstarted", name);

    while (!zsys_interrupted) {

        void *which = zpoller_wait (poller, -1);

        if (!which)
            break;

        if (which == pipe) {
            zmsg_t *msg = zmsg_recv (pipe);
            char *command = zmsg_popstr (msg);
            zsys_info ("Got API command=%s", command);
            if (streq (command, "$TERM")) {
                zstr_free (&command);
                zmsg_destroy (&msg);
                break;
            }
            zstr_free (&command);
            zmsg_destroy (&msg);
        }
        else if (which == server) {
            joe_proto_t *msg2 = joe_proto_new (); 
            joe_proto_recv (msg2, server);
            zframe_t *routing_id = joe_proto_routing_id (msg2);
            joe_proto_print (msg2);

            // server response
            joe_proto_t *response = joe_proto_new ();
            joe_proto_set_routing_id (response, routing_id);

            int r = rand();
            if (r < RAND_MAX / 5) {
                joe_proto_set_id (response, JOE_PROTO_ERROR);
                joe_proto_set_reason (response, "server not ready (random)");
            }
            else {
                if (state == 0 && joe_proto_id (msg2) != JOE_PROTO_HELLO) {
                    joe_proto_set_id (response, JOE_PROTO_ERROR);
                    joe_proto_set_reason (response, "bad protocol");
                }
                else {
                    state == 1;
                    filename = joe_proto_filename (msg2);
                    joe_proto_set_id (response, JOE_PROTO_READY);
                }
            }

            joe_proto_destroy (&msg2);
            joe_proto_send (response, server);
        }
    }

    zpoller_destroy (&poller);
    zsock_destroy (&server);
    zstr_free (&name);
}

static
void s_client_actor (zsock_t *pipe, void *args)
{
    char *name = strdup ((char*) args);
    zsock_t *client = zsock_new_dealer ("inproc://fmq");
    zpoller_t *poller = zpoller_new (pipe, client, NULL);
    int state = 0;

    // to signal to runtime it should spawn the thread
    zsock_signal (pipe, 0);
    zsys_debug ("%s:\tstarted", name);

    while (!zsys_interrupted) {

        joe_proto_t *msg = joe_proto_new ();
        joe_proto_set_id (msg, JOE_PROTO_HELLO);
        joe_proto_set_filename (msg, "/etc/passwd");
        joe_proto_send (msg, client);
        state = 1;

        void *which = zpoller_wait (poller, -1);

        if (!which)
            break;

        if (which == pipe) {
            zmsg_t *msg = zmsg_recv (pipe);
            char *command = zmsg_popstr (msg);
            zsys_info ("Got API command=%s", command);
            if (streq (command, "$TERM")) {
                zstr_free (&command);
                zmsg_destroy (&msg);
                break;
            }
            zstr_free (&command);
            zmsg_destroy (&msg);
        }
        else if (which == client) {
            zsys_debug ("recv on client");
            joe_proto_t *response = joe_proto_new ();
            joe_proto_recv (response, client);
            if (state == 1 && joe_proto_id (response) != JOE_PROTO_READY) {
                    zsys_error ("Server not ready. Aborting...");
                    joe_proto_print (response);
                    joe_proto_destroy (&response);
                    break;
            }
            state = 2;
            //TODO: start sending the file
            joe_proto_print (response);
            joe_proto_destroy (&response);
        }
    }

    zpoller_destroy (&poller);
    zsock_destroy (&client);
    zstr_free (&name);
}
int main () {
    zactor_t *client = zactor_new (s_client_actor, "client actor");
    zactor_t *server = zactor_new (s_server_actor, "server actor");
    zclock_sleep (250);

    zactor_destroy (&client);
    zactor_destroy (&server);
}
