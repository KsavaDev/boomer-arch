#ifndef BOOMER_ARCH_H
#define BOOMER_ARCH_H

#include <stdbool.h>
#include <X11/Xlib.h>
#include "la.h"

typedef struct
{
  float shadow;
  float radius;
  float delta_radius;
  bool is_enabled;
} Flashlight;

// atp I cant precisely answer what I should use: const float or #define
const float INITIAL_FL_DELTA_RADIUS      = 250.0;
const float FL_DELTA_RADIUS_DECELERATION = 10.0;

void update(Flashlight *flashlight, float dt);
vec2f get_cursor_position(Display *display);
Window select_window(Display *display);
int X11_error_handler(Display *display, XErrorEvent *error_event);

#endif // BOOMER_ARCH_H
