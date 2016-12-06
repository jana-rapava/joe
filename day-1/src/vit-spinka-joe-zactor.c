#include <joe_proto.h>

static void s_actor_server(zsock_t *pipe, void *args) {
    char *name = strdup ((char*) args);
    zsock_t *server = zsock_new_router ("tcp://172.17.0.3:7777");
    zpoller_t *poller = zpoller_new (pipe, server, NULL);

    // to signal to runtime it should spawn the thread
    zsock_signal (pipe, 0);
    zsys_debug ("%s:\tstarted", name);

    while (!zsys_interrupted) {
        void *which = zpoller_wait (poller, -1);

        if (!which)
            break;

        if (which == pipe) {
            zmsg_t *msg = zmsg_recv (pipe);
            char *command = zmsg_popstr (msg);
            zsys_info ("Got API command=%s", command);
            if (streq (command, "$TERM")) {
                zstr_free (&command);
                zmsg_destroy (&msg);
                break;
            }
            zstr_free (&command);
            zmsg_destroy (&msg);
        }
        if (which == server) {
          joe_proto_t *joe = joe_proto_new();
          joe_proto_recv(joe, server);
          joe_proto_print(joe);
          
          joe_proto_t *joe_resp = joe_proto_new();
          joe_proto_set_routing_id(joe_resp, joe_proto_routing_id(joe));
          joe_proto_destroy(&joe);

          joe_proto_set_id(joe_resp, JOE_PROTO_READY);
          joe_proto_send (joe_resp, server);
          joe_proto_print(joe_resp);
          joe_proto_destroy(&joe_resp);
        }
    }

    zsock_destroy (&server);
}

static void s_actor_client(zsock_t *pipe, void *args) {
    char *name = strdup ((char*) args);
    zsock_t *client = zsock_new_dealer ("tcp://172.17.0.3:7777");
    zpoller_t *poller = zpoller_new (pipe, client, NULL);

    // to signal to runtime it should spawn the thread
    zsock_signal (pipe, 0);
    zsys_debug ("%s:\tstarted", name);

    joe_proto_t *joe = joe_proto_new();
    joe_proto_set_id(joe, JOE_PROTO_HELLO);
    joe_proto_set_filename(joe, "/root/secret");
    joe_proto_print(joe);
    joe_proto_send(joe, client);
    joe_proto_destroy(&joe);
zsys_info("Client sent");
 
    while (!zsys_interrupted) {

        void *which = zpoller_wait (poller, -1);

        if (!which)
            break;

        if (which == pipe) {
            zmsg_t *msg = zmsg_recv (pipe);
            char *command = zmsg_popstr (msg);
            zsys_info ("Got API command=%s", command);
            if (streq (command, "$TERM")) {
                zstr_free (&command);
                zmsg_destroy (&msg);
                break;
            }
            zstr_free (&command);
            zmsg_destroy (&msg);
        }
        if (which == client) {
zsys_info("Client recv");
          joe_proto_t *joe = joe_proto_new();
          joe_proto_recv(joe, client);
          joe_proto_print(joe);
        }
    }
    zsock_destroy (&client);
}

int main(int argc, char **argv) {
    assert(argc==2);

    zactor_t *server = NULL;
    zactor_t *client = NULL;
    zactor_t *client2 = NULL;

    if(streq(argv[1], "server")) {
      server = zactor_new (s_actor_server, "server");
    } else if(streq(argv[1], "client")) {
      client = zactor_new (s_actor_client, "client");
    } else if(streq(argv[1], "both")) {
      server = zactor_new (s_actor_server, "server");
      client = zactor_new (s_actor_client, "client");
    } else if(streq(argv[1], "three")) {
      server = zactor_new (s_actor_server, "server");
      client = zactor_new (s_actor_client, "client");
      client2 = zactor_new (s_actor_client, "client2");
    }

    zclock_sleep (5000);

    zactor_destroy(&server);
    zactor_destroy(&client);
    zactor_destroy(&client2);

    return 0;
}
