#pragma once
#include <vector>
#include <string>
#include <tuple>
#include "geometry.h"

class Model {
    std::vector<vec3> verts;
    std::vector<vec3> triangles;
public:
    Model(const std::string filename);
    const int get_triangles_size() {return triangles.size();}
    std::tuple<const vec3&, const vec3&, const vec3&> get_triangle(int i) 
    {
        return {verts[triangles[i].data[0] - 1], verts[triangles[i].data[1] - 1], verts[triangles[i].data[2] - 1]};
    }
};