#include "app.h"
#include "platform.h"
#include <stdio.h>

bool Running = true;

WindowsSpecification specs = {
    .Height = 1920, .Width = 1080, .title = "HollowEngine"};

int main() {

  Init();
  platform_init(&specs);

  while (Running && !platform_shouldclose()) {
    Update(0.016);
    platform_update();
  }

  Shutdown();
  platform_shutdown();

  return 0;
}
