/*  =========================================================================
    joe - Joe example server

    Copyright (c) the Contributors as noted in the AUTHORS file.       
    This file is part of CZMQ, the high-level C binding for 0MQ:       
    http://czmq.zeromq.org.                                            
                                                                       
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.           
    =========================================================================
*/

#ifndef JOE_H_H_INCLUDED
#define JOE_H_H_INCLUDED

#define JOE_SERVER_TEST_SERVICE_URL "tcp://127.0.0.1:5555"

//  Include the project library file
#include "joe_library.h"


// joe's server
void
joes_server (zsock_t *pipe, void *args);

//  Add your own public definitions here, if you need them

#endif
