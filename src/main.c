#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glext.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/cursorfont.h>
#include <X11/extensions/Xrandr.h>
#include <linux/limits.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "app/config.h"
#include "util/la.h"
#include "app/navigation.h"
#include "app/doomer.h"
#include "app/shader.h"
#include "app/backend/X11/X11_screenshot.h"
#include "app/backend/X11/X11_backend.h"

int main(void)
{
  char *config_file = "config.txt";

  bool windowed = false;
  float delay_sec = 0.0;

  // TODO: implement application arguments

  Config def_conf = default_config;

  if (access(config_file, F_OK) == 0)
  {
    def_conf = config_load(config_file);
  }
  else
  {
    printf("%s doesn't exist. Using default values.\n", config_file);
  }

  printf("Using config: \n");
  config_print(def_conf, "default_config");

  long event_mask = ButtonPressMask 
									| ButtonReleaseMask 
									| KeyPressMask 
									| KeyReleaseMask 
									| PointerMotionMask 
									| ExposureMask 
									| ClientMessage;

  XApp *app = XApp_init(&def_conf, "doomer", "doomer", event_mask, windowed);

  app->init                       = XApp_init;
  app->gl_init                    = XApp_gl_init;
  app->draw                       = XApp_draw;
  app->error_handler              = XApp_error_handler;
  app->destroy                    = XApp_destroy;

	app->gl_backend->delete_buffers = gl_delete_buffers;

  bool quitting = false;

  printf("Creating camera.\n");
  Camera *camera = calloc(1, sizeof(Camera));
  camera->scale = 1.0;
  printf("getting cursor position.\n");
  vec2f pos = get_cursor_position(app->display);

  Mouse *mouse = calloc(1, sizeof(Mouse));
  mouse->curr = pos;
  mouse->prev = pos;
  Flashlight *flashlight = calloc(1, sizeof(Flashlight));
  flashlight->is_enabled = false;
  flashlight->radius = 200.0;
  bool mirror = false;

  float dt = 1.0 / app->rate;
  Window origin_window;
  int revert_to_return;
  printf("XGetInputFocus.\n");
  XGetInputFocus(app->display, &origin_window, &revert_to_return);

  while (!quitting)
  {
    if (!windowed)
    {
      XSetInputFocus(app->display, app->window, RevertToParent, CurrentTime);
    }

    XWindowAttributes wa;
    XGetWindowAttributes(app->display, app->window, &wa);
    glViewport(0, 0, wa.width, wa.height);

		printf("camera state: pos: x: %d, y: %d\n", camera->position.x, camera->position.y);
    XEvent event;
    while (XPending(app->display) > 0)
    {
      printf("XPending(display) = %d\n", XPending(app->display));
      printf("Next event.\n");
      XNextEvent(app->display, &event);
			int state = event.xkey.state; // Why state is 0? Its not a app->display

      printf("Switching type.\n");
      switch(event.type)
      {
        case MotionNotify:
				{
          printf("case: MotionNotify.\n");
          mouse->curr = vec2(event.xmotion.x, event.xmotion.y);

          if (mouse->drag)
          {
            vec2f delta = vec2f_sub(world(*camera, mouse->prev), world(*camera, mouse->curr));
            vec2f_add_assign(&camera->position, delta);

            camera->velocity = vec2f_mul_scalar(delta, app->rate);
          }

          mouse->prev = mouse->curr;
          break;
				}
        case ClientMessage:
				{
          printf("case: ClientMessage.\n");
          if ((Atom)(event.xclient.data.l[0]) == *app->delete_message)
          {
            quitting = true;
          }
          break;
				}
        case KeyPress:
        {
          printf("case: KeyPress.\n");
          KeySym key = XLookupKeysym(&event.xkey, 0);
          printf("Key is %lu\n", key);

          switch (key)
          {
						case XK_equal:  
						{
							printf("event.xkey.state: %u\n", state);
							scroll_up(state, ControlMask, flashlight, camera, def_conf, *mouse); 
							break;
						}
            case XK_minus: scroll_down(event.xkey.state, ControlMask, flashlight, camera, def_conf, *mouse); break;
            case XK_0:
						{
              camera->scale = 1.0;
              camera->delta_scale = 0.0;
              camera->position = vec2(0.0, 0.0);
              camera->velocity = vec2(0.0, 0.0);
              mirror = false;
              break;
						}
            case XK_q:
            case XK_Escape:
              quitting = true;
              break;
            // TODO: implement XK_r -- reloading shaders
            // TODO: implement XK_m -- mirroring X11_screenshot
            case XK_f:
						{
              flashlight->is_enabled = !flashlight->is_enabled;
              break;
						}
          }
					break;
        }
        case ButtonPress:
				{
          printf("case: ButtonPress.\n");
          switch(event.xbutton.button)
          {
            case Button1:
						{
              mouse->prev = mouse->curr;
              mouse->drag = true;
              camera->velocity = vec2(0.0, 0.0);
              break;
						}
            case Button4: scroll_up(event.xkey.state, ControlMask, flashlight, camera, def_conf, *mouse); break;
            case Button5: scroll_down(event.xkey.state, ControlMask, flashlight, camera, def_conf, *mouse); break;
          }
          break;
				}
        case ButtonRelease:
				{
          printf("case: ButtonRelease.\n");
          switch(event.xbutton.button)
          {
            case Button1:
              mouse->drag = false;
              break;
          }
          break;
				}
      }
    }

    camera_update(camera, def_conf, dt, *mouse, vec2(wa.width, wa.height));
    flashlight_update(flashlight, dt);

    XApp_draw(app->screenshot->image, 
							*camera, 
							*mouse, 
							*flashlight, 
							app->gl_backend->shader_program, 
							app->gl_backend->vao, 
							app->gl_backend->texture, 
							vec2(wa.width, wa.height), 
							mirror);

    glXSwapBuffers(app->display, app->window);
    glFinish();

#ifdef LIVE
    X11_screenshot_refresh(app->screenshot, app->display, app->tracking_window);

    int w = app->screenshot->image->width;
    int h = app->screenshot->image->height;

    GLfloat vertices[4][5] = 
    {
      {w, 0, 0.0, 1.0, 1.0},
      {w, h, 0.0, 1.0, 0.0},
      {0, h, 0.0, 0.0, 0.0},
      {0, 0, 0.0, 0.0, 1.0}
    };

    GLuint indices[6] = 
    {
      0, 1, 3,
      1, 2, 3
    };

    glBindBuffer(GL_ARRAY_BUFFER, app->gl_backend->vbo);
    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(sizeof(vertices)), vertices, GL_STATIC_DRAW);
    glTexImage2D(GL_TEXTURE_2D, 
                 0, GL_RGB, 
                 app->screenshot->image->width, app->screenshot->image->height,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, app->screenshot->image->data);
#endif
  }


  printf("Freed allocated structures.\n");
  XSetInputFocus(app->display, origin_window, RevertToParent, CurrentTime);
  XSync(app->display, 0);

	//gl_delete_buffers(app->gl_backend);

  free(mouse);
  free(camera);
  free(flashlight);
  XApp_destroy(app);
}
