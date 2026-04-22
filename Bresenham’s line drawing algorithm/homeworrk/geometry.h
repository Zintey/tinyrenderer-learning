#pragma once
#include <cmath>
#include <cassert>
#include <iostream>

template<int n> struct vec {
    double data[n] = {};
    double& operator[] (const int i) { assert(i >= 0 & i < n); return data[i];}
    const double& operator[] (const int i) const { assert(i >= 0 & i < n); return data[i];}
};

template<int n> std::ostream& operator<<(std::ostream& out, const vec<n>& vec)
{
    for (int i = 0; i < n; i++) out << vec[i] << ' ';
    return out;
}

template<> struct vec<3> {
    static constexpr int n = 3; 
    union {
        double data[3] = {};
        struct {double x, y, z;};
    };
    double& operator[] (const int i) { assert(i >= 0 & i < n); return data[i];}
    const double& operator[] (const int i) const { assert(i >= 0 & i < n); return data[i];}
};

typedef vec<3> vec3;