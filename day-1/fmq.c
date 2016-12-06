#include <czmq.h>

int main () {

    zsys_init();

    zsock_t *server = zsock_new_router ("inproc://fmq");
    zsock_t *client = zsock_new_dealer ("inproc://fmq");

    // send a HELLO request
    zmsg_t *msg = zmsg_new ();
    zmsg_addstr (msg, "HELLO");
    zmsg_addstr (msg, "/etc/passwd");
    zmsg_send (&msg, client);

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

    // client reads response
    zsys_debug ("recv on client");
    zmsg_t *client_response = zmsg_recv (client);
    zmsg_print (client_response);
    zmsg_destroy (&client_response);

    zsock_destroy (&server);
    zsock_destroy (&client);

}
