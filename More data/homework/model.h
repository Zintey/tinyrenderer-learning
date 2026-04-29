#pragma once
#include <vector>
#include <string>
#include <tuple>
#include "geometry.h"
#include "our_gl.h"

class Model {
    const TGAImage& _texture;
    const TGAImage& _normal_tex;
    std::vector<vec3f> verts;
    std::vector<vec3f> normals;
    std::vector<vec2f> uvs;
    std::vector<vec3i> triangles_vert;
    std::vector<vec3i> triangles_norm;
    std::vector<vec3i> triangles_uv; 
    Transform transform;
public:
    Model(const std::string filename, const TGAImage& texture, const TGAImage& normal_tex);
        

    const int get_triangles_size() {return triangles_vert.size();}
    std::tuple<vec4f, vec4f, vec4f> get_triangle_verts(int i) const
    {
        return {verts[triangles_vert[i].data[0] - 1], verts[triangles_vert[i].data[1] - 1], verts[triangles_vert[i].data[2] - 1]};
    }
    vec4f get_triangle_norm(int i, int index) const
    {
        return gl::embed4(normals[triangles_norm[i].data[index] - 1], 0);
    }
    vec2f get_triangle_uv(int i, int index) const
    {
        return uvs[triangles_uv[i].data[index] - 1];
    }
    void set_position(vec3f position) {transform.position = position;}
    void set_rotation(vec3f rotation) {transform.rotation = rotation;}
    void set_scale(vec3f scale) {transform.scale = scale;}
    const vec3f& get_position() const { return transform.position; }
    const vec3f& get_rotation() const { return transform.rotation; }
    const vec3f& get_scale() const { return transform.scale; }
    const Transform& get_transform() const { return transform; }
    const TGAImage& get_texture() const { return _texture; }
    const TGAImage& get_normal_tex() const { return _normal_tex; }
};