#include <stdio.h>
#include <stdlib.h>
#include "doomer.h"
#include "navigation.h"

void scroll_up(unsigned int state, long control_mask, Flashlight *flashlight, Camera *camera, Config config, Mouse mouse)
{
  printf("Scrolling up.\n");
	printf("state & control_mask: %d\n", state & control_mask);
	if ((state & control_mask) == 0)
	{
		printf("state: %d\n", state);
		printf("control_mask: %d\n", control_mask);
	}
  if ((state & control_mask) > 0 && flashlight->is_enabled)
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

void scroll_down(unsigned int state, long control_mask, Flashlight *flashlight, Camera *camera, Config config, Mouse mouse)
{
	printf("state & control_mask: %d\n", state & control_mask);
	if ((state & control_mask) == 0)
	{
		printf("state: %d\n", state);
		printf("control_mask: %d\n", control_mask);
	}
  if ((state & control_mask) > 0 && flashlight->is_enabled)
  {
    flashlight->delta_radius -= INITIAL_FL_DELTA_RADIUS;
  }
  else 
  {
    camera->delta_scale -= config.scroll_speed;
    camera->scale_pivot = mouse.curr;
  }
}
