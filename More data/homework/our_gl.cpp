#include <algorithm>
#include "our_gl.h"


mat<4, 4, double> View, Projection, Viewport;
std::vector<double> zbuffer;

mat<4, 4, double> gl::get_pitch_matrix(double pitch)
{
    return {
        {1,               0,                0, 0},
        {0, std::cos(pitch), -std::sin(pitch), 0},
        {0, std::sin(pitch),  std::cos(pitch), 0},
        {0,               0,                0, 1}
    };
}
mat<4, 4, double> gl::get_yaw_matrix(double yaw)
{
    return {
        { std::cos(yaw), 0, std::sin(yaw), 0},
        {             0, 1,             0, 0},
        {-std::sin(yaw), 0, std::cos(yaw), 0},
        {             0, 0,             0, 1}
    };
}
mat<4, 4, double> gl::get_roll_matrix(double roll)
{
    return {
        {std::cos(roll), -std::sin(roll), 0, 0},
        {std::sin(roll),  std::cos(roll), 0, 0},
        {             0,               0, 1, 0},
        {             0,               0, 0, 1}
    };
}

// 得到Model矩阵，从模型空间到世界空间
mat<4, 4, double> gl::get_model_matrix(Transform transform)
{
    mat<4, 4> P, R, S; // 平移 旋转 缩放矩阵
    P = mat<4, 4, double>::identity(),R = mat<4, 4, double>::identity(),S = mat<4, 4, double>::identity();
    P.set_col(3, embed4(transform.position));
    S[0][0] = transform.scale.x;
    S[1][1] = transform.scale.y;
    S[2][2] = transform.scale.z;
    R = get_pitch_matrix(transform.rotation[0]) // 旋转矩阵 由 pitch yaw roll x/y/z三个轴的旋转组成
      * get_yaw_matrix(transform.rotation[1])
      * get_roll_matrix(transform.rotation[2]);
    return P * R * S;
}

// 得到View矩阵，从世界空间到摄像机空间，摄像机将对准target点，且target与摄像机的连线为新的z轴
// 摄像机空间里，摄像机位于原点，朝向z的负半轴
void gl::lookat(const vec3f eye, const vec3f target, const vec3f up)
{
    vec3f z = (eye - target).normalize();
    vec3f x = up.cross(z).normalize();
    vec3f y = z.cross(x).normalize();     // 得到摄像机坐标系的x/y/z轴

    /*
    View矩阵是将摄像机放到原点，且朝向target，把朝向设为负z轴
    上面已经得到了摄像机的xyz轴
    下面要计算把摄像机移到原点的矩阵transform 和 旋转到正确朝向的矩阵 rotation
    transform 很简单，其平移项就等于 -eye的坐标
    rotation 不好求，但rotation的逆矩阵，也就是从原坐标系到摄像机坐标系的旋转矩阵好求
    我们求出后只要求其逆矩阵就行了
    最后View=rotation * transform 这里是先平移再旋转，因为旋转是沿原点旋转的，先旋转再平移是错的
    */
    mat<4, 4> transform = mat<4, 4>::identity();
    mat<4, 4> rotation = mat<4, 4>::identity();
    transform.set_col(3, embed4(-eye, 1.0));
    rotation.set_col(0, embed4(x, 0.0));
    rotation.set_col(1, embed4(y, 0.0));
    rotation.set_col(2, embed4(z, 0.0)); // 先得到原坐标系旋转到摄像机坐标系的旋转矩阵，再求其逆矩阵
    rotation = transpose(rotation); // 正交矩阵的逆矩阵等于其转置，rotation由三个互相垂直的轴组成，所以正交
    View = rotation * transform;
}

