#ifndef NAVIGATION_H
#define NAVIGATION_H

#include "config.h"
#include "../util/la.h"
#include <stdbool.h>

#define VELOCITY_THRESHOLD 15.0

typedef struct
{
	vec2f curr;
	vec2f prev;
	bool drag;
} Mouse;

typedef struct
{
	vec2f position;
	vec2f velocity;
	vec2f scale_pivot;
	float scale;
	float delta_scale;
} Camera;

vec2f world(Camera camera, vec2f v);
// In original code here is the PXImage, but it dont even used in proc block. So I removed it.
void camera_update(Camera *camera, Config config, float dt, Mouse mouse, vec2f window_size);

#endif // NAVIGATION_H
