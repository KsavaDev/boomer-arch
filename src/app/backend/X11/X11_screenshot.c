#include "X11_screenshot.h"
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XShm.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

X11_screenshot *X11_screenshot_new(Display *display, Window window)
{
  X11_screenshot *result = malloc(sizeof(X11_screenshot));
  XWindowAttributes *attributes = malloc(sizeof(XWindowAttributes));
  XGetWindowAttributes(display, window, attributes);

#ifdef MIT_SHM
  result->shminfo = (XShmSegmentInfo*)calloc(1, sizeof(XShmSegmentInfo));
  int screen = XDefaultScreen(display);
  result->image = XShmCreateImage(display, 
      XDefaultVisual(display, screen), 
      XDefaultDepthOfScreen(ScreenOfDisplay(display, screen)), 
      ZPixmap, 
      NULL, 
      result->shminfo, 
      attributes->width, 
      attributes->height);

  result->shminfo->shmid = syscall_three(SHMGET, 
      IPC_PRIVATE, 
      result->image->bytes_per_line * result->image->height, 
      IPC_CREATE | 0777);

  result->shminfo->shmaddr = (char*)(syscall_three(SHMAT, result->shminfo->shmid, 0, 0));

  result->image->data = result->shminfo->shmaddr;
  result->shminfo->readOnly = 0;

  XShmAttach(display, result->shminfo);
#else
  result->image = XGetImage(display, window, 0, 0, attributes.width, attributes.height, AllPlanes, ZPixmap);

#endif // MIT_SHM
  free(attributes);
  return result;
}

void X11_screenshot_destroy(X11_screenshot *X11_screenshot, Display *display)
{
#ifdef MIT_SHM
  XSync(display, 0);
  XShmDetach(display, X11_screenshot->shminfo);
  XDestroyImage(X11_screenshot->image);
  syscall_one(SHMDT, (long)X11_screenshot->shminfo->shmaddr);
  syscall_three(SHMCTL, X11_screenshot->shminfo->shmid, IPC_RMID, 0);
  free(X11_screenshot->shminfo);
#else
  XDestroyImage(X11_screenshot->image);
#endif // MIT_SHM
}

// NOTE: produces many X11 errors while tracked live update window is resized
void X11_screenshot_refresh(X11_screenshot *X11_screenshot, Display *display, Window window)
{
  XWindowAttributes attributes;
  XGetWindowAttributes(display, window, &attributes);

#ifdef MIT_SHM
  if (XShmGetImage(display, window, X11_screenshot->image, 0, 0, AllPlanes) == 0 || 
      attributes.width != X11_screenshot->image->width ||
      attributes.height != X11_screenshot->image->height)
  {
    X11_screenshot_destroy(X11_screenshot, display);
    X11_screenshot = X11_screenshot_new(display, window);
  }
#else 
    XImage *refreshed_image = XGetSubImage(display, window, 0, 0, attributes.width, attributes.height, AllPlanes, ZPixmap, X11_screenshot->image, 0, 0);
    if (refreshed_image == NULL || 
        refreshed_image->width != attributes.width ||
        refreshed_image->height != attributes.height)
    {
      XImage *new_image = XGetImage(display, window, 0, 0, attributes.width, attributes.height, AllPlanes, ZPixmap);
      if (new_image != NULL)
      {
        XDestroyImage(X11_screenshot->image);
        X11_screenshot->image = new_image;
      }
    }
		else 
    {
      X11_screenshot->image = refreshed_image;
    }
#endif // MIT_SHM
}

void X11_screenshot_save_to_ppm(XImage image, char *filepath)
{
  FILE *fp = fopen(filepath, "wb");

  fprintf(fp, "P6\n%u %u\n255\n", image.width, image.height);

  for (int i= 0; i < image.width * image.height; i++)
  {
    // Im not sure if its correct way to write pixels to ppm
    fputc(image.data[i * 4 + 2], fp);
    fputc(image.data[i * 4 + 1], fp);
    fputc(image.data[i * 4 + 0], fp);
  }

  fclose(fp);
}
