#ifndef DOOMER_H
#define DOOMER_H

// This is a main file (dommer/(.c, .h)) where it checks what Window System is installed on machine and by this information uses different method for scrabbing image from screen and providing it to application for zooming.

#include "../util/la.h"
#include "navigation.h"

void scroll_up(unsigned int state, long control_mask, Flashlight *flashlight, Camera *camera, Config config, Mouse mouse);
void scroll_down(unsigned int state, long control_mask, Flashlight *flashlight, Camera *camera, Config config, Mouse mouse);

#endif // DOOMER_H
