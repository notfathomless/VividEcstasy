#pragma once

// GLFW_EXPOSE_NATIVE_WAYLAND GLFW_EXPOSE_NATIVE_X11
// #define GLFW_EXPOSE_NATIVE_WAYLAND

/* #define GLFW_EXPOSE_NATIVE_X11
#define GLFW_EXPOSE_NATIVE_GLX */

#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL

#include <GLFW/glfw3.h>
// #define GLFW_NATIVE_INCLUDE_NONE
#include <GLFW/glfw3native.h>