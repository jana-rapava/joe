#include <czmq.h>

#include "fmq_proto.h"

int main () {

    zsys_init();

    zsock_t *client = zsock_new_dealer ("tcp://127.0.0.1:5555");

    // send a HELLO request
    fmq_proto_t* message_ = fmq_proto_new();
    fmq_proto_set_id(message_, FMQ_PROTO_HELLO);
    fmq_proto_set_filename(message_, "/etc/passwd");
    zhash_t* aux_ = zhash_new();
    zhash_insert(aux_, "type", "text");
    fmq_proto_set_aux(message_, &aux_);
    fmq_proto_send(message_, client);

    zclock_sleep(200);

    fmq_proto_t* response_ = fmq_proto_new();
    fmq_proto_recv(response_, client);
    fmq_proto_print(response_);

    fmq_proto_destroy(&message_);
    fmq_proto_destroy(&response_);
    zclock_sleep(1000);


    zsock_destroy (&client);
}
