#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <glad/glad.h>
#include <glm/glm.hpp>

class Shader {
public:
    unsigned int ID;

    // VS+FS 持失切
    Shader(const char* vertexPath, const char* fragmentPath);
    // VS+FS+GS 持失切
    Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath);

    void use() const;
    void setBool(const std::string& name, bool      value) const;
    void setInt(const std::string& name, int       value) const;
    void setFloat(const std::string& name, float    value) const;
    void setMat4(const std::string& name, const glm::mat4& mat) const;
};

#endif