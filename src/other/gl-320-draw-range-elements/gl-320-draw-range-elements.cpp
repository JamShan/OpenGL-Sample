#include <GL/glew.h>
#include <GL/freeglut.h>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <gli/gli.hpp>
#include <iostream>
#include <string>
#include <array>

using namespace std;
#include <shader.h>

#define BUFFER_OFFSET(offset) ((GLvoid*) (NULL + offset))

const int posX(300), posY(0);
const int Width(720), Height(640);
const char *Title = "gl-300-test-uniform";
const char *vs_shader_code("draw-multiple.vert");
const char *fs_shader_code("draw-multiple.frag");
GLuint program = 0, vao = 0;
GLuint mvp_loc = 0;
GLuint uTransform_loc = 0, uMaterial_loc;
GLuint uboTransform, uboMaterial;
Shader uniformShader("test-uniform");

namespace Buffer
{
	enum type 
	{
		POSITION,
		ELEMENT,
		TRANSFORM,
		MAX,
	};
}
array<GLuint, Buffer::MAX> buffer;

// With DDS textures, v texture coordinate are reversed, from top to bottom
GLsizei const VertexCount(8);
GLsizeiptr const VertexSize = VertexCount * sizeof(glm::vec2);
//������������
glm::vec2 const VertexData[VertexCount] = { //������ô���õ�����
		glm::vec2(-0.4f,-0.6f),
		glm::vec2( 0.5f,-0.4f),
		glm::vec2( 0.6f, 0.4f),
		glm::vec2(-0.5f, 0.5f),
		glm::vec2(-0.5f,-0.5f),
		glm::vec2( 0.5f,-0.5f),
		glm::vec2( 0.5f, 0.5f),
		glm::vec2(-0.5f, 0.5f)
};

GLsizei const ElementCount(12);
GLsizeiptr const ElementSize = ElementCount * sizeof(GLushort);
GLushort const ElementData[ElementCount] =
{
	0, 1, 2,
	2, 3, 0,
	4, 5, 6,
	6, 7, 4
};


void load_shaders() //����shader�ļ�
{
	uniformShader.init();
	uniformShader.attach(GL_VERTEX_SHADER, vs_shader_code);
	uniformShader.attach(GL_FRAGMENT_SHADER, fs_shader_code);
	uniformShader.link();
	uniformShader.use();
	program = uniformShader.program;  
	mvp_loc = glGetUniformLocation(program, "mvp_matrix");
	uTransform_loc = glGetUniformBlockIndex(program, "transform");//��ȡuniform��shader�е�λ������

	glUniformBlockBinding(program, uTransform_loc, 3); //��λ�������󶨵��󶨵� ��layout�ڵ�location,�뻺����󽻻��ӿ�

}

void init_buffer()
{
	glGenBuffers(Buffer::MAX, &buffer[0]);	 //�������ݻ������

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer[Buffer::ELEMENT]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, ElementSize, ElementData, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, buffer[Buffer::POSITION]);
	glBufferData(GL_ARRAY_BUFFER, VertexSize, VertexData, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0); //�������ݽ����

	GLint UniformBufferOffset(0);
	glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &UniformBufferOffset);
	GLint UniformBlockSize = glm::max(GLint(sizeof(glm::mat4)), UniformBufferOffset);

	glBindBuffer(GL_UNIFORM_BUFFER, buffer[Buffer::TRANSFORM]);
	glBufferData(GL_UNIFORM_BUFFER, UniformBlockSize, NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

}


void init_vertexArray()
{
	glGenVertexArrays(1, &vao);  //������������������״̬
	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, buffer[Buffer::POSITION]);  //��vbo�е����ݴ��͵�shader��ȥ
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer[Buffer::ELEMENT]);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void init()
{ 
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	load_shaders();  //��ʼ��
	init_buffer();
	init_vertexArray();
}

void render(void)
{
	glBindBuffer(GL_UNIFORM_BUFFER, buffer[Buffer::TRANSFORM]); //��tranform��ubo����ʼ������
	glm::mat4 *matrix = (glm::mat4*) glMapBufferRange(GL_UNIFORM_BUFFER,
		0, sizeof(glm::mat4), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
	    //mvp�任����
	    glm::mat4 proj_matrix = glm::perspective(glm::pi<float>() * 0.25f, static_cast<float>(Width) / Height, 0.1f, 100.0f);
		glm::mat4 model_matrix = glm::mat4(1.0f);
		glm::mat4 mvp_matrix= proj_matrix * model_matrix;
		*matrix = mvp_matrix;
	glUnmapBuffer(GL_UNIFORM_BUFFER);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glViewport(0, 0, static_cast<GLsizei>(Width), static_cast<GLsizei>(Height));
	const static float Depth(1.0f);
	glClearBufferfv(GL_DEPTH, 0, &Depth);
	glClearBufferfv(GL_COLOR, 0, &glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)[0]);

	glUseProgram(program);  //��uniformֵ����shader
	glBindBufferBase(GL_UNIFORM_BUFFER, 3, buffer[Buffer::TRANSFORM]);  //���������󶨵��󶨵���
	glBindVertexArray(vao);  //����ǰ��vaoѡ��������ݣ�����ʵ����������


	glViewport(static_cast<GLint>(Width * 0 / 3), 0, static_cast<GLsizei>(Width / 3), static_cast<GLsizei>(Height));
	glDrawElements(GL_TRIANGLES, ElementCount / 2, GL_UNSIGNED_SHORT, BUFFER_OFFSET(0));

	glViewport(static_cast<GLint>(Width * 1 / 3), 0, static_cast<GLsizei>(Width / 3), static_cast<GLsizei>(Height));
	glDrawElementsInstancedBaseVertex(GL_TRIANGLES, ElementCount / 2, GL_UNSIGNED_SHORT, BUFFER_OFFSET(sizeof(GLushort) * ElementCount / 2), 1, 0);

	glViewport(static_cast<GLint>(Width * 2 / 3), 0, static_cast<GLsizei>(Width / 3), static_cast<GLsizei>(Height));
	glDrawElementsInstancedBaseVertex(GL_TRIANGLES, ElementCount / 2, GL_UNSIGNED_SHORT, BUFFER_OFFSET(0), 1, VertexCount / 2);

}

void end()
{
	glDeleteBuffers(Buffer::MAX, &buffer[0]);
	glDeleteProgram(program);
	glDeleteVertexArrays(1, &vao);
}

int main(int argc, char**argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowPosition(posX, posY);
	glutInitWindowSize(Width, Height);
	glutCreateWindow(Title);
	GLenum status = glewInit();  //��ʼ��glew����չ����
	if (status != GLEW_OK) {
		cerr << "Error" << glewGetErrorString(status);
		return EXIT_FAILURE;
	}
	init();
	glutDisplayFunc(render);
	glutMainLoop();
	end();
	return 0;
}