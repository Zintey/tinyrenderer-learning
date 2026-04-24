#pragma once
#include <cmath>
#include <cassert>
#include <iostream>

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
    double lenght() const {
        return std::sqrt(x * x + y * y + z * z);
    }
    vec<3>& normalize() {
        double len = lenght();
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
    Point a, b, c;

    bool contains(const Point& p) const {
        int w0 = (b.x - a.x) * (p.y - a.y) - (b.y - a.y) * (p.x - a.x);
        int w1 = (c.x - b.x) * (p.y - b.y) - (c.y - b.y) * (p.x - b.x);
        int w2 = (a.x - c.x) * (p.y - c.y) - (a.y - c.y) * (p.x - c.x);
        return (w0 >= 0 && w1 >= 0 && w2 >= 0) || (w0 <= 0 && w1 <= 0 && w2 <= 0);
    }
};