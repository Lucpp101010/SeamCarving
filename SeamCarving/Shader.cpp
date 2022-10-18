#include "Shader.h"
#include <fstream>
#include <sstream>
#include <iostream>

// util function
void checkCompileErrors(unsigned int shader, std::string file)
{
    int success;
    char infoLog[1024];
    if (file != "PROGRAM")
    {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::SHADER_COMPILATION_ERROR. file: " << file << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
    else
    {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << file << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
}

Shader::Shader(GLuint id) : id(id) {}

Shader Shader::load(const std::string &file, GLuint type)
{
	std::ifstream f;
	f.open(file);
	std::stringstream ss;
	ss << f.rdbuf();
	std::string str = ss.str();
	const char* cstr = str.c_str();
	
	GLuint id = glCreateShader(type);
	glShaderSource(id, 1, &cstr, NULL);
	glCompileShader(id);
    checkCompileErrors(id, file);
    return Shader(id);
}

GLuint Shader::getId()
{
    return id;
}


Program::Program(GLuint id) : id(id) {}

Program::Program(std::vector<Shader> shaders)
{
    id = glCreateProgram();
    for (Shader& s : shaders)
        glAttachShader(id, s.getId());
    glLinkProgram(id);
    checkCompileErrors(id, "PROGRAM");
}

Program Program::load(std::string file)
{
    Shader v = Shader::load(file + ".vert", GL_VERTEX_SHADER);
    Shader f = Shader::load(file + ".frag", GL_FRAGMENT_SHADER);
    GLuint id = glCreateProgram();
    glAttachShader(id, v.getId());
    glAttachShader(id, f.getId());
    glLinkProgram(id);
    checkCompileErrors(id, "PROGRAM");
    return Program(id);
}

void Program::use()
{
    glUseProgram(id);
}

void Program::setUniformFloat(const std::string& name, float val)
{
    int loc = glGetUniformLocation(id, name.c_str());
    glUniform1f(loc, val);
}

void Program::setUniformInt(const std::string& name, int val)
{
    int loc = glGetUniformLocation(id, name.c_str());
    glUniform1i(loc, val);
}
