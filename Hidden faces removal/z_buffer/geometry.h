#pragma once
#include <cmath>
#include <cassert>
#include <iostream>
#include <algorithm>

struct Point {
    int x, y;
    Point operator* (float f) const { return {(int) std::round(x * f), (int) std::round(y * f)}; }
    Point operator+ (const Point& p) const { return {x + p.x, y + p.y}; }
    Point operator- (const Point& p) const { return {x - p.x, y - p.y}; }
    bool operator<(const Point& p) const { return y != p.y ? y < p.y : x < p.x; }
};

inline std::ostream& operator<<(std::ostream& out, const Point& p) {
    out << '(' << p.x << ", " << p.y << ')';
    return out;
}

template<int n> struct vec {
    double data[n] = {};
    double& operator[] (const int i) { assert(i >= 0 && i < n); return data[i]; }
    const double& operator[] (const int i) const { assert(i >= 0 && i < n); return data[i]; }
};

template<> struct vec<3> {
    static constexpr int n = 3; 
    union {
        double data[3] = {};
        struct {double x, y, z;};
    };
    vec<3>(double xx = 0, double yy = 0, double zz = 0) : x(xx), y(yy), z(zz) {}
    vec<3>(const Point& p) : x(p.x), y(p.y), z(0) {}
    
    double& operator[] (const int i) { assert(i >= 0 && i < n); return data[i]; }
    const double& operator[] (const int i) const { assert(i >= 0 && i < n); return data[i]; }
    
    vec<3> operator- () const {return {-x, -y, -z};}
    vec<3> operator- (const vec<3>& v) const { return {x - v.x, y - v.y, z - v.z}; }
    vec<3> operator+ (const vec<3>& v) const { return {x + v.x, y + v.y, z + v.z}; }
    double operator* (const vec<3>& v) const { return x * v.x + y * v.y + z * v.z; }
    vec<3> operator* (double f) const { return {x * f, y * f, z * f}; }
    
    vec<3> cross(const vec<3>& v) const {
        return {y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x};
    }
    double length() const {
        return std::sqrt(x * x + y * y + z * z);
    }
    vec<3>& normalize() {
        double len = length();
        if (len > 0) {
            x /= len; y /= len; z /= len;
        }
        return *this;
    }
    friend std::ostream& operator<<(std::ostream& out, const vec<3>& v) {
        out << '(' << v.x << ", " << v.y << ", " << v.z << ')';
        return out;
    }
};

typedef vec<3> vec3;

struct Triangle {
    vec3 v[3];
    Triangle (vec3 a, vec3 b, vec3 c) : v{a, b, c} {}
    double min_z() const { return std::min({v[0].z, v[1].z, v[2].z}); }
};