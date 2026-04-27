#include <cmath>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <iostream>
#include <algorithm>

#include "tgaimage.h"
#include "model.h"

constexpr double PI = 3.1415926;

constexpr int width  = 800;
constexpr int height = 800;

constexpr TGAColor white   = {255, 255, 255, 255};
constexpr TGAColor green   = {  0, 255,   0, 255};
constexpr TGAColor red     = {  0,   0, 255, 255};
constexpr TGAColor blue    = {255, 128,  64, 255};
constexpr TGAColor yellow  = {  0, 200, 255, 255};

TGAColor operator* (const double& d, const TGAColor& c)  { return {static_cast<std::uint8_t>(c.bgra[0] * d),static_cast<std::uint8_t>(c.bgra[1] * d),static_cast<std::uint8_t>(c.bgra[2] * d)};}
TGAColor operator* (const TGAColor& c, const double& d)  { return d * c;}
TGAColor operator+ (const TGAColor& c, const TGAColor& d)  { return {static_cast<uint8_t>(c.bgra[0] + d.bgra[0]), static_cast<uint8_t>(c.bgra[1] + d.bgra[1]), static_cast<uint8_t>(c.bgra[2] + d.bgra[2])};}
TGAColor get_rand_color() {return {static_cast<std::uint8_t>(rand()%255), static_cast<std::uint8_t>(rand()%255), static_cast<std::uint8_t>(rand()%255), static_cast<std::uint8_t>(rand()%255)};}

vec3 camera_pos = {0, 0, 3};

vec3 persp(const vec3& v)
{
    return v / (1 - v.z / camera_pos.z);
}

vec3 rot(const vec3& v, double angle = 30)
{
    angle = angle / 180.0 * PI;
    mat<3, 3> Ry = {{std::cos(angle), 0, std::sin(angle)}, {0, 1, 0}, {-std::sin(angle), 0, std::cos(angle)}};
    return Ry * v;
}

Point project(vec3 v) {
    return {static_cast<int>((v.x + 1) * width / 2.0), 
        static_cast<int>((v.y + 1) * height / 2.0)};
}
unsigned char project(double z) { return static_cast<unsigned char>((z + 1) * 255. / 2);}

bool is_inside(const Point& a, const Point& b, const Point& c, const Point& p) 
{
    int w0 = (b.x - a.x) * (p.y - a.y) - (b.y - a.y) * (p.x - a.x);
    int w1 = (c.x - b.x) * (p.y - b.y) - (c.y - b.y) * (p.x - b.x);
    int w2 = (a.x - c.x) * (p.y - c.y) - (a.y - c.y) * (p.x - c.x);
    return (w0 >= 0 && w1 >= 0 && w2 >= 0) || (w0 <= 0 && w1 <= 0 && w2 <= 0);
}

double area(const Point& a, const Point& b, const Point& c) 
{
    return .5 * ((a.x * b.y - b.x * a.y) + (b.x * c.y - c.x * b.y) + (c.x * a.y - a.x * c.y));
}

void rasterize_triangle(TGAImage& framebuffer, Point p0, Point p1, Point p2, TGAColor color) 
{
    int minX = std::max(0, std::min({p0.x, p1.x, p2.x}));
    int minY = std::max(0, std::min({p0.y, p1.y, p2.y}));
    int maxX = std::min(width - 1, std::max({p0.x, p1.x, p2.x}));
    int maxY = std::min(height - 1, std::max({p0.y, p1.y, p2.y}));

    for (int x = minX; x <= maxX; x++) 
    {
        for (int y = minY; y <= maxY; y++) 
        {
            Point p{x, y};
            if (is_inside(p0, p1, p2, p)) 
            {
                framebuffer.set(x, y, color);
            }
        }
    }
}

