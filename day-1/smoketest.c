#include <czmq.h>

int main(
    int argc_,
    char *argv_[]) {
  zsock_t *push = zsock_new_push("tcp://192.168.1.107:7777");

  zstr_send(push, "Ondra");

  zclock_sleep(1000);
//  string = zstr_recv(pull);
//  puts(string);
//  zstr_free(&string);

//  zsock_destroy(&pull);
  zsock_destroy(&push);

  return 0;
}
