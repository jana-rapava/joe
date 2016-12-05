#include <czmq.h>

#include "ocontrol.h"

int main () {

    zsys_init();

    zsock_t *client = zsock_new_dealer ("tcp://127.0.0.1:5000");

    ocontrol_t* message_ = ocontrol_new();
    ocontrol_set_id(message_, OCONTROL_QUIT);
    ocontrol_send(message_, client);
    ocontrol_destroy(&message_);

    zclock_sleep(1000);

    zsock_destroy (&client);
}
