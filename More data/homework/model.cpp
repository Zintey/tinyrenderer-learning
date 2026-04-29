#include "model.h"
#include <fstream>
#include <sstream>

// 解析 OBJ 的一个顶点token，格式可以是：
//   v        (只有顶点)
//   v/vt     (顶点+UV)
//   v//vn    (顶点+法线，无UV)
//   v/vt/vn  (顶点+UV+法线)
// 返回 {vert_idx, uv_idx, norm_idx}，索引为0表示不存在
static std::tuple<int, int, int> parse_face_token(const std::string& token)
{
    int v = 0, vt = 0, vn = 0;
    size_t first = token.find('/');
    if (first == std::string::npos) {
        // 只有顶点
        v = std::stoi(token);
        return {v, 0, 0};
    }
    v = std::stoi(token.substr(0, first));
    size_t second = token.find('/', first + 1);
    if (second == std::string::npos) {
        // v/vt
        vt = std::stoi(token.substr(first + 1));
        return {v, vt, 0};
    }
    // v//vn 或 v/vt/vn
    if (second != first + 1) {
        vt = std::stoi(token.substr(first + 1, second - first - 1));
    }
    if (second + 1 < token.size()) {
        vn = std::stoi(token.substr(second + 1));
    }
    return {v, vt, vn};
}

Model::Model(const std::string filename, const TGAImage& texture, const TGAImage& normal_tex)
    : _texture(texture), _normal_tex(normal_tex)
{
    std::ifstream file;
    file.open(filename);
    if (file.is_open())
    {
        std::string line;
        while(std::getline(file, line))
        {
            if (line.empty()) continue;
            if (!line.empty() && line.back() == '\r') line.pop_back();

            size_t p = line.find(' ');
            if (p == std::string::npos) continue;
            std::string type = line.substr(0, p);
            std::istringstream iss(line.substr(p + 1));

            if (type == "v")
            {
                vec3f vert;
                iss >> vert.x >> vert.y >> vert.z;
                verts.push_back(vert);
            }
            else if (type == "vn")
            {
                vec3f norm;
                iss >> norm.x >> norm.y >> norm.z;
                normals.push_back(norm);
            }
            else if (type == "vt")
            {
                vec2f uv;
                iss >> uv.x >> uv.y;
                uvs.push_back(uv);
            }
            else if (type == "f")
            {
                std::string t0, t1, t2;
                iss >> t0 >> t1 >> t2;
                if (t0.empty() || t1.empty() || t2.empty()) continue;

                auto [v0, vt0, n0] = parse_face_token(t0);
                auto [v1, vt1, n1] = parse_face_token(t1);
                auto [v2, vt2, n2] = parse_face_token(t2);

                triangles_vert.push_back({v0, v1, v2});
                triangles_uv.push_back({vt0, vt1, vt2}); // 记录面的 UV 索引
                triangles_norm.push_back({n0, n1, n2});
            }
        }
        file.close();
    }
}