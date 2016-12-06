#include <czmq.h>

int main(int argc, char **argv)
{
    assert(argc==2);
    zsock_t *push = zsock_new_push ("tcp://192.168.1.107:7777");
    zstr_send (push, argv[1]);

    zclock_sleep(1000);

    zsock_destroy (&push);
    return 0;
}
