#include <malamute.h>

int main () {

    const char *endpoint = "ipc://@aaaaaa";
    zactor_t *server = zactor_new (mlm_server, "Malamute");
    zstr_sendx (server, "BIND", endpoint, NULL);

    mlm_client_t *producer = mlm_client_new ();
    mlm_client_connect (producer, endpoint, 1000, "producer");
    mlm_client_set_producer (producer, "STREAM");

    mlm_client_t *consumer = mlm_client_new ();
    mlm_client_connect (consumer, endpoint, 1000, "consumer");
    mlm_client_set_consumer (consumer, "STREAM", ".*");

    for (int i = 0; i != 10; i++)
        mlm_client_sendx (producer, "SUBJECT", "HELLO", "WORLD", NULL);

    for (int i = 0; i != 10; i++) {
        zmsg_t *msg = mlm_client_recv (consumer);
        zsys_info ("subject=%s", mlm_client_subject (consumer));
        zsys_info ("sender=%s", mlm_client_address (consumer));
        zmsg_print (msg);
        zmsg_destroy (&msg);
    }

    mlm_client_destroy (&consumer);
    mlm_client_destroy (&producer);
    zactor_destroy (&server);

}
