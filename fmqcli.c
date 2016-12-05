#include <czmq.h>

#include "joe_proto.h"

int main () {

    zsys_init();

    zsock_t *client = zsock_new_dealer ("tcp://127.0.0.1:5555");

    // send a HELLO request
    joe_proto_t* message_ = joe_proto_new();
    joe_proto_set_id(message_, JOE_PROTO_HELLO);
    joe_proto_set_filename(message_, "/etc/passwd");
    zhash_t* aux_ = zhash_new();
    zhash_insert(aux_, "type", "text");
    joe_proto_set_aux(message_, &aux_);
    joe_proto_send(message_, client);

    zclock_sleep(200);

    joe_proto_t* response_ = joe_proto_new();
    joe_proto_recv(response_, client);
    joe_proto_print(response_);

    joe_proto_destroy(&message_);
    joe_proto_destroy(&response_);

    zclock_sleep(1000);

    zsock_destroy (&client);
}
