#include <czmq.h>

int main () {

    zsys_init();

    zsock_t *client = zsock_new_dealer ("tcp://127.0.0.1:5555");

    // send a HELLO request
    zmsg_t *msg = zmsg_new ();
    zmsg_addstr (msg, "HELLO");
    zmsg_addstr (msg, "/etc/passwd");
    zmsg_send (&msg, client);

    zclock_sleep(200);

    // client reads response
    zsys_debug ("recv on client");
    zmsg_t *client_response = zmsg_recv (client);
    zmsg_print (client_response);
    zmsg_destroy (&client_response);

    zclock_sleep(1000);

    zsock_destroy (&client);

}
