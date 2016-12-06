/*  =========================================================================
    joe_server - class description

    Copyright (c) the Contributors as noted in the AUTHORS file.       
    This file is part of CZMQ, the high-level C binding for 0MQ:       
    http://czmq.zeromq.org.                                            
                                                                       
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.           
    =========================================================================
*/

#ifndef JOE_SERVER_H_INCLUDED
#define JOE_SERVER_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include <joe.h>

typedef struct {
    const char* actor_name;
    const char* bind_url;
} JoeServerActorParams;

/**
 * @brief Actor of the server
 */
JOE_EXPORT void joe_server_actor(
        zsock_t* pipe_,
        void* udata_);

//  @interface
JOE_EXPORT
void
joe_server_test (bool verbose);

//  @end

#ifdef __cplusplus
}
#endif

#endif
