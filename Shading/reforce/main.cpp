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

double deg2rad(double deg) {return deg * PI / 180.0; }
double rad2deg(double rad) {return rad * 180 / PI; }

struct RandomShader : gl::IShader {
    const Model& _model;
    TGAColor color;
    RandomShader(const Model &model) : _model(model) {}
    virtual vec4f vertex(vec4f& v) const {
        return v = Projection * View
        * gl::get_model_matrix(_model.get_transform()) * v; // MVP 变换
    }
    virtual std::pair<bool,TGAColor> fragment(const vec3f bar) const {
        return {true, color};
    };
};

struct FlatShader : gl::IShader {
    const Model& _model;
    
    // 缓存当前三角形的数据，vertex()写入，fragment()读取
    mutable vec3f world_verts[3];   // 世界空间顶点（用于算面法线）
    mutable int   vert_idx = 0;     // 当前处理到第几个顶点
    
    const vec3f light_dir = vec3f{-1, 0, 2}.normalized();
    
    FlatShader(const Model& model) : _model(model) {}
    
    virtual vec4f vertex(vec4f& v) const override {
        world_verts[vert_idx++] = vec3f{v.x, v.y, v.z};
        if (vert_idx >= 3) vert_idx = 0;
        return v = Projection * View * gl::get_model_matrix(_model.get_transform()) * v;
    }
    
    virtual std::pair<bool, TGAColor> fragment(const vec3f bar) const override {
                    static int count = 0;
    if (count < 3) {
        std::cout << "world_verts[0]: " << world_verts[0] << std::endl;
        std::cout << "world_verts[1]: " << world_verts[1] << std::endl;
        std::cout << "world_verts[2]: " << world_verts[2] << std::endl;
        vec3f edge1 = world_verts[1] - world_verts[0];
        vec3f edge2 = world_verts[2] - world_verts[0];
        vec3f normal = edge1.cross(edge2).normalized();
        std::cout << "normal: " << normal << std::endl;
        std::cout << "light_dir: " << light_dir << std::endl;
        std::cout << "intensity: " << normal.dot(light_dir) << std::endl;
        count++;
    }
        vec3f edge1 = world_verts[1] - world_verts[0];
        vec3f edge2 = world_verts[2] - world_verts[0];
        vec3f normal = edge1.cross(edge2).normalized();
        
        double intensity = std::abs(normal.dot(light_dir));  // 正反面都能被照亮
        auto c = static_cast<uint8_t>(255 * intensity);
        return {true, {c, c, c, 255}};
    }
};


int main(int argc, char** argv) 
{
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <obj_file>" << std::endl;
        return 1;
    }

    constexpr int width  = 1920;
    constexpr int height = 1080;
    constexpr double aspect_radio = (double)width / height;
    const vec3f    eye{-1, 0, 2};
    const vec3f target{ 0, 0, 0};
    const vec3f     up{ 0, 1, 0};

    gl::lookat(eye, target, up);
    double near = 10, far = 300, fov_y = 60.0 * PI / 180.0;
    gl::init_perspective(near, far, fov_y, aspect_radio);
    gl::init_viewport(width/16, height/16, width*7/8, height*7/8);
    gl::init_zbuffer(width, height);

    TGAImage framebuffer(width, height, TGAImage::RGB);
    auto start = std::chrono::steady_clock::now();

    std::srand(std::time(nullptr));

    Model model(argv[1]);
    model.set_position({0.0, 0.0, -2.0});
    model.set_scale({2.0, 2.0, 2.0});
    model.set_rotation({deg2rad(0.0), deg2rad(0.0), deg2rad(0.0)});
    // FlatShader shader(model); 
    // for (int i = 0; i < model.get_triangles_size(); i++) 
    // {
    //     auto [v1, v2, v3] = model.get_triangle(i);
    //     shader.vertex(v1);   // 顶点Shader MVP变换
    //     shader.vertex(v2);
    //     shader.vertex(v3);
    //     // shader.color = get_rand_color();
    //     rasterize(v1, v2, v3, shader, framebuffer);
    // }

    FlatShader shader(model);
    for (int i = 0; i < model.get_triangles_size(); i++) {
        auto [v1, v2, v3] = model.get_triangle(i);
        shader.vertex(v1);
        shader.vertex(v2);
        shader.vertex(v3);
        gl::rasterize(v1, v2, v3, shader, framebuffer);
    }
    
    
    TGAImage zbuffer_img(width, height, TGAImage::Format::GRAYSCALE);
    double min_z = std::numeric_limits<double>::max();
    double max_z = -std::numeric_limits<double>::max();
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            double z = zbuffer[x + y * width];
            if (z > -1000.0 + 1e-6) {      
                min_z = std::min(min_z, z);
                max_z = std::max(max_z, z);
            }
        }
    }
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            double z = zbuffer[x + y * width];  
            unsigned char c = 0;
            if (z > -1000.0 + 1e-6) {
                c = (max_z == min_z) ? 255 
                : static_cast<unsigned char>((z - min_z) / (max_z - min_z) * 255.0);
            }
            zbuffer_img.set(x, y, {c});
        }
    }
            
        
    auto end = std::chrono::steady_clock::now();
    framebuffer.write_tga_file("framebuffer.tga");
    zbuffer_img.write_tga_file("z_buffer.tga");
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() << " ms" << std::endl;
    
    system("start framebuffer.tga");
    system("start z_buffer.tga");
    return 0;
}


