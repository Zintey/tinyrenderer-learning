#pragma once
#include <vector>
#include <string>
#include <tuple>
#include "geometry.h"

class Model {
    std::vector<vec3f> verts;
    std::vector<vec3f> triangles;
    Transform transform;
public:
    Model(const std::string filename);
    const int get_triangles_size() {return triangles.size();}
    std::tuple<vec4f, vec4f, vec4f> get_triangle(int i) 
    {
        return {verts[triangles[i].data[0] - 1], verts[triangles[i].data[1] - 1], verts[triangles[i].data[2] - 1]};
    }
    void set_position(vec3f position) {transform.position = position;}
    void set_rotation(vec3f rotation) {transform.rotation = rotation;}
    void set_scale(vec3f scale) {transform.scale = scale;}
    const vec3f& get_position() const { return transform.position; }
    const vec3f& get_rotation() const { return transform.rotation; }
    const vec3f& get_scale() const { return transform.scale; }
    const Transform& get_transform() const { return transform; }
};