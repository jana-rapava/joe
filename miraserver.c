# include <czmq.h>
# include <signal.h>
# include <unistd.h>

void signal_handler(int sig);
void handle_error(zframe_t *routing_id);

zsock_t *server;

int main(){
    //signal(SIGINT, signal_handler);

    server = zsock_new_router("tcp://192.168.1.184:7777");
    
    while(!zsys_interrupted){
        // server - receive the REQUEST
        zmsg_t *msg = zmsg_recv (server);
        zframe_t *routing_id = zmsg_pop (msg);
        char *hello = zmsg_popstr (msg);
        char *filename = zmsg_popstr (msg);
        
        zmsg_print (msg);

        if (streq(hello, "HELLO")){
            zmsg_t *response = zmsg_new ();
            zmsg_add (response, routing_id);
            zmsg_addstr (response, "READY");
            zmsg_send (&response, server);
        }
        else{
            handle_error(routing_id);
        }

        zmsg_destroy (&msg);

    }
}

void handle_error(zframe_t *routing_id){
    zmsg_t *response = zmsg_new ();
    zmsg_add (response, routing_id);
    zmsg_addstr (response, "ERROR");
    zmsg_send (&response, server);
}

void signal_handler(int sig){
    if (sig == SIGINT){
        printf("DYING\n");
        zsock_destroy(&server);
        exit(0);
    }
}
