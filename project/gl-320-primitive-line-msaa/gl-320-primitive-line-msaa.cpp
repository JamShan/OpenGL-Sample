#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

using namespace std;

#include <shader.h>

Shader primitiveShader("primitive-front-face");
const GLsizei VertexCount(4);
const GLsizeiptr VertexSize = VertexCount * sizeof(glm::vec2);
const glm::vec2 VertexData[VertexCount] = {
	glm::vec2(-1.0f, -1.0f),
	glm::vec2(1.0f, -1.0f),
	glm::vec2(1.0f, 1.0f),
	glm::vec2(-1.0f, 1.0f)
};

const GLsizei ElementCount(6);
const GLsizeiptr ElementSize = ElementCount * sizeof(GLushort);
const GLushort ElementData[ElementCount] = {
	0, 1, 2,
	2, 3, 0
};

namespace Buffer
{
	enum type {
		ELEMENT,
		VERTEX,
		MAX
	};
};

vector<GLuint> buffer(Buffer::MAX);
GLuint vao = 0;
GLuint program = 0;
GLuint mvp_matrix_loc;
GLuint vertex_loc;
const char *vert_shader_code("gl-320-primitive-front-face.vert");
const char *frag_shader_code("gl-320-primitive-front-face.frag");

struct Window {
	char *title;
	int width, height;
	int posX, posY;
	Window(char *t, const int &w, const int &h, const int &px, const int &py):
		title(t), width(w), height(h), posX(px), posY(py){}
};

Window window("primitive-front-face", 720, 640,300, 0);

void load_shader()
{
	primitiveShader.init();
	primitiveShader.attach(GL_VERTEX_SHADER, vert_shader_code);
	primitiveShader.attach(GL_FRAGMENT_SHADER, frag_shader_code);
	primitiveShader.link();
	primitiveShader.use();
	program = primitiveShader.program;
	mvp_matrix_loc = glGetUniformLocation(program, "mvp_matrix");
}

void init_buffer()
{
	glGenBuffers(Buffer::MAX, &buffer[0]);
	
	glBindBuffer(GL_ARRAY_BUFFER, buffer[Buffer::VERTEX]);
	glBufferData(GL_ARRAY_BUFFER, VertexSize, VertexData, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer[Buffer::ELEMENT]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, ElementSize, ElementData, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

}

void init_vertexArray()
{
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, buffer[Buffer::VERTEX]);
	vertex_loc = glGetAttribLocation(program, "Position");
	glVertexAttribPointer(vertex_loc, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), 0);
	glEnableVertexAttribArray(vertex_loc);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void init()
{
	glEnable(GL_CULL_FACE);
	load_shader();
	init_buffer();
	init_vertexArray();
}

void render()
{
	glm::mat4 proj_matrix = glm::perspective(1.0f, static_cast<float>(window.width) / static_cast<float>(window.height), 1.0f, 1000.0f);
	glm::mat4 model_matrix = glm::mat4(1.0f);
	glm::mat4 mvp_matrix = proj_matrix * model_matrix;
	const static glm::vec4 black(0.0f, 0.0f, 0.f, 1.0f);
	glClearBufferfv(GL_COLOR, 0, &black[0]);

	glUseProgram(program);
	glUniformMatrix4fv(mvp_matrix_loc, 1, GL_FALSE, &mvp_matrix[0][0]);
	glBindVertexArray(vao);
	glViewport(0, 0, static_cast<GLsizei>(window.width), static_cast<GLsizei>(window.height));
	glProvokingVertex(GL_LAST_VERTEX_CONVENTION); //Specifies the vertex to be used as the source of data for flat shaded varyings.
	glDrawElementsInstancedBaseVertex(GL_TRIANGLES, ElementCount, GL_UNSIGNED_SHORT, NULL, 1, 0);
}

void end()
{
	glDeleteBuffers(Buffer::MAX, &buffer[0]);
	glDeleteVertexArrays(1, &vao);
	glDeleteProgram(program);
}

int main(int argc, char**argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowPosition(window.posX, window.posY);
	glutInitWindowSize(window.width, window.height);
	glutCreateWindow(window.title);
	GLenum status = glewInit();
	if (status != GLEW_OK)
		cerr << glewGetErrorString(status) << endl;
	init();
	glutDisplayFunc(render);
	glutMainLoop();
	end();
}