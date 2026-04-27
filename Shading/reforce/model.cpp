#include "model.h"
#include <fstream>
#include <sstream>


Model::Model(const std::string filename)
{
    std::ifstream file;
    file.open(filename);
    if (file.is_open())
    {
        std::string line;
        while(std::getline(file, line))
        {
            if (line.empty()) continue;
            std::istringstream iss(line);
            char type;
            iss >> type;
            switch (type)
            {
                case 'v':
                {
                    vec3f vert;
                    iss >> vert.x >> vert.y >> vert.z;
                    verts.push_back(vert);
                    break;
                }
                case 'f':
                {
                    vec3f triangle;
                    char dummy_c;
                    int dummy;
                    iss >> triangle.data[0] >> dummy_c >> dummy >> dummy_c >> dummy;
                    iss >> triangle.data[1] >> dummy_c >> dummy >> dummy_c >> dummy;
                    iss >> triangle.data[2] >> dummy_c >> dummy >> dummy_c >> dummy;
                    triangles.push_back(triangle);
                    break;
                }
                default:
                    break;
            }
        }
        file.close();
    }   
}
