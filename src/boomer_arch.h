#ifndef BOOMER_ARCH_H
#define BOOMER_ARCH_H

#include <stdbool.h>
#include <X11/Xlib.h>
#include <GL/gl.h>
#include "la.h"
#include "navigation.h"

typedef struct
{
  float shadow;
  float radius;
  float delta_radius;
  bool is_enabled;
} Flashlight;

#define INITIAL_FL_DELTA_RADIUS      250.0
#define FL_DELTA_RADIUS_DECELERATION 10.0

void flashlight_update(Flashlight *flashlight, float dt);
vec2f get_cursor_position(Display *display);
Window select_window(Display *display);
int X11_error_handler(Display *display, XErrorEvent *error_event);
void draw(XImage screenshot, Camera camera, GLuint shader, GLuint vao, GLuint texture, vec2f window_size, Mouse mouse, Flashlight flashlight, bool mirror);

#endif // BOOMER_ARCH_H
