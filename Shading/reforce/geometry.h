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

template<int n, typename T> 
struct vec {
    T data[n] = {};
    vec() {}
    vec(std::initializer_list<T> l) {
        auto it = l.begin();
        for (int i = 0; i < n && it != l.end(); ++i, ++it) data[i] = *it;
    }
    T& operator[] (const int i) { assert(i >= 0 && i < n); return data[i]; }
    const T& operator[] (const int i) const { assert(i >= 0 && i < n); return data[i]; }
};

template<typename T> struct vec<2,T> {
    union {
        T data[2];
        struct { T x, y; };
        struct { T u, v; };
    };
    vec<2,T>() : x(0), y(0) {}
    vec<2,T>(T xx, T yy) : x(xx), y(yy) {}
    vec<2,T>(const vec<2,T>& v) : x(v.x), y(v.y) {}
    template<typename U>
    vec<2,T>(const vec<2,U>& v) : x((T)v.x), y((T)v.y) {}
    vec<2,T>(std::initializer_list<T> l) {
        auto it = l.begin();
        for (int i = 0; i < 2 && it != l.end(); ++i, ++it) data[i] = *it;
    }
    
    T& operator[] (const int i) { assert(i >= 0 && i < 2); return data[i]; }
    const T& operator[] (const int i) const { assert(i >= 0 && i < 2); return data[i]; }
    
    vec<2,T> operator- () const { return {-x, -y}; }
    vec<2,T> operator+ (const vec<2,T>& v) const { return {x + v.x, y + v.y}; }
    vec<2,T> operator- (const vec<2,T>& v) const { return {x - v.x, y - v.y}; }
    vec<2,T> operator* (double f) const { return {x * f, y * f}; }
    double dot(const vec<2,T>& v) const { return x * v.x + y * v.y; }
    
    double length() const { return std::sqrt(x * x + y * y); }
    vec<2,T>& normalize() { 
        double len = length(); 
        if (len > 0) { x /= len; y /= len; } 
        return *this; 
    }
    vec<2,T> normalized() const {
        vec<2,T> v = *this;
        double len = length();
        if (len > 0) { v.x /= len; v.y /= len; }
        return v;
    }
};

template<typename T> struct vec<3, T> {
    union {
        T data[3];
        struct { T x, y, z; };
    };
    vec<3,T>() : x(0), y(0), z(0) {}
    vec<3,T>(T xx, T yy, T zz) : x(xx), y(yy), z(zz) {}
    vec<3,T>(const Point& p) : x(p.x), y(p.y), z(0) {}
    vec<3,T>(std::initializer_list<T> l) {
        auto it = l.begin();
        for (int i = 0; i < 3 && it != l.end(); ++i, ++it) data[i] = *it;
    }
    
    T& operator[] (const int i) { assert(i >= 0 && i < 3); return data[i]; }
    const T& operator[] (const int i) const { assert(i >= 0 && i < 3); return data[i]; }
    
    vec<3,T> operator- () const { return {-x, -y, -z}; }
    vec<3,T> operator+ (const vec<3,T>& v) const { return {x + v.x, y + v.y, z + v.z}; }
    vec<3,T> operator- (const vec<3,T>& v) const { return {x - v.x, y - v.y, z - v.z}; }
    vec<3,T> operator* (double f) const { return {x * f, y * f, z * f}; }
    vec<3,T> operator/ (double f) const { return {x / f, y / f, z / f}; }
    double dot(const vec<3,T>& v) const { return x * v.x + y * v.y + z * v.z; }
    
    vec<3,T> cross(const vec<3,T>& v) const { return {y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x}; }
    double length() const { return std::sqrt(x * x + y * y + z * z); }
    vec<3,T>& normalize() { 
        double len = length(); 
        if (len > 0) { x /= len; y /= len; z /= len; } 
        return *this; 
    }
    vec<3,T> normalized() const {
        vec<3,T> v = *this;
        double len = length();
        if (len > 0) { v.x /= len; v.y /= len; v.z /= len; }
        return v;
    }
    friend std::ostream& operator<<(std::ostream& out, const vec<3,T>& v) {
        out << '(' << v.x << ", " << v.y << ", " << v.z << ')';
        return out;
    }
};

