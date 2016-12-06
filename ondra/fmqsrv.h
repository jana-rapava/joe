#ifndef Joe_FMQSRV_H___
#define Joe_FMQSRV_H___

#include <czmq.h>

void serverActor(
        zsock_t* pipe_,
        void* udata_);

#endif /* Joe_FMQSRV_H___ */
