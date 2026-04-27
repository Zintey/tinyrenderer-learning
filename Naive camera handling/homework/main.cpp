#include <cmath>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <iostream>
#include <algorithm>

#include "tgaimage.h"
#include "model.h"
#include "our_gl.h"

constexpr double PI = 3.1415926;

extern mat<4, 4> View, Projection;
extern std::vector<double> zbuffer;

constexpr TGAColor white   = {255, 255, 255, 255};
constexpr TGAColor green   = {  0, 255,   0, 255};
constexpr TGAColor red     = {  0,   0, 255, 255};
constexpr TGAColor blue    = {255, 128,  64, 255};
constexpr TGAColor yellow  = {  0, 200, 255, 255};

TGAColor operator* (const double& d, const TGAColor& c)  { return {static_cast<std::uint8_t>(c.bgra[0] * d),static_cast<std::uint8_t>(c.bgra[1] * d),static_cast<std::uint8_t>(c.bgra[2] * d)};}
TGAColor operator* (const TGAColor& c, const double& d)  { return d * c;}
TGAColor operator+ (const TGAColor& c, const TGAColor& d)  { return {static_cast<uint8_t>(c.bgra[0] + d.bgra[0]), static_cast<uint8_t>(c.bgra[1] + d.bgra[1]), static_cast<uint8_t>(c.bgra[2] + d.bgra[2])};}
TGAColor get_rand_color() {return {static_cast<std::uint8_t>(rand()%255), static_cast<std::uint8_t>(rand()%255), static_cast<std::uint8_t>(rand()%255), static_cast<std::uint8_t>(rand()%255)};}

struct RandomShader : gl::IShader {
    const Model& _model;
    RandomShader(const Model &model) : _model(model) {}
    virtual vec4f vertex(vec4f& v) const {
        return v = Projection * View
        * gl::get_model_matrix(_model.get_transform()) * v; // MVP 变换
    }
    virtual std::pair<bool,TGAColor> fragment(const vec3f bar) const {
        return {true, get_rand_color()};
    };
};

unsigned char project(double z) { return static_cast<unsigned char>((std::clamp(z, -1.0, 1.0) + 1) * 255. / 2);}

// bool is_inside(const Point& a, const Point& b, const Point& c, const Point& p) 
// {
//     int w0 = (b.x - a.x) * (p.y - a.y) - (b.y - a.y) * (p.x - a.x);
//     int w1 = (c.x - b.x) * (p.y - b.y) - (c.y - b.y) * (p.x - b.x);
//     int w2 = (a.x - c.x) * (p.y - c.y) - (a.y - c.y) * (p.x - c.x);
//     return (w0 >= 0 && w1 >= 0 && w2 >= 0) || (w0 <= 0 && w1 <= 0 && w2 <= 0);
// }

int main(int argc, char** argv) 
{
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <obj_file>" << std::endl;
        return 1;
    }

    constexpr int width  = 800;
    constexpr int height = 800;
    constexpr double aspect_radio = width / height;
    const vec3f    eye{-1, 0, 2};
    const vec3f target{ 0, 0, 0};
    const vec3f     up{ 0, 1, 0};

    gl::lookat(eye, target, up);
    gl::init_perspective(10, 300, 60, aspect_radio);
    gl::init_viewport(width/16, height/16, width*7/8, height*7/8);
    gl::init_zbuffer(width, height);

    TGAImage framebuffer(width, height, TGAImage::RGB);
    auto start = std::chrono::steady_clock::now();

    std::srand(std::time(nullptr));

    Model model(argv[1]);
    std::vector<std::vector<double>> z_buffer(width, std::vector<double>(height, -std::numeric_limits<double>::max()));
    RandomShader random_shader(model); 
    for (int i = 0; i < model.get_triangles_size(); i++) 
    {
        auto [v1, v2, v3] = model.get_triangle(i);
        random_shader.vertex(v1);   // 顶点Shader MVP变换
        random_shader.vertex(v2);
        random_shader.vertex(v3);
        rasterize(v1, v2, v3, random_shader, framebuffer);
    }
    
    
    TGAImage zbuffer(width, height, TGAImage::Format::GRAYSCALE);
    double min_z = std::numeric_limits<double>::max();
    double max_z = -std::numeric_limits<double>::max();
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            double z = z_buffer[x][y];
            if (z > -1e10) {
                min_z = std::min(min_z, z);
                max_z = std::max(max_z, z);
            }
        }
    }
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            double z = z_buffer[x][y];
            unsigned char c = 0;
            if (z > -1e10) {
                if (max_z == min_z) {
                    c = 255;
                } else {
                    c = static_cast<unsigned char>((z - min_z) / (max_z - min_z) * 255.0);
                }
            }
            zbuffer.set(x, y, {c});
        }
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


