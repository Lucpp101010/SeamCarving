#include <glad/glad.h>
#include <vector>

#define _USE_MATH_DEFINES
#include <math.h>

class Vertex
{
public:
	float x, y, tx, ty;
};

class Vao
{
private:
	GLuint id;
	GLuint buf;

public:

	Vao() = default;
	Vao(const std::vector<Vertex> &v);

	GLuint getId();

	static Vao createRect();
	static Vao createCircle(int points);
};