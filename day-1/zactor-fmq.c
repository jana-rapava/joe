#include <czmq.h>
#include <stdlib.h>

static
void s_server_actor (zsock_t *pipe, void *args)
{
    char *name = strdup ((char*) args);
    zsock_t *server = zsock_new_router ("inproc://fmq");
    zpoller_t *poller = zpoller_new (pipe, server, NULL);
    char *filename = NULL;
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
            zmsg_t *msg2 = zmsg_recv (server);
            zframe_t *routing_id = zmsg_pop (msg2);
            zmsg_print (msg2);

            // server response
            zmsg_t *response = zmsg_new ();
            zmsg_add (response, routing_id);

            int r = rand();
            if (r < RAND_MAX / 5)
                zmsg_addstr (response, "ERROR Server not ready"); // we are not ready
            else {
                char *command = zmsg_popstr (msg2);
                if (state == 0) {
                    if (!streq (command, "HELLO"))
                        zmsg_addstr (response, "ERROR Protocol error: expecting HELLO");
                    else {
                        state == 1;
                        filename = zmsg_popstr (msg2);
                        zmsg_addstr (response, "READY");
                    }
                }
                zstr_free (&command);
            }

            zmsg_destroy (&msg2);
            zmsg_send (&response, server);
        }
    }

    zstr_free (&filename);
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

        zmsg_t *msg = zmsg_new ();
        zmsg_addstr (msg, "HELLO");
        zmsg_addstr (msg, "/etc/passwd");
        zmsg_send (&msg, client);
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
            zmsg_t *client_response = zmsg_recv (client);
            char *command = zmsg_popstr (client_response);
            if (state == 1) {
                if (!streq (command, "READY")) {
                    zsys_error ("Server not ready. Aborting...");
                    zstr_free (&command);
                    zmsg_print (client_response);
                    zmsg_destroy (&client_response);
                    break;
                }
                state = 2;
                //TODO: start sending the file
                zstr_free (&command);
                zmsg_print (client_response);
                zmsg_destroy (&client_response);
            }
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
