#include <czmq.h>

#include <cstdlib>
#include <datstr/dstring.h>
#include <iostream>
#include <ondrart/ogetopt2.h>
#include "ocontrol.h"

namespace {

int parseCommandLine(
    int argc_,
    char* argv_[]) {
  OGetopt2 parser_("ctrl");

  parser_.addVersion("ctrl");
  parser_.addEmptyLine();
  parser_.addText("FMQ server control tool");
  parser_.addEmptyLine();

  parser_.addHeadline("Usage:");
  parser_.addBrief("command");
  parser_.addEmptyLine();

  parser_.addHeadline("Options:");
  parser_.addStandardOptions();
  parser_.addEmptyLine();

  parser_.addHeadline("Commands:");
  parser_.addExplanation("quit", "Shutdown the server.");
  parser_.addEmptyLine();

  int code_;
  while(-1 != (code_ = parser_.getOptStd(argc_, argv_))) {
    switch(code_) {
      default:
        parser_.printUsage();
        std::exit(-1);
    }
  }

  return parser_.getOptind();
}

} /* -- namespace */

int main(
        int argc_,
        char *argv_[]) {
    int optind_(parseCommandLine(argc_, argv_));
    dstring command_(argv_[optind_]);

    zsys_init();

    if(command_ == "quit") {
        zsock_t *client = zsock_new_dealer ("tcp://127.0.0.1:5000");
        ocontrol_t* message_ = ocontrol_new();
        ocontrol_set_id(message_, OCONTROL_QUIT);
        ocontrol_send(message_, client);
        ocontrol_destroy(&message_);
        zclock_sleep(1000);

        zsock_destroy (&client);
    }
    else {
        std::cerr << "Invalid command: " << command_ << std::endl;
    }

    return 0;
}
