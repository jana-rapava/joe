#include <czmq.h>

int main () {
    zsys_init();

    zsock_t *server = zsock_new_router ("tcp://0.0.0.0:5555");

    while (!zsys_interrupted) {
        // server - receive the REQUEST
        zmsg_t *msg2 = zmsg_recv (server);
        zframe_t *routing_id = zmsg_pop (msg2);
        zmsg_print (msg2);
        zmsg_destroy (&msg2);

        // server response
        zmsg_t *response = zmsg_new ();
        zmsg_add (response, routing_id);
        zmsg_addstr (response, "ERROR");
        zmsg_send (&response, server);
    }

    zsock_destroy (&server);
}




