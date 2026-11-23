#ifndef CONFIG_H
#define CONFIG_H

typedef struct
{
	float min_scale;
	float scroll_speed;
	float drag_friction;
	float scale_friction;
} Config;


extern Config default_config; 

Config config_load(const char *filepath);
void config_generate_default(const char *filepath);
void config_print(Config conf, const char *filename);

#endif // CONFIG_H
