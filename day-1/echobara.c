#include <czmq.h>

int main (void)
{
    // CLIENTS: USE
    zsock_t *push = zsock_new_push ("tcp://192.168.1.107:7777");
    zstr_send (push, "bara");
    zclock_sleep (1000);
    zsock_destroy (&push);
    return 0;
}
