#include <zyre.h>

int main () {

    ziflist_t *iflist = ziflist_new ();
    ziflist_print (iflist);
    ziflist_destroy (&iflist);


    zyre_t *zyre = zyre_new ("MVY");
    zyre_set_verbose (zyre);
    zsys_info ("zyre_uuid=%s", zyre_uuid (zyre));

    zyre_set_header (zyre, "X-HELLO", "WORLD");
    zyre_start (zyre);

    zyre_join (zyre, "ZMQ");

    zpoller_t *poller = zpoller_new (zyre_socket (zyre), NULL);
    uint64_t last_shout = 0;

    while (!zsys_interrupted) {

        void *which = zpoller_wait (poller, 5000);

        if (zpoller_expired (poller) || zclock_mono () - last_shout >= 5000) {
            zsys_debug ("MVY SHOUTS");
            zyre_shouts (zyre, "ZMQ", "%s:%s",
                zyre_name (zyre),
                zyre_uuid (zyre));
            last_shout = zclock_mono ();
        }

        if (which == zyre_socket (zyre)) {
            zmsg_t *msg = zyre_recv (zyre);
            zmsg_print (msg);
            zmsg_destroy (&msg);
        }

    }

    zpoller_destroy (&poller);
    zyre_destroy (&zyre);

}
