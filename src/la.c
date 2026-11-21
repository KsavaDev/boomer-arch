// Linear Algebra
#include "la.h"
#include "math.h"

vec2f vec2(float x, float y)
{
	vec2f result;
	result.x = x;
	result.y = y;
	return result;
}

vec2f vec2f_mul_scalar(vec2f a, float s)
{
	return vec2(a.x * s, a.y * s);
}

vec2f vec2f_mul(vec2f a, vec2f b)
{
	return vec2(a.x * b.x, a.y * b.y);
}

vec2f vec2f_div_scalar(vec2f a, float s)
{
	return vec2(a.x / s, a.y / s);
}

vec2f vec2f_div(vec2f a, vec2f b)
{
	return vec2(a.x / b.x, a.y / b.y);
}

vec2f vec2f_sub(vec2f a, vec2f b)
{
	return vec2(a.x - b.x, a.y - b.y);
}

vec2f vec2f_add(vec2f a, vec2f b)
{
	return vec2(a.x + b.x, a.y + b.y);
}

void vec2f_sub_assign(vec2f *cur, vec2f b)
{
	cur->x -= b.x;
	cur->y -= b.y;
}

void vec2f_add_assign(vec2f *cur, vec2f b)
{
	cur->x += b.x;
	cur->y += b.y;
}

float vec2f_length(vec2f a)
{
	return sqrt(a.x * a.x + a.y * a.y);
}

vec2f vec2f_normalize(vec2f a)
{
	float len = vec2f_length(a);
	if (len == 0)
		return vec2(0.0, 0.0);
	else
	  return vec2(a.x / len, a.y / len);
}
