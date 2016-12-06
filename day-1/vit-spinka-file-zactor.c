#include <czmq.h>
#include <time.h>

static void s_actor_server(zsock_t *pipe, void *args) {
    char *name = strdup ((char*) args);
    zsock_t *server = zsock_new_router ("tcp://172.17.0.3:7777");
    zpoller_t *poller = zpoller_new (pipe, server, NULL);

    // to signal to runtime it should spawn the thread
    zsock_signal (pipe, 0);
    zsys_debug ("%s:\tstarted", name);

    char *filename = NULL;
    long filesize = 0;

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

          zmsg_t *response = zmsg_new();
          zmsg_add (response, routing_id);

          if(streq(command_text, "HELLO")) {
            char *command_filename = zmsg_popstr (msg);
            if (!filename) {
              filename = strdup(command_filename);
              filesize = 0;
              zmsg_addstr(response, "READY");
              zmsg_addstr(response, "New filename accepted.");
            } else if (*command_filename && !streq(filename, command_filename)) {
              zmsg_addstr(response, "ERROR");
              zmsg_addstr(response, "Another file already in transfer.");
            } else if (*command_filename && streq(filename, command_filename)) {
              zmsg_addstr(response, "ERROR");
              zmsg_addstr(response, "This file already in transfer.");
            } else {
              zmsg_addstr(response, "ERROR");
              zmsg_addstr(response, "No filename specified.");
            }
          } else if(streq(command_text, "CHUNK")) {
            char *command_filename = zmsg_popstr (msg);
            long command_offset = atoll(zmsg_popstr (msg));
            long command_size = atoll(zmsg_popstr (msg));
            long command_checksum = atoll(zmsg_popstr (msg));
            zframe_t *command_data = zmsg_pop(msg);
            if (!filename) {
              zmsg_addstr(response, "ERROR");
              zmsg_addstr(response, "No file was open in transfer.");
            } else if (*command_filename && !streq(filename, command_filename)) {
              zmsg_addstr(response, "ERROR");
              zmsg_addstr(response, "Another file is in transfer.");
            } else if (*command_filename && streq(filename, command_filename)) {
              /* sending the right file */
              //TODO process chunk data
              filesize = command_offset + command_size;
              zmsg_addstr(response, "READY");
              zmsg_addstr(response, "Chunk transfer ok.");
            } else {
              zmsg_addstr(response, "ERROR");
              zmsg_addstr(response, "No filename specified");
            }
            zframe_destroy(&command_data);
          } else if(streq(command_text, "CLOSE")) {
            char *command_filename = zmsg_popstr (msg);
            long command_filesize = atoll(zmsg_popstr (msg));
            if (!filename) {
              zmsg_addstr(response, "ERROR");
              zmsg_addstr(response, "No file was open in transfer.");
            } else if (*command_filename && !streq(filename, command_filename)) {
              zmsg_addstr(response, "ERROR");
              zmsg_addstr(response, "Another file is in transfer.");
              free(filename);
              filename = NULL;
            } else if (*command_filename && streq(filename, command_filename)) {
              /* closing the right file */
              if (filesize == command_filesize) {
                zmsg_addstr(response, "READY");
                zmsg_addstr(response, "File transfer completed.");
              } else {
                zmsg_addstr(response, "ERROR");
                zmsg_addstr(response, "Size mismatch.");
              }
              free(filename);
              filename = NULL;
            } else {
              zmsg_addstr(response, "ERROR");
              zmsg_addstr(response, "No filename specified");
              free(filename);
              filename = NULL;
            }
          } else {
            zmsg_addstr(response, "ERROR");
            zmsg_addstr(response, "Invalid request");
          }
          zmsg_destroy (&msg);

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
    char *filename_full = "/home/oracle/release_dir/replicate/README.txt";
    char *filename_target = "README.txt";
    FILE *file = fopen(filename_full, "r");
    assert(file);
    long file_size_so_far = 0;
    int finished = 0;

    zmsg_t *msg = zmsg_new();
    zmsg_addstr(msg, "HELLO");
    zmsg_addstr(msg, filename_target);
 
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
            char *result = zmsg_popstr (msg);
            if (!streq(result, "READY")) {
              zsys_info("Error in transfer, client aborting.");
              zstr_free (&result);
              zmsg_destroy (&msg);
              break;
            }
            if (finished) {
              zsys_info("Happy end.");
              zstr_free (&result);
              zmsg_destroy (&msg);
              break;
            }

            zmsg_t *msg = zmsg_new();
   
            char buf[100];
            int count = fread(buf, 1, sizeof(buf), file);
            if (count > 0) {
              zmsg_addstr(msg, "CHUNK");
              zmsg_addstr(msg, filename_target);
              zmsg_addstrf(msg, "%lu", file_size_so_far);
              zmsg_addstrf(msg, "%u", count);
              zmsg_addstrf(msg, "%u", 0); //TODO: checksum
              zmsg_addmem(msg, buf, count);
              file_size_so_far += count;
            } else {
              zmsg_addstr(msg, "CLOSE");
              zmsg_addstr(msg, filename_target);
              zmsg_addstrf(msg, "%lu", file_size_so_far);
              finished = 1;
            }
            zmsg_print(msg);
            zmsg_send(&msg, client);
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

    zclock_sleep (50000);

    zactor_destroy(&server);
    zactor_destroy(&client);
    zactor_destroy(&client2);

    return 0;
}
