#include <czmq.h>
#include <time.h>

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
          zmsg_t *msg = zmsg_recv (server);
          zmsg_print(msg);
  
          zframe_t *routing_id = zmsg_pop(msg);
          char *command_text = zmsg_popstr (msg);
          char *command_filename = zmsg_popstr (msg);
          zmsg_destroy (&msg);
   
          int ok = 1;
          if (strcmp(command_text,"HELLO"))
            ok = 0;
  
          int r = rand();
          if (r < RAND_MAX*0.5)
            ok = 2; 
  
          zmsg_t *response = zmsg_new();
          zmsg_add (response, routing_id);
          if (ok == 1) {
            zmsg_addstr(response, "READY");
            zmsg_addstr(response, command_filename);
          } else if (ok ==0)  {
            zmsg_addstr(response, "ERROR");
            zmsg_addstr(response, "Invalid request");
          } else {
            zmsg_addstr(response, "ERROR");
            zmsg_addstr(response, "I don't like you");
          }
          zmsg_send(&response, server);
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

    zmsg_t *msg = zmsg_new();
    zmsg_addstr(msg, "HELLO");
    zmsg_addstr(msg, "/root/secret.txt");
 
    zmsg_print(msg);
    zmsg_send(&msg, client);

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
            msg = zmsg_recv (client);
            zmsg_print(msg);
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
