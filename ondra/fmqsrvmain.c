#include <czmq.h>

#include "fmqsrv.h"
#include "ocontrol.h"

int main () {
    zsys_init();

    zactor_t *server_ = zactor_new(serverActor, "server1");

    zsock_t* ctrl_ = zsock_new_router("tcp://0.0.0.0:5000");
    while(!zsys_interrupted) {
        ocontrol_t* ctrlmsg_ = ocontrol_new();
        ocontrol_recv(ctrlmsg_, ctrl_);
        if(ocontrol_id(ctrlmsg_) == OCONTROL_QUIT) {
            break;
        }
    }

    zstr_sendx(server_, "QUIT", NULL);
    zsock_wait(server_);
    zsys_debug("Process ended");
    zactor_destroy(&server_);
    zsock_destroy(&ctrl_);

    return 0;
}
