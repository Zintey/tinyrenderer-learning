#pragma once
#include <cmath>
#include <cassert>
#include <iostream>
#include <algorithm>
#include <initializer_list>

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
    vec() {}
    vec(std::initializer_list<double> l) {
        auto it = l.begin();
        for (int i = 0; i < n && it != l.end(); ++i, ++it) data[i] = *it;
    }
    double& operator[] (const int i) { assert(i >= 0 && i < n); return data[i]; }
    const double& operator[] (const int i) const { assert(i >= 0 && i < n); return data[i]; }
};

template<> struct vec<2> {
    union {
        double data[2];
        struct { double x, y; };
        struct { double u, v; };
    };
    vec<2>() : x(0), y(0) {}
    vec<2>(double xx, double yy) : x(xx), y(yy) {}
    vec<2>(std::initializer_list<double> l) {
        auto it = l.begin();
        for (int i = 0; i < 2 && it != l.end(); ++i, ++it) data[i] = *it;
    }
    
    double& operator[] (const int i) { assert(i >= 0 && i < 2); return data[i]; }
    const double& operator[] (const int i) const { assert(i >= 0 && i < 2); return data[i]; }
    
    vec<2> operator- () const { return {-x, -y}; }
    vec<2> operator+ (const vec<2>& v) const { return {x + v.x, y + v.y}; }
    vec<2> operator- (const vec<2>& v) const { return {x - v.x, y - v.y}; }
    vec<2> operator* (double f) const { return {x * f, y * f}; }
    double operator* (const vec<2>& v) const { return x * v.x + y * v.y; }
    
    double length() const { return std::sqrt(x * x + y * y); }
    vec<2>& normalize() { double len = length(); if (len > 0) { x /= len; y /= len; } return *this; }
};

template<> struct vec<3> {
    union {
        double data[3];
        struct { double x, y, z; };
    };
    vec<3>() : x(0), y(0), z(0) {}
    vec<3>(double xx, double yy, double zz) : x(xx), y(yy), z(zz) {}
    vec<3>(const Point& p) : x(p.x), y(p.y), z(0) {}
    vec<3>(std::initializer_list<double> l) {
        auto it = l.begin();
        for (int i = 0; i < 3 && it != l.end(); ++i, ++it) data[i] = *it;
    }
    
    double& operator[] (const int i) { assert(i >= 0 && i < 3); return data[i]; }
    const double& operator[] (const int i) const { assert(i >= 0 && i < 3); return data[i]; }
    
    vec<3> operator- () const { return {-x, -y, -z}; }
    vec<3> operator+ (const vec<3>& v) const { return {x + v.x, y + v.y, z + v.z}; }
    vec<3> operator- (const vec<3>& v) const { return {x - v.x, y - v.y, z - v.z}; }
    vec<3> operator* (double f) const { return {x * f, y * f, z * f}; }
    vec<3> operator/ (double f) const { return {x / f, y / f, z / f}; }
    double operator* (const vec<3>& v) const { return x * v.x + y * v.y + z * v.z; }
    
    vec<3> cross(const vec<3>& v) const { return {y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x}; }
    double length() const { return std::sqrt(x * x + y * y + z * z); }
    vec<3>& normalize() { double len = length(); if (len > 0) { x /= len; y /= len; z /= len; } return *this; }
    
    friend std::ostream& operator<<(std::ostream& out, const vec<3>& v) {
        out << '(' << v.x << ", " << v.y << ", " << v.z << ')';
        return out;
    }
};

template<> struct vec<4> {
    union {
        double data[4];
        struct { double x, y, z, w; };
    };
    vec<4>() : x(0), y(0), z(0), w(0) {}
    vec<4>(double xx, double yy, double zz, double ww) : x(xx), y(yy), z(zz), w(ww) {}
    vec<4>(std::initializer_list<double> l) {
        auto it = l.begin();
        for (int i = 0; i < 4 && it != l.end(); ++i, ++it) data[i] = *it;
    }
    
    double& operator[] (const int i) { assert(i >= 0 && i < 4); return data[i]; }
    const double& operator[] (const int i) const { assert(i >= 0 && i < 4); return data[i]; }
    
    vec<4> operator- () const { return {-x, -y, -z, -w}; }
    vec<4> operator+ (const vec<4>& v) const { return {x + v.x, y + v.y, z + v.z, w + v.w}; }
    vec<4> operator- (const vec<4>& v) const { return {x - v.x, y - v.y, z - v.z, w - v.w}; }
    vec<4> operator* (double f) const { return {x * f, y * f, z * f, w * f}; }
    double operator* (const vec<4>& v) const { return x * v.x + y * v.y + z * v.z + w * v.w; }
    
    double length() const { return std::sqrt(x * x + y * y + z * z + w * w); }
    vec<4>& normalize() { double len = length(); if (len > 0) { x /= len; y /= len; z /= len; w /= len; } return *this; }
};

