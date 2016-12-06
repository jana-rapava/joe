#include <czmq.h>

static
void s_actor1 (zsock_t *pipe, void *args)
{
    char *name = strdup ((char*) args);
    zpoller_t *poller = zpoller_new (pipe, NULL);

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
    }

    zpoller_destroy (&poller);
    zstr_free (&name);
}

int main () {


    zactor_t *server = zactor_new (s_actor1, "actor1");

    zstr_sendx (server, "HELLO", NULL);
    zstr_sendx (server, "HELLO", "WORLD", NULL);


    zclock_sleep (250);

    zactor_destroy (&server);

}
