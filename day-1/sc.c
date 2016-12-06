#include <czmq.h>

void
send_file (zsock_t *client)
{
    
    zmsg_t *file = zmsg_new ();
    zmsg_addstr (file, "1");        
    zmsg_addstr (file, " here is your file :-)");    
    zmsg_send (&file, client);
}


int main () {

    zsock_t *client = zsock_new_dealer ("tcp://192.168.1.184:7777");

    while (!zsys_interrupted)
    {
        // send message to the server
        zmsg_t *msg = zmsg_new ();
        zmsg_addstr (msg, "HELLO");
        zmsg_addstr (msg, "/etc/password");        
        zmsg_send (&msg, client);

        // receive response
        zmsg_t *client_response = zmsg_recv (client);        
        zclock_sleep (1000); // ms
        zmsg_print (client_response);

        // process response from server
        char *svrresp = zmsg_popstr (client_response);

        if (streq (svrresp, "READY"))
        {
            // server is ok send "file"
            send_file (client);
            zclock_sleep (1000); 
            
        }
        else
        if (streq (svrresp, "ERROR"))    
        {
            zsys_debug ("...ERROR...");
        } else
        {
            zsys_debug ("... invalid protocol ...");
        }
            
        zmsg_destroy (&client_response);        
    }
            
    zsock_destroy (&client);
    
}
