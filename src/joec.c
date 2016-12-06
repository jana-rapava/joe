/*  =========================================================================
    joec - Joe client
    Copyright (c) the Contributors as noted in the AUTHORS file.       
    This file is part of CZMQ, the high-level C binding for 0MQ:       
    http://czmq.zeromq.org.                                            
                                                                       
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.           
    =========================================================================
*/

/*
@header
    joec - Joe client
@discuss
@end
*/

#include "joe_classes.h"

static void s_actor_client(zsock_t *pipe, void *args) {
    char *name = strdup ((char*) args);
    zsock_t *client = zsock_new_dealer ("tcp://192.168.1.153:5000");
    zpoller_t *poller = zpoller_new (pipe, client, NULL);

    // to signal to runtime it should spawn the thread
    zsock_signal (pipe, 0);
    zsys_debug ("%s:\tstarted", name);
    char *filename_full = "/etc/passwd";
    char *filename_target = "passwd";
    FILE *file = fopen (filename_full, "r");
    assert (file);
    unsigned long file_size_so_far = 0;
    unsigned int checksum = 0;
    int finished = 0;

    joe_proto_t *msg = joe_proto_new ();
    joe_proto_set_id (msg, JOE_PROTO_HELLO);
    joe_proto_set_filename (msg, filename_target);
 
    joe_proto_print (msg);
    joe_proto_send (msg, client);
    //TODO: destroy?

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
            joe_proto_recv (msg, client);
            //zsys_info ("Received this message:");
            joe_proto_print (msg);
            int result = joe_proto_id (msg);
            if (result != JOE_PROTO_READY) {
              zsys_info("Error in transfer, client aborting.");
              joe_proto_destroy (&msg);
              break;
            }
            if (finished) {
              zsys_info("Happy end.");
              joe_proto_destroy (&msg);
              break;
            }

            joe_proto_t *msg = joe_proto_new();
   
            char buf[100];
            unsigned int count = fread (buf, 1, sizeof(buf), file);
            zsys_info ("Read %u bytes from file\n", count);
            zchunk_t *filedata = zchunk_new ((const void *) buf, count);
            if (count > 0) {
              joe_proto_set_id (msg, JOE_PROTO_CHUNK);
              joe_proto_set_filename (msg, filename_target);
              joe_proto_set_offset (msg, scanf ("%lu", &file_size_so_far));
              joe_proto_set_size (msg, scanf ("%u", &count));
              joe_proto_set_checksum (msg, scanf ("%u", &checksum)); //TODO: checksum
              joe_proto_set_data (msg, &filedata);
              file_size_so_far += count;
            } else {
              joe_proto_set_id (msg, JOE_PROTO_CLOSE);
              joe_proto_set_filename (msg, filename_target);
              joe_proto_set_size (msg, scanf ("%lu", &file_size_so_far));
              finished = 1;
            }
            joe_proto_print (msg);
            joe_proto_send (msg, client);
            zchunk_destroy (&filedata);
        }
    }

    joe_proto_destroy (&msg);
    zsock_destroy (&client);
}

int main (int argc, char *argv [])
{
    bool verbose = false;
    int argn;
    for (argn = 1; argn < argc; argn++) {
        if (streq (argv [argn], "--help")
        ||  streq (argv [argn], "-h")) {
            puts ("joec [options] ...");
            puts ("  --verbose / -v         verbose test output");
            puts ("  --help / -h            this information");
            return 0;
        }
        else
        if (streq (argv [argn], "--verbose")
        ||  streq (argv [argn], "-v"))
            verbose = true;
        else {
            printf ("Unknown option: %s\n", argv [argn]);
            return 1;
        }
    }
    //  Insert main code here
    if (verbose)
        zsys_info ("joec - Joe client");

    zactor_t *client = zactor_new (s_actor_client, "joec");
    zclock_sleep (5000);
    zactor_destroy(&client);

    return 0;
}
