#pragma once
#include "tgaimage.h"
#include "geometry.h"

namespace gl
{
mat<4, 4> get_pitch_matrix(double pitch);
mat<4, 4> get_yaw_matrix(double yaw);
mat<4, 4> get_roll_matrix(double roll);
mat<4, 4> get_model_matrix(Transform transform);
void lookat(const vec3f eye, const vec3f center, const vec3f up);
void init_perspective(double near, double far, double fov_y, double aspect_ratio);
void init_viewport(const int x, const int y, const int w, const int h);
void init_zbuffer(const int width, const int height);

struct IShader {
    virtual vec4f vertex(int face, vec4f& v) = 0;
    virtual std::pair<bool,TGAColor> fragment(const vec3f bar) const = 0;
};

vec4f embed4(const vec3f& v, double fill = 1.0);

vec3f proj3(const vec4f& v);

void rasterize(const vec4f& v1, const vec4f& v2, const vec4f& v3, const IShader &shader, TGAImage &framebuffer);

void shadowing(const vec4f& v1, const vec4f& v2, const vec4f& v3, TGAColor shadow_color, TGAImage &framebuffer);
}