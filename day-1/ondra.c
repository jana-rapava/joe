#include <czmq.h>

int main(
    int argc_,
    char *argv_[]) {
  /* -- nice comment */
  zsock_t *push = zsock_new_push("tcp://192.168.1.107:7777");
  zstr_send(push, "Ondra");
  zclock_sleep(1000);
  zsock_destroy(&push);

  return 0;
}
