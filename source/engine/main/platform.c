#include "platform.h"
#include <GLFW/glfw3.h>
#include <stdio.h>

bool platform_init(WindowsSpecification *WindowsSpecs) {

  if (!glfwInit()) {
    printf("GLFW init error\n");
    return -1;
  };

  window = glfwCreateWindow(WindowsSpecs->Height, WindowsSpecs->Width,
                            WindowsSpecs->title, NULL, NULL);

  if (!window) {
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  return true;
}

void platform_update() {
  glfwSwapBuffers(window);
  glfwPollEvents();
}

void platform_shutdown() { glfwTerminate(); }
