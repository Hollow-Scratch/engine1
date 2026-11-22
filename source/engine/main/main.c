#include "app.h"
#include <stdio.h>

bool Running = true;

int main() {

  Init();

  while (Running) {
    Update(0.016);
  }

  Shutdown();

  return 0;
}
