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
#include "config.h"
#include "la.h"
#include "navigation.h"
#include "doomer.h"
#include "shader.h"
#include "screenshot.h"

static void scroll_up(XEvent event, Flashlight *flashlight, Camera *camera, Config config, Mouse mouse)
{
  printf("Scrolling up.\n");
  if ((event.xkey.state & ControlMask) > 0 && flashlight->is_enabled)
  {
    printf("changing flashlight delta_radius.\n");
    flashlight->delta_radius += INITIAL_FL_DELTA_RADIUS;
  }
  else 
  {
    printf("changing camera delta_scale and scale_pivot.\n");
    camera->delta_scale += config.scroll_speed;
    camera->scale_pivot = mouse.curr;
  }
  printf("Done scrolling up.\n");
}

static void scroll_down(XEvent event, Flashlight *flashlight, Camera *camera, Config config, Mouse mouse)
{
  if ((event.xkey.state & ControlMask) > 0 && flashlight->is_enabled)
  {
    flashlight->delta_radius -= INITIAL_FL_DELTA_RADIUS;
  }
  else 
  {
    camera->delta_scale -= config.scroll_speed;
    camera->scale_pivot = mouse.curr;
  }
}

int main(void)
{
  char boomer_dir[PATH_MAX];

  if (getcwd(boomer_dir, sizeof(boomer_dir)) != NULL)
  {
    printf("Current directory: %s\n", boomer_dir);
  }
  else 
  {
    perror("getcwd() error");
    exit(1);
  }

  char *src_dir = strcat(boomer_dir, "/src");

  printf("src dir: %s\n", src_dir);

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

  Display *display = XOpenDisplay(NULL);
  if (display == NULL)
  {
    fprintf(stderr, "Failed to open display\n");
    exit(1);
  }

  XSetErrorHandler(X11_error_handler);

  // TODO: implement switching tracking_window by select.
  Window tracking_window = DefaultRootWindow(display);

  XRRScreenConfiguration *screen_config = XRRGetScreenInfo(display, DefaultRootWindow(display));

  short rate = XRRConfigCurrentRate(screen_config);
  printf("Screen rate: %d\n", rate);

  int screen = XDefaultScreen(display);
  
  int *maj = malloc(sizeof(int));
  int *min = malloc(sizeof(int));

	if (glXQueryVersion(display, maj, min))
	{
		if ((*maj == 1 && *min <3) || (*maj < 1))
    {
      fprintf(stderr, "Invalid GLX version. Required >=1.3\n");
    }
		printf("GLX Version: %d.%d\n", *maj, *min);
	}

	printf("%s\n", glXQueryExtensionsString(display, screen));

  free(maj);
  free(min);

  int attrs[] = 
  {
    GLX_RGBA,
    GLX_DEPTH_SIZE, 24,
    GLX_DOUBLEBUFFER,
    None
  };

  XVisualInfo *visual_info = glXChooseVisual(display, screen, attrs);
  if (visual_info == NULL)
  {
    fprintf(stderr, "No apporpriate visual found");
    exit(1);
  }

  printf("Visual %lu selected\n", visual_info->visualid);

  printf("Initializing XSetWindowAttributes.\n");
  XSetWindowAttributes *set_window_attributes = malloc(sizeof(XSetWindowAttributes));

  set_window_attributes->colormap = XCreateColormap(display, DefaultRootWindow(display), visual_info->visual, AllocNone);

  set_window_attributes->event_mask = ButtonPressMask | ButtonReleaseMask | KeyPressMask | KeyReleaseMask | PointerMotionMask | ExposureMask | ClientMessage;

  if (!windowed)
  {
    set_window_attributes->override_redirect = 1;
    set_window_attributes->save_under = 1;
  }

  printf("Initializing XWindowAttributes.\n");
  XWindowAttributes *window_attributes = malloc(sizeof(XWindowAttributes));
  XGetWindowAttributes(display, DefaultRootWindow(display), window_attributes);

  printf("Creating Window.\n");
  Window window = XCreateWindow(display, DefaultRootWindow(display),
                                 0, 0, 
                                 window_attributes->width, window_attributes->height, 0,
                                 visual_info->depth, InputOutput, visual_info->visual,
                                 CWColormap | CWEventMask | CWOverrideRedirect | CWSaveUnder,
                                 set_window_attributes);
  
  printf("Creating new screenshot.\n");
  Screenshot *screenshot = screenshot_new(display, tracking_window);
  printf("Done creating new screenshot.\n");

  printf("Mapping Window.\n");
  XMapWindow(display, window);

  char *name = "doomer";
  char *class = "doomer";

  printf("Creating XClassHint and setting it.\n");
  XClassHint hints = {.res_name = name, .res_class = class};

  XStoreName(display, window, name);
  XSetClassHint(display, window, &hints);

  printf("Creating Atom delete_message.\n");
  Atom delete_message = XInternAtom(display, "WM_DELETE_WINDOW", 0);

  printf("Setting WM protocols.\n");
  XSetWMProtocols(display, window, &delete_message, 1);

  printf("Creating GLX Context.\n");
  GLXContext context = glXCreateContext(display, visual_info, NULL, GL_TRUE);
  glXMakeCurrent(display, window, context);

  printf("Reading shader files.\n");
  Shader *vertex_shader = shader_read("src/vert.glsl");
  Shader *fragment_shader = shader_read("src/frag.glsl");
  printf("Done reading shader files.\n");

  printf("Content of vertex_shader:\n");
  printf("%s\n", vertex_shader->content);

  printf("Content of fragment_shader:\n");
  printf("%s\n", fragment_shader->content);

  printf("Creating new shader_program.\n");
  GLuint shader_program = shader_new_program(vertex_shader, fragment_shader);
  

  int w = screenshot->image->width;
  int h = screenshot->image->height;

  printf("Creating vao, vbo, ebo.\n");
  GLuint *vao = malloc(sizeof(GLuint)), *vbo = malloc(sizeof(GLuint)), *ebo = malloc(sizeof(GLuint));

  printf("Creating vertices_foo.\n");
  GLfloat vertices_foo[4][5] = 
  {
    {w, 0, 0.0, 1.0, 1.0},
    {w, h, 0.0, 1.0, 0.0},
    {0, h, 0.0, 0.0, 0.0},
    {0, 0, 0.0, 0.0, 1.0}
  };

  printf("Creating indices.\n");
  GLuint indices[6] = 
  {
    0, 1, 3,
    1, 2, 3
  };

  printf("================ OpenGL ================\n");
  printf("Gen vertex arrays.\n");
  glGenVertexArrays(1, vao);
  printf("Gen vbo buffers.\n");
  glGenBuffers(1, vbo);
  printf("Gen ebo buffers.\n");
  glGenBuffers(1, ebo);

  printf("Binding vao.\n");
  glBindVertexArray(*vao);

  printf("Binding array buffer vbo.\n");
  glBindBuffer(GL_ARRAY_BUFFER, *vbo);
  printf("Binding buffer data vertices_foo.\n");
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_foo), vertices_foo, GL_STATIC_DRAW);

  printf("Binding array buffer ebo.\n");
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *ebo);
  printf("Binding buffer data indices.\n");
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

  printf("Creating stride.\n");
  GLsizei stride = (GLsizei)(5 * sizeof(GLfloat));

  GLint aPos_location = glGetAttribLocation(shader_program, "aPos");
  printf("aPos_location: %d\n", aPos_location);
  GLint aTexCoord_location = glGetAttribLocation(shader_program, "aTexCoord");
  printf("aTexCoord_location: %d\n", aTexCoord_location);

  printf("Vertex attrib pointer (0).\n");
  glVertexAttribPointer(aPos_location, 2, GL_FLOAT, false, stride, (pointer)(0));
  printf("Enable vertex attrib pointer (0).\n");
  glEnableVertexAttribArray(aPos_location);

  printf("Vertex attrib pointer (1).\n");
  glVertexAttribPointer(aTexCoord_location, 2, GL_FLOAT, false, stride, (pointer)(3 * sizeof(GLfloat)));
  printf("Enable vertex attrib pointer (1).\n");
  glEnableVertexAttribArray(aTexCoord_location);

  GLuint texture = (GLuint)0;
  
  glGenTextures(1, &texture);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 
               screenshot->image->width, screenshot->image->height, 
               0, GL_BGRA, GL_UNSIGNED_BYTE, screenshot->image->data);

  glGenerateMipmap(GL_TEXTURE_2D);


  glEnable(GL_TEXTURE_2D);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

  bool quitting = false;

  printf("Creating camera.\n");
  Camera *camera = calloc(1, sizeof(Camera));
  camera->scale = 1.0;
  printf("getting cursor position.\n");
  vec2f pos = get_cursor_position(display);

  Mouse *mouse = calloc(1, sizeof(Mouse));
  mouse->curr = pos;
  mouse->prev = pos;
  Flashlight *flashlight = calloc(1, sizeof(Flashlight));
  flashlight->is_enabled = false;
  flashlight->radius = 200.0;
  bool mirror = false;

  float dt = 1.0 / rate;
  Window origin_window;
  int revert_to_return;
  printf("XGetInputFocus.\n");
  XGetInputFocus(display, &origin_window, &revert_to_return);
  screenshot_save_to_ppm(*screenshot->image, "screenshot.ppm");

  while (!quitting)
  {
    if (!windowed)
    {
      XSetInputFocus(display, window, RevertToParent, CurrentTime);
    }

    XWindowAttributes wa;
    XGetWindowAttributes(display, window, &wa);
    glViewport(0, 0, wa.width, wa.height);

    XEvent event;
    while (XPending(display) > 0)
    {
      printf("XPending(display) = %d\n", XPending(display));
      printf("Next event.\n");
      XNextEvent(display, &event);

      printf("Switching type.\n");
      switch(event.type)
      {
        case MotionNotify:
          printf("case: MotionNotify.\n");
          mouse->curr = vec2(event.xmotion.x, event.xmotion.y);

          if (mouse->drag)
          {
            vec2f delta = vec2f_sub(world(*camera, mouse->prev), world(*camera, mouse->curr));
            vec2f_add_assign(&camera->position, delta);

            camera->velocity = vec2f_mul_scalar(delta, rate);
          }

          mouse->prev = mouse->curr;
          break;
        case ClientMessage:
          printf("case: ClientMessage.\n");
          if ((Atom)(event.xclient.data.l[0]) == delete_message)
          {
            quitting = true;
          }
          break;
        case KeyPress:
        {
          printf("case: KeyPress.\n");
          KeySym key = XLookupKeysym(&event.xkey, 0);
          printf("Key is %lu\n", key);

          switch (key)
          {
            case XK_equal: scroll_up(event, flashlight, camera, def_conf, *mouse); break;
            case XK_minus: scroll_down(event, flashlight, camera, def_conf, *mouse); break;
            case XK_0:
              camera->scale = 1.0;
              camera->delta_scale = 0.0;
              camera->position = vec2(0.0, 0.0);
              camera->velocity = vec2(0.0, 0.0);
              mirror = false;
              break;
            case XK_q:
            case XK_Escape:
              quitting = true;
              break;
            // TODO: implement XK_r -- reloading shaders
            // TODO: implement XK_m -- mirroring screenshot
            case XK_f:
              flashlight->is_enabled = !flashlight->is_enabled;
              break;
          }
        }

        case ButtonPress:
          printf("case: ButtonPress.\n");
          switch(event.xbutton.button)
          {
            case Button1:
              mouse->prev = mouse->curr;
              mouse->drag = true;
              camera->velocity = vec2(0.0, 0.0);
              break;
            case Button4: scroll_up(event, flashlight, camera, def_conf, *mouse); break;
            case Button5: scroll_down(event, flashlight, camera, def_conf, *mouse); break;
          }
          break;
        case ButtonRelease:
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

    printf("update camera.\n");
    camera_update(camera, def_conf, dt, *mouse, vec2(wa.width, wa.height));
    printf("update flashlight.\n");
    flashlight_update(flashlight, dt);

    printf("draw.\n");
    draw(*screenshot->image, *camera, shader_program, *vao, texture, vec2(wa.width, wa.height), *mouse, *flashlight, mirror);

    printf("swap buffers.\n");
    glXSwapBuffers(display, window);
    glFinish();

    //while (true)
    //{
    //  printf("Screenshot refreshing.\n");
    //  screenshot_refresh(screenshot, display, tracking_window);

    //  int w = screenshot->image->width;
    //  int h = screenshot->image->height;

    //  GLfloat vertices[4][5] = 
    //  {
    //    {w, 0, 0.0, 1.0, 1.0},
    //    {w, h, 0.0, 1.0, 0.0},
    //    {0, h, 0.0, 0.0, 0.0},
    //    {0, 0, 0.0, 0.0, 1.0}
    //  };

    //  GLuint indices[6] = 
    //  {
    //    0, 1, 3,
    //    1, 2, 3
    //  };

    //  printf("Binding array buffer vbo.\n");
    //  glBindBuffer(GL_ARRAY_BUFFER, *vbo);
    //  glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(sizeof(vertices)), vertices, GL_STATIC_DRAW);
    //  glTexImage2D(GL_TEXTURE_2D, 
    //               0, GL_RGB, 
    //               screenshot->image->width, screenshot->image->height,
    //               0, GL_RGBA, GL_UNSIGNED_BYTE, screenshot->image->data);
    //}
  }


  printf("Freed allocated structures.\n");
  XSetInputFocus(display, origin_window, RevertToParent, CurrentTime);
  XSync(display, 0);


  glDeleteVertexArrays(1, vao);
  glDeleteBuffers(1, vbo);
  glDeleteBuffers(1, ebo);

  free(mouse);
  free(camera);
  free(flashlight);
  screenshot_destroy(screenshot, display);
  free(set_window_attributes);
  free(visual_info);
  free(window_attributes);
  XCloseDisplay(display);
}
