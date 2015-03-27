#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/noise.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <iostream>
#include <string>

using namespace std;
#include <shader.h>

#define BUFFER_OFFSET(offset) ((GLvoid*) (NULL + offset))

struct Window {
	char *title;
	int width, height;
	int posX, posY;
	Window(char *t, const int &w, const int &h, const int &px, const int &py) 
		  :title(t), width(w), height(h), posX(px), posY(py){}
};

const char* vert_shader_code("texture-3d.vert");
const char* frag_shader_code("texture-3d.frag");
Window window("texture-3d-test", 720, 640, 300, 0);

const GLsizei vertexCount(12);
const GLsizeiptr vertexSize = vertexCount * sizeof(glm::vec2);
const glm::vec2 vertexData[vertexCount] = {
	glm::vec2(-1.0f, -1.0f), glm::vec2(0.0f, 1.0f),
	glm::vec2(1.0f, -1.0f),  glm::vec2(1.0f, 1.0f),
	glm::vec2(1.0f,  1.0f),  glm::vec2(1.0f, 0.0f),
	glm::vec2(1.0f,  1.0f),  glm::vec2(1.0f, 0.0f),
	glm::vec2(-1.0f, 1.0f),  glm::vec2(0.0f, 0.0f),
	glm::vec2(-1.0f, -1.0f), glm::vec2(0.0f, 1.0f)
};

GLuint vao = 0;
GLuint program = 0;
GLuint vbo, texture;
GLint textureMatrix_loc;
GLint diffuse_loc;
Shader texture3dShader("texture-3d");

void load_shader()
{
	texture3dShader.init();
	texture3dShader.attach(GL_VERTEX_SHADER, vert_shader_code);
	texture3dShader.attach(GL_FRAGMENT_SHADER, frag_shader_code);
	texture3dShader.link();
	texture3dShader.use();
	program = texture3dShader.program;
	textureMatrix_loc = glGetUniformLocation(program, "Orientation");
	diffuse_loc = glGetUniformLocation(program, "diffuse");
}

void init_buffer()
{
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vertexSize,vertexData, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void init_texture()
{
	const size_t size(32);
	vector<float> data(size * size * size);
	for (size_t i = 0; i != size; ++i)
	  for (size_t j = 0; j != size; ++j)
		for (size_t k = 0; k != size; ++k)
			data[k + j * size + i *size * size] = glm::simplex(glm::vec4(k, j, i, 0.0f) 
			                                      / float((size / 8 -1)));
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &texture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, texture);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, int(glm::log2(float(size))));
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glTexImage3D(GL_TEXTURE_3D, 0,
		GL_R32F,
		GLsizei(size),
		GLsizei(size),
		GLsizei(size),
		0, 
		GL_RED, GL_FLOAT,
		&data[0]);
	glGenerateMipmap(GL_TEXTURE_3D);
	glPixelStorei(GL_UNPACK_ALIGNMENT,4);
}

void init_vertexArray()
{
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2) * 2, nullptr);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2) * 2, BUFFER_OFFSET(sizeof(glm::vec2)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);
}

void init()
{
	load_shader();
	init_buffer();
	init_texture();
	init_vertexArray();
}

void end()
{
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
	glDeleteTextures(1, &texture);
	glDeleteProgram(program);
}

void render()
{
   static glm::vec3 orientation(0);
   orientation += glm::vec3(0.02, 0.01, 0.005);
   glm::mat3 texture_matrix = glm::mat3(glm::yawPitchRoll(orientation.x, orientation.y, orientation.z));
   glViewport(0, 0, window.width, window.height);
   glClearBufferfv(GL_COLOR, 0, &glm::vec4(1.0f, 0.5f, 0.0f, 1.0f)[0]);

   glUseProgram(program);
   glUniform1i(diffuse_loc, 0);
   glUniformMatrix3fv(textureMatrix_loc, 1, GL_FALSE, &texture_matrix[0][0]);

   glActiveTexture(GL_TEXTURE0);
   glBindTexture(GL_TEXTURE_3D, texture);
   glBindVertexArray(vao);
   glDrawArraysInstanced(GL_TRIANGLES, 0, vertexCount, 1);
   glutSwapBuffers();
};

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowPosition(window.posX, window.posY);
	glutInitWindowSize(window.width, window.height);
	glutCreateWindow(window.title);
	GLenum err = glewInit();
	if (err = GLEW_OK)
		cerr << glewGetErrorString(err) << endl;
	init();
	glutDisplayFunc(render);
	glutMainLoop();
}
