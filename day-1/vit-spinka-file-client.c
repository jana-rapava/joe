#include <czmq.h>

int main(int argc, char **argv) {
    assert(argc==2);

    zsock_t *client = zsock_new_dealer ("tcp://172.17.0.3:7777");

    while (!zsys_interrupted) {
        zmsg_t *msg = zmsg_new();
        zmsg_addstr(msg, "HELLO");
        zmsg_addstr(msg, argv[1]);
 
        zmsg_print(msg);
        zmsg_send(&msg, client);
        zclock_sleep(2000);
 
        msg = zmsg_recv (client);
        zmsg_print(msg);
    }

    zsock_destroy (&client);

    return 0;
}

