#ifndef SCREENSHOT_H
#define SCREENSHOT_H

#include <X11/X.h>
#include <X11/Xlib.h>
#include <stdio.h>
// TODO: add MIT_SHM to application options
#ifndef MIT_SHM

  #define MIT_SHM
  #include <X11/extensions/XShm.h>

#if defined(__amd64__) || defined(__x86_64__)
 
typedef enum
{
  SHMGET  = 29,
  SHMAT   = 30,
  SHMCTL  = 31,
  SHMDT   = 67,
} Number;

static inline long syscall_one(Number n, long a1)
{
  long result;
__asm__ volatile
    (
     "syscall"
     : "=a"(result)
     : "a"((long)n), "D"(a1)
     : "memory", "r11", "rcx", "cc"
    );
  return result;
}

static inline long syscall_three(Number n, long a1, long a2, long a3)
{
  long result;
  __asm__ volatile
    (
     "syscall"
     : "=a"(result)
     : "a"((long)n), "D"(a1), "S"(a2), "d"(a3)
     : "memory", "r11", "rcx", "cc"
    );
  return result;
}


#else
#error "Supported only Linux x86_64."
#endif // __amd64__

enum 
{
  IPC_PRIVATE = 0,
  IPC_CREATE  = 512,
  IPC_RMID    = 0,
};

#endif // MIT_SHM 
typedef struct
{
  XImage image;
#ifdef MIT_SHM
  XShmSegmentInfo *shminfo;
#endif // MIT_SHM
} Screenshot;

Screenshot *screenshot_new(Display *display, Window window);
void screenshot_destroy(Screenshot *screenshot, Display *display);
void screenshot_refresh(Screenshot *screenshot, Display *display, Window window);
void screenshot_save_to_ppm(XImage image, char *filepath);

#endif // SCREENSHOT_H
