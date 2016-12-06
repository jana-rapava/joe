#include <czmq.h>

int main (void)
{
    // CLIENTS: USE
    // zsock_new_push ("tcp://192.168.1.107:7777");
    zsock_t *pull = zsock_new_pull ("tcp://192.168.1.107:7777");

    while (!zsys_interrupted) {

        char *msg = zstr_recv (pull);
        zsys_info ("%s\n", msg);
        zstr_free (&msg);

    }

    zsock_destroy (&pull);
}
