#include <cmath>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <iostream>
#include <algorithm>

#include "tgaimage.h"
#include "model.h"
#include "our_gl.h"
#include "light.h"

constexpr double PI = 3.1415926;

extern mat<4, 4> View, Projection;
extern std::vector<double> zbuffer;

constexpr TGAColor white   = {255, 255, 255, 255};
constexpr TGAColor green   = {  0, 255,   0, 255};
constexpr TGAColor red     = {  0,   0, 255, 255};
constexpr TGAColor blue    = {255, 128,  64, 255};
constexpr TGAColor yellow  = {  0, 200, 255, 255};

TGAColor operator* (const double& d, const TGAColor& c)  { 
    return {static_cast<std::uint8_t>(std::min(255.0, c.bgra[0] * d)), // B -> bgra[0]
            static_cast<std::uint8_t>(std::min(255.0, c.bgra[1] * d)), // G -> bgra[1]
            static_cast<std::uint8_t>(std::min(255.0, c.bgra[2] * d)), // R -> bgra[2]
            255};
}
TGAColor operator* (const TGAColor& c, const double& d)  { return d * c;}
TGAColor operator+ (const TGAColor& c, const TGAColor& d)  { 
    return {static_cast<std::uint8_t>(std::min(255, c.bgra[0] + d.bgra[0])), 
            static_cast<std::uint8_t>(std::min(255, c.bgra[1] + d.bgra[1])), 
            static_cast<std::uint8_t>(std::min(255, c.bgra[2] + d.bgra[2])), 255};
}
TGAColor get_rand_color() {return {static_cast<std::uint8_t>(rand()%255), static_cast<std::uint8_t>(rand()%255), static_cast<std::uint8_t>(rand()%255), static_cast<std::uint8_t>(rand()%255)};}

double deg2rad(double deg) {return deg * PI / 180.0; }
double rad2deg(double rad) {return rad * 180 / PI; }

struct RandomShader : gl::IShader {
    const Model& _model;
    TGAColor color;
    RandomShader(const Model &model) : _model(model) {}
    virtual vec4f vertex(int face, vec4f& v)  {
        return v = Projection * View
        * gl::get_model_matrix(_model.get_transform()) * v; // MVP 变换
    }
    virtual std::pair<bool,TGAColor> fragment(const vec3f bar) const {
        return {true, color};
    };
};

struct PhongShader : gl::IShader {
    const Model& _model;
    mutable vec3f verts[3];
    mutable vec3f norms[3];
    mutable vec2f uvs[3];
    mat<4,4,double> ModelMatrix;
    
    std::vector<Light> lights;
    vec3f eye_pos;

    double kd = 1.0, ks = 0.15, p = 6;

    PhongShader(const Model& model, vec3f eye_pos_ , std::vector<Light> lights_, double d = 1.0, double s = 0.15, double p_ = 6) 
        : _model(model), eye_pos(eye_pos_), lights(lights_),
         kd(d), ks(s), p(p_) {}

    virtual vec4f vertex(int face, vec4f& v) override {
        static int cnt = 0;
        ModelMatrix = gl::get_model_matrix(_model.get_transform());
        vec4f v_world = ModelMatrix * v;
        verts[cnt] = (v_world / v_world.w).xyz();

        norms[cnt] = (ModelMatrix * _model.get_triangle_norm(face, cnt)).xyz();

        uvs[cnt] = _model.get_triangle_uv(face, cnt);

        cnt = (cnt + 1) % 3;
        return v = Projection * View * v_world;
    }

    virtual std::pair<bool,TGAColor> fragment(const vec3f bar) const override 
    {
        vec2f uv = uvs[0] * bar[0] + uvs[1] * bar[1] + uvs[2] * bar[2]; 
        vec3f position = verts[0] * bar[0] + verts[1] * bar[1] + verts[2] * bar[2]; 

        int tex_x = std::max(0, std::min(_model.get_texture().width() - 1,  (int)(uv.x * _model.get_texture().width())));
        int tex_y = std::max(0, std::min(_model.get_texture().height() - 1, (int)((1.0 - uv.y) * _model.get_texture().height())));
        TGAColor tex_color = _model.get_texture().get(tex_x, tex_y);
        
        // vec3f normal = norms[0] * bar[0] + norms[1] * bar[1] + norms[2] * bar[2];
        TGAColor norm_tex = _model.get_normal_tex().get(tex_x, tex_y);
        vec3f normal = {(double)norm_tex.bgra[2], (double)norm_tex.bgra[1], (double)norm_tex.bgra[0]};
        normal = normal / 127.5 - vec3f{1.0, 1.0, 1.0};
        normal = (transpose(invert(ModelMatrix)) * gl::embed4(normal, 0)).xyz();
        normal = normal.normalized();
        TGAColor color = {0, 0, 0};
        for (auto light : lights)
        {
            vec3f l = (light.transform.position - position).normalize();
            double r = (light.transform.position - position).length();

            double n_dot_l = normal.dot(l);
            // 漫反射
            TGAColor Ld = tex_color * (std::max(0.0, n_dot_l) * (light.intensity / (r * r)));

            // 高光
            TGAColor Ls = {0, 0, 0, 255};
            if (n_dot_l > 0.0) 
            {
                vec3f view_dir = (eye_pos - position).normalize();
                vec3f h = (view_dir + l).normalize();
                Ls = light.color * (std::pow(std::max(0.0, h.dot(normal)), p) * ks * (light.intensity / (r * r)));
            }
            color = color + Ld + Ls;
        }
        
        return {true, color};
    }
};

int main(int argc, char** argv) 
{
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " <obj_file>" << std::endl;
        return 1;
    }

    constexpr int width  = 2000;
    constexpr int height = 2000;
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

    for (int i = 1; i < argc; i += 3)
    {    
        TGAImage texture;
        TGAImage normal_tex;
        texture.read_tga_file(argv[i + 1]);
        normal_tex.read_tga_file(argv[i + 2]);

        Model model(argv[i], texture, normal_tex);
        model.set_position({0.0, 0.0, 0.0});
        model.set_scale({1.0, 1.0, 1.0});
        model.set_rotation({deg2rad(0.0), deg2rad(30.0), deg2rad(0.0)});

        std::vector<Light> lights;
        lights.emplace_back(white, 8, Transform({0, 2., 2}, {0, 0, 0}, {0, 0, 0}));
        lights.emplace_back(red, 10, Transform({-3, 2., 2}, {0, 0, 0}, {0, 0, 0}));
        PhongShader shader(model, eye, lights, 1.0, 0.4, 100);
        for (int i = 0; i < model.get_triangles_size(); i++) {
            auto [v1, v2, v3] = model.get_triangle_verts(i);
            shader.vertex(i, v1);
            shader.vertex(i, v2);
            shader.vertex(i, v3);
            // shader.color = get_rand_color();
            gl::rasterize(v1, v2, v3, shader, framebuffer);
        }
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