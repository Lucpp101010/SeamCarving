#include "Vao.h"

Vao::Vao(const std::vector<Vertex> &v)
{
	glGenVertexArrays(1, &id);
	glBindVertexArray(id);
	glGenBuffers(1, &buf);
	glBindBuffer(GL_ARRAY_BUFFER, buf);
	glBufferData(GL_ARRAY_BUFFER, v.size() * sizeof(Vertex), &(v[0]), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), NULL);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

GLuint Vao::getId()
{
	return id;
}

Vao Vao::createRect()
{
	std::vector<Vertex> vertices = {
		{-1, -1, 0, 1},
		{1, -1, 1, 1} ,
		{-1, 1, 0, 0} ,
		{-1, 1, 0, 0} ,
		{1, 1, 1, 0}  ,
		{1, -1, 1, 1}
	};
	return Vao(vertices);
}

Vao Vao::createCircle(int points)
{
	std::vector<Vertex> v(points);
	for (int i = 0; i < points; ++i)
	{
		float x = cos(M_PI * 2 * (float)i / points);
		float y = sin(M_PI * 2 * (float)i / points);
		v[i] = { x, y, (x + 1) / 2, (y + 1) / 2 };
	}
	return Vao(v);
}
