#include <GL/glew.h>
#include <GL/freeglut.h>
#include <string>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <array>

using namespace std;

#include <shader.h>

struct Window {
	Window(char *t, const int &w, const int &h, const int &px, const int &py)
		:title(t), width(w), height(h), posX(px), posY(py){}
	char * title;
	int width, height;
	int posX, posY;
};

namespace Buffer
{
   enum type {
	   TRANSFORM,
	   MAX,
   };
}
array<GLuint, Buffer::MAX> buffer;
Window window("without-vertex-attrib", 720, 640, 300, 0);
Shader attribShader("without-vertex-attrib");
const char *vert_shader_code("draw-without-vertex-attrib.vert");
const char *frag_shader_code("draw-without-vertex-attrib.frag");

GLuint program, vao;
void load_shader()
{
	attribShader.init();
	attribShader.attach(GL_VERTEX_SHADER, vert_shader_code);
	attribShader.attach(GL_FRAGMENT_SHADER, frag_shader_code);
	attribShader.link();
	attribShader.use();
	program = attribShader.program;
	glUniformBlockBinding(program, glGetUniformBlockIndex(program, "transform"), 0); 
}

void init_buffer()
{
	glGenBuffers(Buffer::MAX, &buffer[0]);
	GLint UniformBufferOffset(0);
	glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &UniformBufferOffset);
	GLint UniformBlockSize = glm::max(GLint(sizeof(glm::mat4)), UniformBufferOffset);
	glBindBuffer(GL_UNIFORM_BUFFER, buffer[Buffer::TRANSFORM]);
	glBufferData(GL_UNIFORM_BUFFER, UniformBlockSize, NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void init_vertexArray()
{
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glBindVertexArray(0);
}

void init()
{
	load_shader();
	init_buffer();
	init_vertexArray();
}

void render()
{
	glBindBuffer(GL_UNIFORM_BUFFER, buffer[Buffer::TRANSFORM]);
	glm::mat4* pointer = (glm::mat4*) glMapBufferRange(
		GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4) * 1,
		GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
	glm::mat4 project_matrix = glm::perspective(glm::pi<float>() * 0.25f, static_cast<float>(window.width) / static_cast<float>(window.height), 0.1f, 100.0f);
	glm::mat4 model = glm::mat4(1.0f);
	glm::mat4 view = glm::lookAt(glm::vec3(0.0, 0.0, 5.0), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
	*pointer = project_matrix * view * model;
	glUnmapBuffer(GL_UNIFORM_BUFFER);
	glViewport(0, 0, static_cast<GLsizei>(window.width), static_cast<GLsizei>(window.height));
	
	const static float depth(1.0f);
	glClearBufferfv(GL_DEPTH, 0, &depth);
	glClearBufferfv(GL_COLOR, 0, &glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)[0]);
	glUseProgram(program);
	glBindVertexArray(vao);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, buffer[Buffer::TRANSFORM]);
	glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 1);
	glutSwapBuffers();
	glutPostRedisplay();
}


int main(int argc, char ** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowPosition(window.posX, window.posY);
	glutInitWindowSize(window.width, window.height);
	glutCreateWindow(window.title);
	GLenum status = glewInit();
	if (status != GLEW_OK)
		cerr << glewGetErrorString(status) << endl;
	init();
	glutDisplayFunc(render);
	glutMainLoop();
	return 0;
}
	
