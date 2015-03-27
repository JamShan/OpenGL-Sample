#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <shader.h>
#include <vector>

using std::vector;

Shader programShader("program");

namespace Buffer {
	enum type {
		VERTEX,
		ELEMENT,
		TRANSFORM,
		MATERIAL,
		MAX
	};
}

vector<GLuint> buffer(Buffer::MAX);

GLuint program, vao;
GLuint vertex_loc, uniform_material_loc, uniform_transform_loc;
GLuint material_loc = 3, transform_loc = 4;

const GLsizei VertexCount(4);
const GLsizei VertexSize = VertexCount * sizeof(glm::vec2);
static const glm::vec2 VertexData[VertexCount] = {
	  glm::vec2(-1.0f, -1.0f),
	  glm::vec2(1.0f, -1.0f),
	  glm::vec2(1.0f, 1.0f),
	  glm::vec2(-1.0f, 1.0f)
};

GLsizei const ElementCount(6);
GLsizeiptr const ElementSize = ElementCount * sizeof(GLushort);
static const GLushort ElementData[ElementCount] = {
	  0, 1, 2,
	  2, 3, 0
};

void init_shader()
{
	programShader.init();
	programShader.attach(GL_VERTEX_SHADER, "gl-320-program.vert");
	programShader.attach(GL_FRAGMENT_SHADER, "gl-320-program.frag");
	programShader.link();
	programShader.use();
	program = programShader.program;
	uniform_material_loc = glGetUniformBlockIndex(program, "material");
	uniform_transform_loc = glGetUniformBlockIndex(program, "transform");
}

void init_buffer()
{
	glGenBuffers(Buffer::MAX, &buffer[0]);
	
	glBindBuffer(GL_ARRAY_BUFFER, buffer[Buffer::VERTEX]); //���붥�����ݵ��������
	glBufferData(GL_ARRAY_BUFFER, VertexSize, VertexData, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer[Buffer::ELEMENT]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, ElementSize, ElementData, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	GLint UniformBlockSize = 0; //��ʱ����������
	glGetActiveUniformBlockiv(program, uniform_transform_loc, 
		       GL_UNIFORM_BLOCK_DATA_SIZE, &UniformBlockSize); //��ѯuniform block ��С
	glBindBuffer(GL_UNIFORM_BUFFER, buffer[Buffer::TRANSFORM]);
    glBufferData(GL_UNIFORM_BUFFER, UniformBlockSize, 0, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glm::vec4 Diffuse(1.0f, 0.5f, 0.0f, 1.0f);
    glGetActiveUniformBlockiv(program, uniform_material_loc, 
		                     GL_UNIFORM_BLOCK_DATA_SIZE, &UniformBlockSize);
	glBindBuffer(GL_UNIFORM_BUFFER, buffer[Buffer::MATERIAL]);
	glBufferData(GL_UNIFORM_BUFFER, UniformBlockSize, &Diffuse[0], GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

}

void init_vertexArray()
{
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glBindBuffer(GL_VERTEX_ARRAY, buffer[Buffer::VERTEX]);
	vertex_loc = glGetAttribLocation(program, "position");
	glVertexAttribPointer(vertex_loc, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vertex_loc);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer[Buffer::ELEMENT]);
	glBindVertexArray(0);
}


void init()
{
	init_shader();
	init_buffer();
	init_vertexArray();
}

void render()
{
	glBindBuffer(GL_UNIFORM_BUFFER, buffer[Buffer::TRANSFORM]);
	glm::mat4* Pointer = (glm::mat4*)glMapBufferRange(
			    GL_UNIFORM_BUFFER, 0,	sizeof(glm::mat4),
			    GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

	glm::mat4 Projection = glm::perspective(glm::pi<float>() * 0.25f, 4.0f / 3.0f, 0.1f, 100.0f);
	glm::mat4 Model = glm::mat4(1.0f);
	glm::mat4 view_matrix = glm::lookAt(glm::vec3(0.0, 0.0, 5.0), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));;
	glm::mat4 MVP = Projection  * view_matrix * Model ;
	*Pointer = MVP;
				// Make sure the uniform buffer is uploaded
	glUnmapBuffer(GL_UNIFORM_BUFFER);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glViewport(0, 0, 720, 640);
	glClearBufferfv(GL_COLOR, 0, &glm::vec4(1.0f)[0]);

	glUseProgram(program);
	glUniformBlockBinding(program, uniform_transform_loc, 0); //��ubo��index�󶨵��󶨵���
    glUniformBlockBinding(program, uniform_material_loc, 1);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, buffer[Buffer::TRANSFORM]); //��buffer��󶨵����
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, buffer[Buffer::MATERIAL]);
	glBindVertexArray(vao);
	glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL, 1);
	glutSwapBuffers();
	glutPostRedisplay();
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowSize(720, 640);
	glutInitWindowPosition(300, 150);
	glutCreateWindow("gl-320-program");
	glewInit();
	init();
	glutDisplayFunc(render);
	glutMainLoop();
}
