#include <learnopengl/shader.h>
#include <glad/glad.h>
#include <fstream>
#include <sstream>
#include <iostream>

static void checkCompileErrors(unsigned int id, const std::string& type) {
    int success;
    char infoLog[1024];
    if (type != "PROGRAM") {
        glGetShaderiv(id, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(id, 1024, NULL, infoLog);
            std::cerr << "ERROR::SHADER_COMPILATION_ERROR of type: " << type
                << "\n" << infoLog
                << "\n -- --------------------------------------------------- -- "
                << std::endl;
        }
    }
    else {
        glGetProgramiv(id, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(id, 1024, NULL, infoLog);
            std::cerr << "ERROR::PROGRAM_LINKING_ERROR of type: " << type
                << "\n" << infoLog
                << "\n -- --------------------------------------------------- -- "
                << std::endl;
        }
    }
}

// VS + FS
Shader::Shader(const char* vPath, const char* fPath) {
    std::string vCode, fCode;
    std::ifstream vFile(vPath), fFile(fPath);
    std::stringstream vSS, fSS;
    vSS << vFile.rdbuf(); fSS << fFile.rdbuf();
    vCode = vSS.str(); fCode = fSS.str();
    const char* vSrc = vCode.c_str();
    const char* fSrc = fCode.c_str();

    unsigned int v = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(v, 1, &vSrc, NULL);
    glCompileShader(v);
    checkCompileErrors(v, "VERTEX");

    unsigned int f = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(f, 1, &fSrc, NULL);
    glCompileShader(f);
    checkCompileErrors(f, "FRAGMENT");

    ID = glCreateProgram();
    glAttachShader(ID, v);
    glAttachShader(ID, f);
    glLinkProgram(ID);
    checkCompileErrors(ID, "PROGRAM");

    glDeleteShader(v);
    glDeleteShader(f);
}

// VS + FS + GS
Shader::Shader(const char* vPath, const char* fPath, const char* gPath) {
    std::string vCode, fCode, gCode;
    std::ifstream vFile(vPath), fFile(fPath), gFile(gPath);
    std::stringstream vSS, fSS, gSS;
    vSS << vFile.rdbuf(); fSS << fFile.rdbuf(); gSS << gFile.rdbuf();
    vCode = vSS.str(); fCode = fSS.str(); gCode = gSS.str();
    const char* vSrc = vCode.c_str();
    const char* fSrc = fCode.c_str();
    const char* gSrc = gCode.c_str();

    unsigned int v = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(v, 1, &vSrc, NULL);
    glCompileShader(v); checkCompileErrors(v, "VERTEX");

    unsigned int f = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(f, 1, &fSrc, NULL);
    glCompileShader(f); checkCompileErrors(f, "FRAGMENT");

    unsigned int g = glCreateShader(GL_GEOMETRY_SHADER);
    glShaderSource(g, 1, &gSrc, NULL);
    glCompileShader(g); checkCompileErrors(g, "GEOMETRY");

    ID = glCreateProgram();
    glAttachShader(ID, v);
    glAttachShader(ID, f);
    glAttachShader(ID, g);
    glLinkProgram(ID); checkCompileErrors(ID, "PROGRAM");

    glDeleteShader(v);
    glDeleteShader(f);
    glDeleteShader(g);
}

void Shader::use() const { glUseProgram(ID); }
void Shader::setBool(const std::string& n, bool v)   const { glUniform1i(glGetUniformLocation(ID, n.c_str()), (int)v); }
void Shader::setInt(const std::string& n, int  v)   const { glUniform1i(glGetUniformLocation(ID, n.c_str()), v); }
void Shader::setFloat(const std::string& n, float v)  const { glUniform1f(glGetUniformLocation(ID, n.c_str()), v); }
void Shader::setMat4(const std::string& n, const glm::mat4& m) const {
    glUniformMatrix4fv(glGetUniformLocation(ID, n.c_str()), 1, GL_FALSE, &m[0][0]);
}