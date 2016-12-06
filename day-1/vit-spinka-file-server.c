#include <czmq.h>
#include <time.h>


int main(int argc, char **argv) {
    srand(time(NULL));

    zsock_t *server = zsock_new_router ("tcp://172.17.0.3:7777");

    while (!zsys_interrupted) { 
      zmsg_t *msg = zmsg_recv (server);
      zmsg_print(msg);

      zframe_t *routing_id = zmsg_pop(msg);
      char *command_text = zmsg_popstr (msg);
      char *command_filename = zmsg_popstr (msg);
      zmsg_destroy (&msg);
 
      int ok = 1;
      if (strcmp(command_text,"HELLO"))
        ok = 0;

      int r = rand();
      if (r < RAND_MAX*0.5)
        ok = 2; 

      zmsg_t *response = zmsg_new();
      zmsg_add (response, routing_id);
      if (ok == 1) {
        zmsg_addstr(response, "READY");
        zmsg_addstr(response, command_filename);
      } else if (ok ==0)  {
        zmsg_addstr(response, "ERROR");
        zmsg_addstr(response, "Invalid request");
      } else {
        zmsg_addstr(response, "ERROR");
        zmsg_addstr(response, "I don't like you");
      }
      zmsg_send(&response, server);
    }
    
    zsock_destroy (&server);

    return 0;
}


