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

TGAColor operator* (const double& d, const TGAColor& c)  { 
    return {static_cast<std::uint8_t>(std::min(255.0, c.bgra[2] * d)),
            static_cast<std::uint8_t>(std::min(255.0, c.bgra[1] * d)),
            static_cast<std::uint8_t>(std::min(255.0, c.bgra[0] * d)), 255};
}
TGAColor operator* (const TGAColor& c, const double& d)  { return d * c;}
TGAColor operator+ (const TGAColor& c, const TGAColor& d)  { 
    return {static_cast<std::uint8_t>(std::min(255, c.bgra[2] + d.bgra[2])), 
            static_cast<std::uint8_t>(std::min(255, c.bgra[1] + d.bgra[1])), 
            static_cast<std::uint8_t>(std::min(255, c.bgra[0] + d.bgra[0])), 255};
}
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

struct PhongShader : gl::IShader {
    const Model& _model;
    mutable vec3f tri[3];
    const vec3f light_dir = {0, 0.5, 1.0}; 
    const double light_intensity = 1.0; 
    const double shininess = 128.0; 

    PhongShader(const Model& model) : _model(model) {}

    virtual vec4f vertex(vec4f& v) const override {
        vec4f v_world = gl::get_model_matrix(_model.get_transform()) * v;
        static int cnt = 0;
        tri[cnt++] = (v_world / v_world.w).xyz(); 
        if(cnt == 3) cnt = 0;
        return v = Projection * View * v_world;
    }

    virtual std::pair<bool,TGAColor> fragment(const vec3f bar) const override {
        TGAColor ambient = {20, 20, 20}; 
        TGAColor diffuse = {255, 255, 255};
        TGAColor specular = {255, 255, 255};
        double a = 1.0, d = 0.0, s = 0.0;

        vec3f normal = ((tri[1] - tri[0]).cross(tri[2] - tri[1])).normalize();
        vec3f l = light_dir.normalized();
        double n_dot_l = normal.dot(l);

        d = std::max(0.0, n_dot_l * light_intensity);

        if (n_dot_l > 0.0) {
            vec3f center = (tri[0] + tri[1] + tri[2]) / 3.0;
            vec3f eye = {0.5, 0.0, 2.0}; 
            vec3f view_dir = (eye - center).normalize();
            vec3f reflect_dir = (normal * (n_dot_l * 2.0) - l).normalize();
            
            s = std::pow(std::max(0.0, reflect_dir.dot(view_dir)), shininess) * light_intensity * 0.6;
        }

        TGAColor color = ambient * a + diffuse * d + specular * s;
        return {true, color};
    }
};

int main(int argc, char** argv) 
{
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <obj_file>" << std::endl;
        return 1;
    }

    constexpr int width  = 2500;
    constexpr int height = 2500;
    constexpr double aspect_radio = (double)width / height;
    const vec3f    eye{0.5, 0, 2};
    const vec3f target{ 0, 0, 0};
    const vec3f     up{ 0, 1, 0};

    gl::lookat(eye, target, up);
    double near = 0.1, far = 20.0, fov_y = 60.0 * PI / 180.0; 
    gl::init_perspective(near, far, fov_y, aspect_radio);
    gl::init_viewport(width/16, height/16, width*7/8, height*7/8);
    gl::init_zbuffer(width, height);

    TGAImage framebuffer(width, height, TGAImage::RGB);
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            framebuffer.set(x, y, {50, 50, 50, 255});
        }
    }

    auto start = std::chrono::steady_clock::now();

    std::srand(std::time(nullptr));

    Model model(argv[1]);
    model.set_position({0.0, 0.0, 0.0});
    model.set_scale({1.0, 1.0, 1.0});
    model.set_rotation({deg2rad(0.0), deg2rad(30.0), deg2rad(0.0)});

    PhongShader shader(model);
    for (int i = 0; i < model.get_triangles_size(); i++) {
        auto [v1, v2, v3] = model.get_triangle(i);
        shader.vertex(v1);
        shader.vertex(v2);
        shader.vertex(v3);
        // shader.color = get_rand_color();
        gl::rasterize(v1, v2, v3, shader, framebuffer);
    }
    
    TGAImage zbuffer_img(width, height, TGAImage::Format::GRAYSCALE);
    double min_z = std::numeric_limits<double>::max();
    double max_z = -std::numeric_limits<double>::max();
    
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            double z = zbuffer[x + y * width];
            if (z < std::numeric_limits<double>::max() - 1e-6) {      
                min_z = std::min(min_z, z);
                max_z = std::max(max_z, z);
            }
        }
    }
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            double z = zbuffer[x + y * width];  
            unsigned char c = 0;
            if (z < std::numeric_limits<double>::max() - 1e-6) {
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
    // system("start z_buffer.tga");
    return 0;
}