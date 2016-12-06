#include <zyre.h>

int main(int argc, char **argv) {
    zyre_t *zyre = zyre_new ("JIM");
    zsys_info ("Got zyre_uuid: %s", zyre_uuid(zyre) );

    zyre_set_header (zyre, "X-HELLO", "WORLD");
    zyre_start (zyre);

    zyre_join (zyre, "ZMQ");

    zpoller_t *poller = zpoller_new(zyre_socket(zyre), NULL);
    uint64_t last_shout=0;

    while (!zsys_interrupted) {
        void *which = zpoller_wait (poller, 5000);

        if (zpoller_expired(poller) || zclock_mono() - last_shout >= 5000) {
            zsys_debug ("SHOUTS");
            zyre_shouts (zyre, "ZMQ", "Is anyone here? Reply-to: %s:%s",
                zyre_name(zyre),
                zyre_uuid(zyre)
            );
            last_shout = zclock_mono();
        } else {
            if (which == zyre_socket(zyre)) {
                zmsg_t *msg = zyre_recv(zyre);
                zmsg_print (msg);
                zmsg_destroy (&msg);
            }
        }

    }

    zsys_info ("Infinite loop has ended, cleaning up...");
    zpoller_destroy(&poller);
    zyre_destroy (&zyre);

    zsys_info ("Sayonara");
    return 0;
}
