#include <czmq.h>

// server actor
static
void s_actor1 (zsock_t *pipe, void *args)
{
    zsock_t *svr = zsock_new_router ("inproc://test");
    
    char *name = strdup ((char*) args);
    zpoller_t *poller = zpoller_new (pipe, svr ,NULL);

    // to signal to runtime it should spawn the thread
    zsock_signal (pipe, 0);
    zsys_debug ("\t%s:started", name);

    while (!zsys_interrupted) {

        void *which = zpoller_wait (poller, -1);

        if (!which)
            break;
        
        if (which == pipe)
        {
            zmsg_t *msg = zmsg_recv (pipe);
            char *command = zmsg_popstr (msg);
            zsys_info ("Got API command=%s", command);

            if (streq (command, "$TERM"))
            {                
                zstr_free (&command);
                zmsg_destroy (&msg);
                break;
            }                       
        }     
        if (which == svr)
        {
            zmsg_t *msg_subj = zmsg_recv (svr);
            zframe_t *r_id = zmsg_pop (msg_subj);
            char *command_subj = zmsg_popstr (msg_subj);
            zsys_info ("Got API command=%s", command_subj);

            if (streq (command_subj, "HELLO"))
            {
                zmsg_t *msg = zmsg_new ();
                zmsg_add (msg, r_id);
                zmsg_addstr (msg, "READY");
                zmsg_send (&msg, svr);

            }                
            else
            {
                zmsg_t *msg = zmsg_new ();
                zmsg_add (msg, r_id);
                zmsg_addstr (msg, "ERROR");
                zmsg_send (&msg, svr);
            }
            zstr_free (&command_subj);
            zmsg_destroy (&msg_subj);
        }
        zclock_sleep (1000);                
    }
    zsock_destroy (&svr);
    zpoller_destroy (&poller);
    zstr_free (&name);
}

// client
static
void s_actor2 (zsock_t *pipe, void *args)
{
    zsock_t *client = zsock_new_dealer ("inproc://test");
    char *name = strdup ((char*) args);
    zpoller_t *poller = zpoller_new (pipe, client, NULL);

    // to signal to runtime it should spawn the thread
    zsock_signal (pipe, 0);
    zsys_debug ("\t%s:started", name);

    zmsg_t *send = zmsg_new ();
    zmsg_addstr (send, "HELLO");
    zmsg_addstr (send, "/etc/password");        
    zmsg_send (&send, client);
    printf("sending HELLO\n");
 
    while (!zsys_interrupted)
    {
        void *which = zpoller_wait (poller, -1);

        if (!which)
            break;
        
        if (which == pipe)
        {
            zmsg_t *msg = zmsg_recv (pipe);
            char *command = zmsg_popstr (msg);
            zsys_info ("Got API command=%s", command);

            if (streq (command, "$TERM"))
            {                
                zstr_free (&command);
                zmsg_destroy (&msg);
                break;
            }                       
        }     
        if (which == client)
        {
            zmsg_t *msg_subj = zmsg_recv (client);
            char *command_subj = zmsg_popstr (msg_subj);
            zsys_info ("Got API command=%s", command_subj);

            //everything is ok so send the file
            if (streq (command_subj, "READY"))
            {               
                zmsg_t *msg = zmsg_new ();
                zmsg_addstr (msg, "CHUNK");
                zmsg_addstr (msg, "filename");
                zmsg_addstr (msg, "offset");                
                zmsg_send (&msg, client);

            }
            else if (streq (command_subj, "ERROR"))
            {               
                zmsg_t *msg = zmsg_new ();
                zmsg_addstr (msg, " I am not going to send anything");
                zmsg_send (&msg, client);
                break;
            }
            else
            {
                zsys_debug ("%s invalid subject ", name);
            }
                zclock_sleep (2000);                
        }  
    }
    zsock_destroy (&client);
    zpoller_destroy (&poller);
    zstr_free (&name);
}


int main () {

    zactor_t *server = zactor_new (s_actor1, "actor1");
    zclock_sleep (2000);
    zactor_t *client = zactor_new (s_actor2, "actor2");

    // zstr_sendx (server, "HELLO", NULL);
    //    zstr_sendx (server, "HELLO", "WORLD", NULL);

 
     
    zclock_sleep (5000);
    zactor_destroy (&client);
    zactor_destroy (&server);

}
