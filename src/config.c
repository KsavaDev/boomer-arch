#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Config default_config = 
{
	.min_scale = 0.01,
	.scroll_speed = 1.5,
	.drag_friction = 6.0,
	.scale_friction = 4.0,
};

Config config_load(const char *filepath)
{
	Config result = default_config;

	FILE *fp = fopen(filepath, "r");
	char buf[256];
	float value;

	if (fp == NULL)
	{
		fprintf(stderr, "Unable to open file.");
		exit(1);
	}

	while (fscanf(fp, "%s = %f", buf, &value) == 2)
	{
		if (strcmp(buf, "min_scale") == 0)
		{
			result.min_scale = value;
			continue;
		}			
		else if (strcmp(buf, "scroll_speed") == 0)
		{
			result.scroll_speed = value;
			continue;
		}			
		else if (strcmp(buf, "drag_friction") == 0)
		{
			result.drag_friction = value;
			continue;
		}
		else if (strcmp(buf, "scale_friction") == 0)
		{
			result.scale_friction = value;
			continue;
		}
		else 
		{
			fprintf(stderr, "Unkown config key %s\n", buf);
			exit(1);
		}
	}

	fclose(fp);

	return result;
}

void config_generate_default(const char *filepath)
{
	FILE *f = fopen(filepath, "w");

	fprintf(f, "min_scale = %0.2f\n", default_config.min_scale);
	fprintf(f, "scroll_speed = %0.2f\n", default_config.scroll_speed);
	fprintf(f, "drag_friction = %0.2f\n", default_config.drag_friction);
	fprintf(f, "scale_friction = %0.2f\n", default_config.scale_friction);
	
	fclose(f);
}

void config_print(Config conf, const char *filename)
{
	printf("Contents of file %s are:\n  min_scale = %0.2f\n  scroll_speed = %0.2f\n  drag_friction = %0.2f\n  scale_friction = %0.2f\n", filename, conf.min_scale, conf.scroll_speed, conf.drag_friction, conf.scale_friction);
}
