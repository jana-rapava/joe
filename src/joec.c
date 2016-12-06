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

#include <czmq.h>
#include <stdio.h>

#include "joe.h"

#define CHUNK_SIZE 1024

static
void sendFile(
        const char* filename_,
        zsock_t* client_) {
    FILE* file_ = fopen(filename_, "rb");
    zchunk_t* chunk_ = zchunk_read(file_, CHUNK_SIZE);
    uint64_t offset_ = 0;
    while(chunk_ != NULL) {
        uint64_t size_ = zchunk_size(chunk_);
        joe_proto_t* message_ = joe_proto_new();
        joe_proto_set_id(message_, JOE_PROTO_CHUNK);
        joe_proto_set_size(message_, size_);
        joe_proto_set_offset(message_, offset_);
        joe_proto_set_filename(message_, filename_);
        joe_proto_set_checksum(message_, 0);
        joe_proto_set_data(message_, &chunk_);
        joe_proto_send(message_, client_);
        joe_proto_destroy(&message_);

        joe_proto_t* response_ = joe_proto_new();
        joe_proto_recv(response_, client_);
        if(joe_proto_id(response_) != JOE_PROTO_READY) {
            joe_proto_destroy(&response_);
            break;
        }
        joe_proto_destroy(&response_);

        offset_ += size_;
        chunk_ = zchunk_read(file_, CHUNK_SIZE);
    }
    fclose(file_);
}

int main (int argc, char *argv [])
{
    bool verbose = false;
    char* url_ = NULL;
    char* file_ = NULL;
    int argn;
    for (argn = 1; argn < argc; argn++) {
        if (streq (argv [argn], "--help")
        ||  streq (argv [argn], "-h")) {
            puts ("joec [options] ...");
            puts ("  --verbose / -v         verbose test output");
            puts ("  --help / -h            this information");
            puts ("  --server url / -s url  specify target server");
            puts ("  --file file / -f file  specify the filename");
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
        else if(streq (argv [argn], "--file")
        ||  streq (argv [argn], "-f")) {
            file_ = strdup(argv[argn + 1]);
            ++argn;
        }
        else {
            printf ("Unknown option: %s\n", argv [argn]);
            return 1;
        }
    }
    //  Insert main code here
    if (verbose)
        zsys_info ("joec - Joe client");

    if(url_ == NULL || file_ == NULL) {
        zsys_error("Specify the target URL and the transfered file!");
        return -1;
    }

    zsys_init();

    zsock_t *client_ = zsock_new_dealer(url_);

    // send a HELLO request
    joe_proto_t* message_ = joe_proto_new();
    joe_proto_set_id(message_, JOE_PROTO_HELLO);
    joe_proto_set_filename(message_, file_);
    zhash_t* aux_ = zhash_new();
    zhash_insert(aux_, "type", "text");
    joe_proto_set_aux(message_, &aux_);
    joe_proto_send(message_, client_);
    joe_proto_destroy(&message_);

    /* -- get the response */
    joe_proto_t* response_ = joe_proto_new();
    joe_proto_recv(response_, client_);
    joe_proto_print(response_);
    if(joe_proto_id(response_) == JOE_PROTO_READY) {
        zsys_info("The server loves me!");
        sendFile(file_, client_);
    }
    else {
        zsys_info("The server hates me!");
    }
    joe_proto_destroy(&response_);

    zclock_sleep(1000);
    zsock_destroy (&client_);

    return 0;
}
