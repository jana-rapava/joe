/*  =========================================================================
    joed - Joe daemon

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
    joed - Joe daemon
@discuss
@end
*/

#include <czmq.h>
#include "joe_classes.h"

int main (int argc, char *argv [])
{
    bool verbose = false;
    char* url_ = NULL;
    int argn;
    for (argn = 1; argn < argc; argn++) {
        if (streq (argv [argn], "--help")
        ||  streq (argv [argn], "-h")) {
            puts ("joed [options] ...");
            puts ("  --verbose / -v         verbose test output");
            puts ("  --help / -h            this information");
            puts ("  --server url / -s url  specify binding url");
            return 0;
        }
        else
        if (streq (argv [argn], "--verbose")
        ||  streq (argv [argn], "-v"))
            verbose = true;
        else if(streq (argv [argn], "--server")
        ||  streq (argv [argn], "-s")) {
            url_ = strdup(argv[argn + 1]);
            ++argn;
        }
        else {
            printf ("Unknown option: %s\n", argv [argn]);
            return 1;
        }
    }

    if(url_ == NULL) {
        zsys_error("Specify the binding URL");
        return -1;
    }

    //  Insert main code here
    if (verbose)
        zsys_info ("joed - Joe daemon");


    zsys_init();

    JoeServerActorParams params_ = {"server1", url_};
    zactor_t *server_ = zactor_new(joe_server_actor, &params_);

    zclock_sleep(10000);

    zstr_sendx(server_, "QUIT", NULL);
    zsock_wait(server_);
    zsys_debug("Process ended");
    zactor_destroy(&server_);
    free(url_);

    return 0;
}
