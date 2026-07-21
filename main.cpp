#include "controller/app.h"
#include "utility/exceptions.h"
#include <iostream>

int main() {

  try {
    app application;
    application.run();
  }  catch (const std::exception &e) {
    std::cerr << "Unexpected error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