typedef vec<2> vec2;
typedef vec<3> vec3;
typedef vec<4> vec4;

template<int R, int C> struct mat {
    vec<C> rows[R] = {};
    
    mat() {}
    mat(std::initializer_list<vec<C>> l) {
        auto it = l.begin();
        for (int i = 0; i < R && it != l.end(); ++i, ++it) rows[i] = *it;
    }

    vec<C>& operator[] (const int i) { assert(i >= 0 && i < R); return rows[i]; }
    const vec<C>& operator[] (const int i) const { assert(i >= 0 && i < R); return rows[i]; }
    
    vec<R> col(const int i) const {
        assert(i >= 0 && i < C);
        vec<R> ret;
        for (int j = 0; j < R; j++) ret[j] = rows[j][i];
        return ret;
    }

    void set_col(const int i, const vec<R>& v) {
        assert(i >= 0 && i < C);
        for (int j = 0; j < R; j++) rows[j][i] = v[j];
    }

    static mat<R, C> identity() {
        mat<R, C> ret;
        for (int i = 0; i < R; i++) 
            for (int j = 0; j < C; j++) 
                ret[i][j] = (i == j ? 1.0 : 0.0);
        return ret;
    }

    mat<R, C> operator*(double f) const {
        mat<R, C> ret;
        for (int i = 0; i < R; i++) 
            for (int j = 0; j < C; j++) 
                ret[i][j] = rows[i][j] * f;
        return ret;
    }
};

template<int R, int C> vec<R> operator*(const mat<R, C>& m, const vec<C>& v) {
    vec<R> ret;
    for (int i = 0; i < R; i++) ret[i] = m[i] * v;
    return ret;
}

template<int R1, int C1, int C2> mat<R1, C2> operator*(const mat<R1, C1>& m1, const mat<C1, C2>& m2) {
    mat<R1, C2> ret;
    for (int i = 0; i < R1; i++) 
        for (int j = 0; j < C2; j++) 
            ret[i][j] = m1[i] * m2.col(j);
    return ret;
}

template<int R, int C> mat<C, R> transpose(const mat<R, C>& m) {
    mat<C, R> ret;
    for (int i = 0; i < C; i++) ret[i] = m.col(i);
    return ret;
}

template<int n> mat<n, n> invert(mat<n, n> m) {
    mat<n, n> inv = mat<n, n>::identity();
    for (int i = 0; i < n; i++) {
        double pivot = m[i][i];
        if (std::abs(pivot) < 1e-9) {
            for (int j = i + 1; j < n; j++) {
                if (std::abs(m[j][i]) > std::abs(pivot)) {
                    std::swap(m[i], m[j]);
                    std::swap(inv[i], inv[j]);
                    pivot = m[i][i];
                    break;
                }
            }
        }
        assert(std::abs(pivot) > 1e-9 && "Matrix is singular");
        for (int j = 0; j < n; j++) {
            m[i][j] /= pivot;
            inv[i][j] /= pivot;
        }
        for (int k = 0; k < n; k++) {
            if (k != i) {
                double factor = m[k][i];
                for (int j = 0; j < n; j++) {
                    m[k][j] -= m[i][j] * factor;
                    inv[k][j] -= inv[i][j] * factor;
                }
            }
        }
    }
    return inv;
}

struct Triangle {
    vec3 v[3];
    Triangle (vec3 a, vec3 b, vec3 c) : v{a, b, c} {}
    double min_z() const { return std::min({v[0].z, v[1].z, v[2].z}); }
};