template<typename T> struct vec<4, T> {
    union {
        T data[4];
        struct { T x, y, z, w; };
    };
    vec<4,T>() : x(0), y(0), z(0), w(0) {}
    vec<4,T>(const vec<3,T>& v) : x(v.x), y(v.y), z(v.z), w(1) {}
    vec<4,T>(T xx, T yy, T zz, T ww) : x(xx), y(yy), z(zz), w(ww) {}
    vec<4,T>(std::initializer_list<T> l) {
        auto it = l.begin();
        for (int i = 0; i < 4 && it != l.end(); ++i, ++it) data[i] = *it;
    }
    
    T& operator[] (const int i) { assert(i >= 0 && i < 4); return data[i]; }
    const T& operator[] (const int i) const { assert(i >= 0 && i < 4); return data[i]; }
    
    vec<4,T> operator- () const { return {-x, -y, -z, -w}; }
    vec<4,T> operator+ (const vec<4,T>& v) const { return {x + v.x, y + v.y, z + v.z, w + v.w}; }
    vec<4,T> operator- (const vec<4,T>& v) const { return {x - v.x, y - v.y, z - v.z, w - v.w}; }
    vec<4,T> operator* (double f) const { return {x * f, y * f, z * f, w * f}; }
    vec<4,T> operator/ (double f) const { return {x / f, y / f, z / f, w / f}; }
    double dot(const vec<4,T>& v) const { return x * v.x + y * v.y + z * v.z + w * v.w; }
    
    vec<4,T> xyzw() const { return {x, y, z, w}; }
    vec<3,T> xyz()  const { return {x, y, z}; }
    vec<2,T> xy()   const { return {x, y}; }
    vec<2,T> xz()   const { return {x, z}; }
    vec<2,T> yz()   const { return {y, z}; }
    double length() const { return std::sqrt(x * x + y * y + z * z + w * w); }
    vec<4,T>& normalize() { 
        double len = length(); 
        if (len > 0) { x /= len; y /= len; z /= len;  w /= len;} 
        return *this; 
    }
    vec<4,T> normalized() const {
        vec<4,T> v = *this;
        double len = length();
        if (len > 0) { v.x /= len; v.y /= len; v.z /= len; v.w /= len;}
        return v;
    }
};

typedef vec<2,double> vec2f;
typedef vec<3,double> vec3f;
typedef vec<4,double> vec4f;

typedef vec<2,int> vec2i;
typedef vec<3,int> vec3i;
typedef vec<4,int> vec4i;

template<int R, int C, typename T = double> struct mat {
    vec<C, T> rows[R] = {};
    
    mat() {}
    mat(std::initializer_list<vec<C, T>> l) {
        auto it = l.begin();
        for (int i = 0; i < R && it != l.end(); ++i, ++it) rows[i] = *it;
    }

    vec<C, T>& operator[] (const int i) { assert(i >= 0 && i < R); return rows[i]; }
    const vec<C,T>& operator[] (const int i) const { assert(i >= 0 && i < R); return rows[i]; }
    
    vec<R, T> col(const int i) const {
        assert(i >= 0 && i < C);
        vec<R, T> ret;
        for (int j = 0; j < R; j++) ret[j] = rows[j][i];
        return ret;
    }

    // 设置列
    void set_col(const int i, const vec<R, T>& v) {
        assert(i >= 0 && i < C);
        for (int j = 0; j < R; j++) rows[j][i] = v[j];
    }

    static mat<R, C, T> identity() {
        mat<R, C> ret;
        for (int i = 0; i < R; i++) 
            for (int j = 0; j < C; j++) 
                ret[i][j] = (i == j ? (T)1.0 : (T)0.0);
        return ret;
    }


    mat<R, C, T> operator*(double f) const {
        mat<R, C, T> ret;
        for (int i = 0; i < R; i++) 
            for (int j = 0; j < C; j++) 
                ret[i][j] = rows[i][j] * f;
        return ret;
    }
};

template<int R, int C, typename T> vec<R, T> operator*(const mat<R, C, T>& m, const vec<C, T>& v) {
    vec<R, T> ret;
    for (int i = 0; i < R; i++) ret[i] = m[i].dot(v);
    return ret;
}

template<int R1, int C1, int C2, typename T> mat<R1, C2, T> operator*(const mat<R1, C1, T>& m1, const mat<C1, C2, T>& m2) {
    mat<R1, C2, T> ret;
    for (int i = 0; i < R1; i++) 
        for (int j = 0; j < C2; j++) 
            ret[i][j] = m1[i].dot(m2.col(j));
    return ret;
}

template<int R, int C, typename T> mat<C, R, T> transpose(const mat<R, C, T>& m) {
    mat<C, R, T> ret;
    for (int i = 0; i < C; i++) ret[i] = m.col(i);
    return ret;
}

template<int n, typename T> mat<n, n, T> invert(mat<n, n, T> m) {
    mat<n, n, T> inv = mat<n, n, T>::identity();
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


struct Transform {
    vec3f rotation = {0, 0, 0};
    vec3f scale = {1, 1, 1};
    vec3f position = {0, 0, 0};
};

// struct Triangle {
//     vec3f v[3];
//     Triangle (vec3f a, vec3f b, vec3f c) : v{a, b, c} {}
//     double min_z() const { return std::min({v[0].z, v[1].z, v[2].z}); }
// };