void rasterize_triangle(TGAImage& framebuffer, TGAImage& zbuffer, vec3 v1, vec3 v2, vec3 v3, TGAColor color)
{
    v1 = rot(v1), v2 = rot(v2), v3 = rot(v3);
    v1 = persp(v1), v2 = persp(v2), v3 = persp(v3);
    int z1 = project(v1.z);
    int z2 = project(v2.z);
    int z3 = project(v3.z);
    Point p0 = project(v1);
    Point p1 = project(v2);
    Point p2 = project(v3);
    int minX = std::max(0, std::min({p0.x, p1.x, p2.x}));
    int minY = std::max(0, std::min({p0.y, p1.y, p2.y}));
    int maxX = std::min(width - 1, std::max({p0.x, p1.x, p2.x}));
    int maxY = std::min(height - 1, std::max({p0.y, p1.y, p2.y}));

    double st = area(p0, p1, p2);
// #pragma omp parallel for
    for (int x = minX; x <= maxX; x++) 
    {
        for (int y = minY; y <= maxY; y++) 
        {
            Point p{x, y};
            double a = area(p0, p1, p) / st;
            double b = area(p1, p2, p) / st;
            double c = area(p2, p0, p) / st;
            if (a < 0 || b < 0 || c < 0) continue;
            unsigned char z = static_cast<unsigned char> (a * z1 + b * z2 + c * z3);
            if (zbuffer.get(x, y).bgra[0] < z)
            {
                zbuffer.set(x, y, {z});
                framebuffer.set(x, y, color);
            }
        }
    }
}

void rasterize_triangle(TGAImage& framebuffer, Point p0, Point p1, Point p2, TGAColor c0, TGAColor c1, TGAColor c2) 
{
    int minX = std::max(0, std::min({p0.x, p1.x, p2.x}));
    int minY = std::max(0, std::min({p0.y, p1.y, p2.y}));
    int maxX = std::min(width - 1, std::max({p0.x, p1.x, p2.x}));
    int maxY = std::min(height - 1, std::max({p0.y, p1.y, p2.y}));

    double st = area(p0, p1, p2);
    if (st == 0) return;

    for (int x = minX; x <= maxX; x++) 
    {
        for (int y = minY; y <= maxY; y++) 
        {
            Point p{x, y};
            if (is_inside(p0, p1, p2, p)) 
            {
                double a = area(p0, p1, p) / st;
                double b = area(p1, p2, p) / st;
                double c = area(p2, p0, p) / st;
                TGAColor color = (a * c0) + (b * c1) + (c * c2);
                framebuffer.set(x, y, color);
            }
        }
    }
}

int main(int argc, char** argv) 
{
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <obj_file>" << std::endl;
        return 1;
    }

    TGAImage framebuffer(width, height, TGAImage::RGB);
    TGAImage zbuffer(width, height, TGAImage::Format::GRAYSCALE);
    auto start = std::chrono::steady_clock::now();

    std::srand(std::time(nullptr));

    // triangle(framebuffer, vec3(7  / 128.0 - 0.5, 45 / 128.0 - 0.5), vec3(3 / 128.0 - 0.5, 100 / 128.0 - 0.5), vec3(45 / 128.0 - 0.5, 60 / 128.0 - 0.5)
    //         , get_rand_color(), get_rand_color(), get_rand_color());
    // triangle(framebuffer, vec3(120 / 128.0 - 0.5, 35 / 128.0 - 0.5), vec3(90 / 128.0 - 0.5, 5 / 128.0 - 0.5), vec3(45 / 128.0 - 0.5, 110 / 128.0 - 0.5)
    //         , get_rand_color(), get_rand_color(), get_rand_color());
    // triangle(framebuffer, vec3(115 / 128.0 - 0.5, 83 / 128.0 - 0.5), vec3(80 / 128.0 - 0.5, 90 / 128.0 - 0.5), vec3(85 / 128.0 - 0.5, 120 / 128.0 - 0.5)
    //         , get_rand_color(), get_rand_color(), get_rand_color());

    Model model(argv[1]);
    
    for (int i = 0; i < model.get_triangles_size(); i++) 
    {
        auto [v1, v2, v3] = model.get_triangle(i);
        rasterize_triangle(framebuffer, zbuffer, v1, v2, v3, get_rand_color());
    }
    
    auto end = std::chrono::steady_clock::now();
    framebuffer.write_tga_file("framebuffer.tga");
    zbuffer.write_tga_file("z_buffer.tga");
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() << " ms" << std::endl;
    
    system("start framebuffer.tga");
    system("start z_buffer.tga");
    return 0;
}


