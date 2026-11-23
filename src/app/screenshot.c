#include "screenshot.h"
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XShm.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Screenshot *screenshot_new(Display *display, Window window)
{
  printf("Allocating screenshot.\n");
  Screenshot *result = malloc(sizeof(Screenshot));
  XWindowAttributes *attributes = malloc(sizeof(XWindowAttributes));
  printf("Getting Window attributes.\n");
  XGetWindowAttributes(display, window, attributes);

#ifdef MIT_SHM
  printf("Entering MIT_SHM branch.\n");
  result->shminfo = (XShmSegmentInfo*)malloc(sizeof(XShmSegmentInfo));
  int screen = XDefaultScreen(display);
  printf("Creating Shm image.\n");
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
  memset(result->image->data, 0xAA, result->image->bytes_per_line * result->image->height);
  Status status = XShmGetImage(display, window, result->image, 0, 0, AllPlanes);
  fprintf(stderr, "XShmGetImage -> %d, depth=%d, bpp=%d, first bytes: %02x %02x %02x %02x\n",
        status, result->image->depth, result->image->bits_per_pixel,
        (unsigned char)result->image->data[0], (unsigned char)result->image->data[1],
        (unsigned char)result->image->data[2], (unsigned char)result->image->data[3]);

#else
  result->image = XGetImage(display, window, 0, 0, attributes.width, attributes.height, AllPlanes, ZPixmap);

#endif // MIT_SHM
  free(attributes);
  return result;
}

void screenshot_destroy(Screenshot *screenshot, Display *display)
{
#ifdef MIT_SHM
  XSync(display, 0);
  XShmDetach(display, screenshot->shminfo);
  XDestroyImage(screenshot->image);
  syscall_one(SHMDT, (long)screenshot->shminfo->shmaddr);
  syscall_three(SHMCTL, screenshot->shminfo->shmid, IPC_RMID, 0);
  free(screenshot->shminfo);
#else
  XDestroyImage(screenshot->image);
#endif // MIT_SHM
}

// NOTE: produces many X11 errors while tracked live update window is resized
void screenshot_refresh(Screenshot *screenshot, Display *display, Window window)
{
  printf("Entering screenshot_refresh.\n");
  printf("Allocating Window attributes.\n");
  XWindowAttributes *attributes = malloc(sizeof(XWindowAttributes));
  XGetWindowAttributes(display, window, attributes);

#ifdef MIT_SHM
  printf("Entering MIT_SHM branch.\n");
  if (XShmGetImage(display, window, screenshot->image, 0, 0, AllPlanes) == 0 || 
      attributes->width != screenshot->image->width ||
      attributes->height != screenshot->image->height)
  {
    printf("Destroing and creating screenshot.\n");
    screenshot_destroy(screenshot, display);
    screenshot = screenshot_new(display, window);
  }
  else 
  {
    printf("Getting refreshed_image.\n");
    printf("Is screenshot->image exist? Address: %p\n", screenshot->image);
    printf("attributes->width = %d\n", attributes->width);
    printf("attributes->height = %d\n", attributes->height);
    XImage *refreshed_image = XGetSubImage(display, window, 0, 0, attributes->width, attributes->height, AllPlanes, ZPixmap, screenshot->image, 0, 0);
    printf("refreshed_image is got.\n");
    if (refreshed_image == NULL || 
        refreshed_image->width != attributes->width ||
        refreshed_image->height != attributes->height)
    {
      printf("refreshed_image is NULL. Getting new_image.\n");
      printf("Display is exist? Address: %p\n", display);
      printf("Window is exist? Address: %p\n", &window);
      XImage *new_image = XGetImage(display, window, 0, 0, attributes->width, attributes->height, AllPlanes, ZPixmap);
      if (new_image != NULL)
      {
        printf("Destroing screenshot->image. And screenshot->image = new_image;\n");
        XDestroyImage(screenshot->image);
        screenshot->image = new_image;
      }
      else 
      {
        printf("screenshot->image = refreshed_image;\n");
        screenshot->image = refreshed_image;
      }
    }
  }
#endif // MIT_SHM
  free(attributes);
}

void screenshot_save_to_ppm(XImage image, char *filepath)
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
