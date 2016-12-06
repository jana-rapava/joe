#include <zyre.h>
#include <getopt.h>
#include <libgen.h>

static void
usage(
        char *argv0
) {
    char *cmd = basename(argv0);
    printf("Usage: %s [-v|-q] [-i ifname] [-n nodename] [-c channel]\n", cmd);
    printf("Usage: %s [-l | --zyre-if-list]\n", cmd);
}

int
main (
        int argc,
        char **argv
) {
    int opt;
    char *opt_zyre_if = NULL;
    // zyre_if = "vbbr0";
    bool opt_zyre_if_list = false, opt_zyre_verbose = false;

    char *opt_zyre_name = "JIM";
    char *opt_zyre_channel = "ZMQ";

    const char* opts_str = "i:ln:c:vq";
    const struct option opts[] = {
        { "zyre-if", 1, NULL, 'i' },
        { "zyre-if-list", 0, NULL, 'l' },
        { "zyre-name", 1, NULL, 'n' },
        { "zyre-channel", 1, NULL, 'c' },
        { "verbose", 0, NULL, 'v' },
        { "quiet", 0, NULL, 'q' },
/*
        { "version", 0, NULL, 'V' },
        { "help", 0, NULL, 'h' },
*/
        { NULL, 0, NULL, 0 },
    };

    while ((opt = getopt_long(argc, argv, opts_str, opts, NULL)) != -1) {
        switch (opt) {
            case 'i':
                opt_zyre_if = optarg;
                break;
            case 'l':
                opt_zyre_if_list = true;
                break;
            case 'n':
                opt_zyre_name = optarg;
                break;
            case 'c':
                opt_zyre_channel = optarg;
                break;
            case 'v':
                opt_zyre_verbose = true;
                break;
            case 'q':
                opt_zyre_verbose = false;
                break;
            case 'h':
            default:
                usage(argv[0]);
                return 1;
        }
    }

    if (opt_zyre_if_list) {
        zsys_init();
        ziflist_t *iflist = ziflist_new ();
        zsys_info ("Interface list %s", (iflist != NULL) ? "below :" : "is <NULL>");
        if (iflist != NULL)
            ziflist_print (iflist);
        ziflist_destroy (&iflist);
        goto good_exit;
    }

    zyre_t *zyre = zyre_new (opt_zyre_name);
    if (opt_zyre_verbose)
        zyre_set_verbose (zyre);
    zsys_info ("Got zyre_uuid: %s", zyre_uuid (zyre));

    if (opt_zyre_if != NULL) {
        zsys_info ("Setting interface '%s' to use for zyre", opt_zyre_if);
        zyre_set_interface (zyre, opt_zyre_if);
    }

    zyre_set_header (zyre, "X-HELLO", "WORLD");
    zyre_start (zyre);

    zsys_info ("Joining channel '%s' as '%s:%s'...", opt_zyre_channel, zyre_name (zyre), zyre_uuid (zyre));
    zyre_join (zyre, opt_zyre_channel);

    zpoller_t *poller = zpoller_new (zyre_socket (zyre), NULL);
    uint64_t last_shout = 0;

    while (!zsys_interrupted) {
        void *which = zpoller_wait (poller, 5000);
        zsys_debug("%s dropped out of poller() at %" PRIu64, zyre_name (zyre), zclock_mono());

        if (zsys_interrupted) {
            zsys_info ("Sorry, I was interrupted");
            break;
        }

        if (zpoller_expired (poller) || zclock_mono () - last_shout >= 5000) {
            zsys_debug ("%s SHOUTS at %" PRIu64, zyre_name (zyre), zclock_mono());
            zyre_shouts (zyre, opt_zyre_channel, "Is anyone here? Reply-to: %s:%s",
                zyre_name (zyre),
                zyre_uuid (zyre)
            );
            last_shout = zclock_mono ();
        }

        if (which == zyre_socket (zyre)) {
            zyre_event_t *event = zyre_event_new (zyre);
            zyre_event_print (event);
            zyre_event_destroy (&event);
        }

        zsys_debug("%s continuing loop at %" PRIu64, zyre_name (zyre), zclock_mono());
    }

    zsys_info ("Infinite loop has ended, cleaning up (press Ctrl+C again if this seems frozen)...");
    zpoller_destroy (&poller);
    zyre_stop (zyre);
    zyre_destroy (&zyre);

good_exit:
    zsys_info ("All ended well\nSayonara");
    return 0;
}
