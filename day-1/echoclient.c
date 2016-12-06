#include <czmq.h>
int main (void)
{
    zsock_t *push = zsock_new_push ("tcp://192.168.1.107:7777");

    while (!zsys_interrupted) {
        zstr_send (push, "Jana Rapava");
        sleep (5);
    }
    zsock_destroy (&push);
    return 0;
}