// 得到Projection矩阵 从摄像机空间到标准设备坐标NDC   
// 把近平面为z=-near，远平面为z=-far，视角为fov_y，宽高比为aspect_ratio的视锥全部映射到[-1,1]的立方体
void gl::init_perspective(double near, double far, double fov_y, double aspect_ratio) {
    Projection = {
        // x方向: x_clip = x / (aspect_ratio * tan(fov_y / 2))
        // 最终 x_ndc = x_clip / -z (由第4行触发)，实现了随深度变远的透视缩小
        {1.0 / (aspect_ratio * std::tan(fov_y / 2.0)), 0, 0, 0},    

        // y方向: 与x同理，但不需要除以宽高比
        {0, 1.0 / std::tan(fov_y / 2.0), 0, 0}, 

        // z方向: z_clip = A*z + B。因为最终 z_ndc = (A*z + B) / -z = -A - B/z
        // 为了满足 z=-near 时 z_ndc=-1, z=-far 时 z_ndc=1 的非线性映射关系
        // 即要满足 -1 = -A - B/near;          1 = -A - B/far
        // 解方程得出此处的 A 为 -(far+near)/(far-near)，B 为 -2*near*far/(far-near)
        {0, 0, -(far + near) / (far - near), -(2.0 * near * far) / (far - near)}, 

        // -1 使转换后的坐标的w值乘上了-z
        // 按照齐次坐标的定义，就是把x/y/z轴统一除了-z
        {0, 0, -1, 0} 
    };
}


// 视口转换 从NDC[-1,1] 到 屏幕坐标 [0, height - 1], [0, width - 1]
void gl::init_viewport(const int x, const int y, const int w, const int h) {
    Viewport = {
        {w / 2.0,       0, 0, x + w / 2.0}, 
        {      0, h / 2.0, 0, y + h / 2.0}, 
        {      0,       0, 1,           0}, 
        {      0,       0, 0,           1}
    };
}

void gl::init_zbuffer(const int width, const int height) {
    zbuffer = std::vector<double>(width * height, std::numeric_limits<double>::max());
}

vec4f gl::embed4(const vec3f& v, double fill) {
    return vec4f(v.x, v.y, v.z, fill);
}


vec3f gl::proj3(const vec4f& v) {
    return vec3f(v.x / v.w, v.y / v.w, v.z / v.w);
}

template<typename T>
double area(const vec<2,T>& a, const vec<2,T>& b, const vec<2,T>& c) 
{
    return .5 * ((a.x * b.y - b.x * a.y) + (b.x * c.y - c.x * b.y) + (c.x * a.y - a.x * c.y));
}

constexpr double epsilon = -1e-5;

void gl::rasterize(const vec4f& v1, const vec4f& v2, const vec4f& v3, const IShader &shader, TGAImage &framebuffer)
{
    vec4f ndc[3] = {v1 / v1.w, v2 / v2.w, v3 / v3.w};
    
    vec2f sreen[3] = {(Viewport * ndc[0]).xy(), (Viewport * ndc[1]).xy(), (Viewport * ndc[2]).xy()};

    int width = framebuffer.width();
    int height = framebuffer.height();
    
    int minX = std::max(0, (int)std::floor(std::min({sreen[0].x, sreen[1].x, sreen[2].x})));
    int minY = std::max(0, (int)std::floor(std::min({sreen[0].y, sreen[1].y, sreen[2].y})));
    int maxX = std::min(width - 1, (int)std::ceil(std::max({sreen[0].x, sreen[1].x, sreen[2].x})));
    int maxY = std::min(height - 1, (int)std::ceil(std::max({sreen[0].y, sreen[1].y, sreen[2].y})));

    double st = area(sreen[0], sreen[1], sreen[2]);
    if (st < epsilon) return;

    for (int x = minX; x <= maxX; x++) 
    {
        for (int y = minY; y <= maxY; y++) 
        {
            vec2f p{(double)x + 0.5, (double)y + 0.5};
            
            double a = area(sreen[1], sreen[2], p) / st;
            double b = area(sreen[2], sreen[0], p) / st;
            double c = area(sreen[0], sreen[1], p) / st;
            
            if (a < epsilon || b < epsilon || c < epsilon) continue;
            
            double z = a * ndc[0].z + b * ndc[1].z + c * ndc[2].z;
            
            if (z < zbuffer[x + y * width]) 
            {
                double weight_a = a / v1.w;
                double weight_b = b / v2.w;
                double weight_c = c / v3.w;
                double Z = weight_a + weight_b + weight_c;
                
                double persp_a = weight_a / Z;
                double persp_b = weight_b / Z;
                double persp_c = weight_c / Z;
                
                auto [flag, color] = shader.fragment({persp_a, persp_b, persp_c});
                if (!flag) continue;
                zbuffer[x + y * width] = z;
                framebuffer.set(x, y, color);
            }
        }
    }
}
