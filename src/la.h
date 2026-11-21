#ifndef LA_H
#define LA_H

// I not sure about #ifdef __cplusplus and it needed or not.

typedef struct
{
	float x;
	float y;
} vec2f;

vec2f vec2(float x, float y);
vec2f vec2f_mul_scalar(vec2f a, float s);
vec2f vec2f_mul(vec2f a, vec2f b);
vec2f vec2f_div_scalar(vec2f a, float s);
vec2f vec2f_div(vec2f a, vec2f b);
vec2f vec2f_sub(vec2f a, vec2f b);
vec2f vec2f_add(vec2f a, vec2f b);
void vec2f_add_assign(vec2f *cur, vec2f b);
void vec2f_sub_assign(vec2f *cur, vec2f b);
float vec2f_length(vec2f a);
vec2f vec2f_normalize(vec2f a);

#endif // LA_H
