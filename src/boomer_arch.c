#include <stdlib.h>
#define GL_GLEXT_PROTOTYPES
#include "boomer_arch.h"
#include "navigation.h"
#include <GL/gl.h>
#include <GL/glext.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/cursorfont.h>
#include <math.h>
#include <stdio.h>

const float FL_DELTA_RADIUS_DECELERATION_C = FL_DELTA_RADIUS_DECELERATION;

void flashlight_update(Flashlight *flashlight, float dt)
{
  printf("updating flashlight.\n");
  if (fabs(flashlight->delta_radius) > 1.0)
  {
    flashlight->radius = fmax(0.0, flashlight->radius + flashlight->delta_radius * dt);
    flashlight->delta_radius -= flashlight->delta_radius * FL_DELTA_RADIUS_DECELERATION_C * dt;
  }
  if (flashlight->is_enabled)
    flashlight->shadow = fmin(flashlight->shadow + 6.0 * dt, 0.8);
  else 
  {
    flashlight->shadow = fmax(flashlight->shadow - 6.0 * dt, 0.0);
  }
}

vec2f get_cursor_position(Display *display)
{
  Window *root = malloc(sizeof(Window)), *child = malloc(sizeof(Window));
  int *root_x = malloc(sizeof(int)), *root_y = malloc(sizeof(int)), *win_x = malloc(sizeof(int)), *win_y = malloc(sizeof(int));
  unsigned int *mask = malloc(sizeof(unsigned int));

  XQueryPointer(display, DefaultRootWindow(display), 
                root, child, root_x, root_y, win_x, win_y, mask);

  free(root);
  free(child);
  free(root_x);
  free(root_y);
  free(win_x);
  free(win_y);
  free(mask);
  return vec2((float)*root_x, (float)*root_y);
}

Window select_window(Display *display)
{
  Cursor cursor = XCreateFontCursor(display, XC_crosshair);
  XFreeCursor(display, cursor);

  Window root = DefaultRootWindow(display);
  XGrabPointer(display, root, 0, ButtonMotionMask | ButtonPressMask | ButtonReleaseMask, GrabModeAsync, GrabModeAsync, root, cursor, CurrentTime);
  XUngrabPointer(display, CurrentTime);

  XGrabKeyboard(display, root, 0, GrabModeAsync, GrabModeAsync, CurrentTime);
  XUngrabKeyboard(display, CurrentTime);

  XEvent *event = NULL;

  while (true)
  {
    XNextEvent(display, event);

    switch (event->type)
    {
      case ButtonPress:
        return event->xbutton.subwindow;
      case KeyPress:
        return root;
      default:
        return 0L;
    }
  }

  return root;
}

int X11_error_handler(Display *display, XErrorEvent *error_event)
{
  char text[256], request[64], num[16];

  XGetErrorText(display, error_event->error_code, text, sizeof(text));
  snprintf(num, sizeof(num), "%d", error_event->error_code);
  XGetErrorDatabaseText(display, "XRequest", num, "unknown", request, sizeof(request));
    fprintf(stderr, "X11 ERROR: %s | request=%s (opcode %d, minor %d) | resource=0x%lx | serial=%lu\n",
          text, request, error_event->request_code, error_event->minor_code, error_event->resourceid, error_event->serial);
  return 0;
}

void draw(XImage screenshot, Camera camera, GLuint shader, GLuint vao, GLuint texture, vec2f window_size, Mouse mouse, Flashlight flashlight, bool mirror)
{
  glClearColor(0.1, 0.1, 0.1, 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  printf("shader_program: %d\n", shader);
  printf("camera Position: x = %f, y = %f\n", camera.position.x, camera.position.y);
  printf("window_size: x = %f, y = %f\n", window_size.x, window_size.y);
  printf("screenshot_size: width = %d, height = %d\n", screenshot.width, screenshot.height);
  printf("cursor_pos: x = %f, y = %f\n", mouse.curr.x, mouse.curr.y);
  glUseProgram(shader);

  printf("uniform location (tex): %d\n", glGetUniformLocation(shader, "tex"));
  printf("uniform location (cameraPos): %d\n", glGetUniformLocation(shader, "cameraPos"));
  printf("uniform location (cameraScale): %d\n", glGetUniformLocation(shader, "cameraScale"));
  printf("uniform location (screenshotSize): %d\n", glGetUniformLocation(shader, "screenshotSize"));
  printf("uniform location (windowSize): %d\n", glGetUniformLocation(shader, "windowSize"));
  printf("uniform location (cursorPos): %d\n", glGetUniformLocation(shader, "cursorPos"));
  printf("uniform location (flShadow): %d\n", glGetUniformLocation(shader, "flShadow"));
  printf("uniform location (flRadius): %d\n", glGetUniformLocation(shader, "flRadius"));

  glUniform1i(glGetUniformLocation(shader, "tex"), 0);
  glUniform2f(glGetUniformLocation(shader, "cameraPos"), camera.position.x, camera.position.y);
  glUniform1f(glGetUniformLocation(shader, "cameraScale"), camera.scale);
  glUniform2f(glGetUniformLocation(shader, "screenshotSize"), screenshot.width, screenshot.height);
  glUniform2f(glGetUniformLocation(shader, "windowSize"), window_size.x, window_size.y);
  glUniform2f(glGetUniformLocation(shader, "cursorPos"), mouse.curr.x, mouse.curr.y);
  glUniform1f(glGetUniformLocation(shader, "flShadow"), flashlight.shadow);
  glUniform1f(glGetUniformLocation(shader, "flRadius"), flashlight.radius);
  glUniform1f(glGetUniformLocation(shader, "mirror"), mirror ? 1 : 0);

  glBindVertexArray(vao);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
}
