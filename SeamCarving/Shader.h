#include <glad/glad.h>

#include <vector>
#include <string>

class Shader 
{
private:
	GLuint id = 0;
	Shader(GLuint id);

public:

	Shader() = default;

	static Shader load(const std::string &file, GLuint type);

	GLuint getId();
};

class Program
{
private:
	GLuint id = 0;
	Program(GLuint id);

public:

	Program() = default;

	Program(std::vector<Shader> shaders);
	static Program load(std::string file);

	void use();

	void setUniformFloat(const std::string& name, float val);
	void setUniformInt(const std::string& name, int val);
};