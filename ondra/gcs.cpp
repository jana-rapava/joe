#include <czmq.h>
#include <zyre.h>

static void gcs_actor(
    zsock_t* pipe_,
    void* udata_) {
  zyre_t* node_ = zyre_new("ondra");
  zyre_set_verbose(node_);
  zyre_print(node_);

  if(zyre_start(node_) < 0) {
    zsys_error("Cannot start the node");
    return;
  }

  zsys_debug("node is running");
  zsock_signal(pipe_, 0);

  zyre_join(node_, "ZMQ");

  zpoller_t* poller_ = zpoller_new (pipe_, zyre_socket(node_), NULL);

  /* -- watch for the events */
  while(!zsys_interrupted) {
    void* which_ = zpoller_wait(poller_, 5000);
    if (which_ == NULL) {
      if (zpoller_terminated (poller_)) {
        break;
      }
      if(zpoller_expired(poller_)) {
        zsys_debug("shout");
        zyre_shouts(node_, "ZMQ", "Hello world");
      }
    }
    else if(which_ == pipe_) {
      break;
    }
    else if(which_ == zyre_socket(node_)) {
      zsys_debug("zyre packet");
      zyre_event_t* event_ = zyre_event_new(node_);
      zyre_event_print(event_);
      zyre_event_destroy(&event_);
    }
  }

  zyre_stop(node_);
  zyre_destroy(&node_);
  zpoller_destroy(&poller_);
}

int main(
    int argc_,
    char* argv_[]) {

  zactor_t *server_ = zactor_new(gcs_actor, NULL);

  zclock_sleep(60 * 1000);

  zactor_destroy(&server_);

  return 0;
}



