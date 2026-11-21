#ifndef NAVIGATION_H
#define NAVIGATION_H

#include "config.h"
#include "la.h"
#include <stdbool.h>

#define VELOCITY_THRESHOLD 15.0

typedef struct
{
	vec2f curr;
	vec2f prev;
	bool drag;
} mouse_t;

typedef struct
{
	vec2f position;
	vec2f velocity;
	vec2f scale_pivot;
	float scale;
	float delta_scale;
} camera_t;

vec2f world(camera_t camera, vec2f v);
// In original code here is the PXImage, but it dont even used in proc block. So I removed it.
void camera_update(camera_t *camera, config_t config, float dt, mouse_t mouse, vec2f window_size);

#endif // NAVIGATION_H
