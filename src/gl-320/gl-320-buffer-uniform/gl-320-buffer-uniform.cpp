#include <GL/glew.h>
#include <GL/freeglut.h>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <gli/gli.hpp>
#include <iostream>
#include <string>

using namespace std;
#include <shader.h>

#define BUFFER_OFFSET(offset) ((GLvoid*) (NULL + offset))

const int posX(300), posY(0);
const int Width(720), Height(640);
const char *Title = "gl-300-test-uniform";

GLuint program = 0;
GLuint mvp_loc = 0;
GLuint vbo = 0, vao = 0, ibo = 0, ubo = 0;
GLuint uTransform_loc = 0, uMaterial_loc;
GLuint uboTransform, uboMaterial;
Shader uniformShader("test-uniform");

// With DDS textures, v texture coordinate are reversed, from top to bottom
GLsizei const VertexCount(4);
GLsizeiptr const VertexSize = VertexCount * sizeof(glm::vec2);
//������������
glm::vec2 const VertexData[VertexCount] = { //������ô���õ�����
		glm::vec2(-1.0f,-1.0f),
		glm::vec2( 1.0f,-1.0f),
		glm::vec2( 1.0f, 1.0f),
		glm::vec2(-1.0f, 1.0f)
};

GLsizei const IndiceCount(6);
GLsizeiptr const IndiceSize = IndiceCount * sizeof(GLushort);
GLushort const indices[IndiceCount] = {
	0, 1, 2,
	2, 3, 0,
};

void load_shaders() //����shader�ļ�
{
	uniformShader.init();
	uniformShader.attach(GL_VERTEX_SHADER, "buffer-uniform.vert");
	uniformShader.attach(GL_FRAGMENT_SHADER, "buffer-uniform.frag");
	uniformShader.link();
	uniformShader.use();
	program = uniformShader.program;  
	mvp_loc = glGetUniformLocation(program, "mvp_matrix");
	uTransform_loc = glGetUniformBlockIndex(program, "transform");//��ȡuniform��shader�е�λ������
	uMaterial_loc  = glGetUniformBlockIndex(program, "material");  //�������ָʾshader��ub��λ��

	glUniformBlockBinding(program, uTransform_loc, 0); //��λ�������󶨵��󶨵� ��layout�ڵ�location,�뻺����󽻻��ӿ�
	glUniformBlockBinding(program, uMaterial_loc, 1);

}

void init_buffer()
{
	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, IndiceSize, indices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glGenBuffers(1, &vbo);  //�������ݻ������
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, VertexSize, VertexData, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0); //�������ݽ����
	//check Error
	GLint UniformBlockSize = 0; //��ѯuniform����shader��ռ�Ŀռ��С
	glGetActiveUniformBlockiv(program, uTransform_loc, GL_UNIFORM_BLOCK_DATA_SIZE, &UniformBlockSize);
	glGenBuffers(1, &uboTransform);  //�����洢tranform��Ļ������
	glBindBuffer(GL_UNIFORM_BUFFER, uboTransform);
	glBufferData(GL_UNIFORM_BUFFER, UniformBlockSize, 0, GL_DYNAMIC_DRAW); //����shaderռ�ݵĿռ���仺�����ռ�
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glm::vec4 Diffuse(1.0f, 0.5f, 0.0f, 1.0f);
	glGetActiveUniformBlockiv(program, uMaterial_loc, GL_UNIFORM_BLOCK_DATA_SIZE, &UniformBlockSize);
	glGenBuffers(1, &uboMaterial);   //�����洢material��Ļ������
	glBindBuffer(GL_UNIFORM_BUFFER, uboMaterial);
	glBufferData(GL_UNIFORM_BUFFER, UniformBlockSize, &Diffuse[0], GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}


void init_vertexArray()
{
	glGenVertexArrays(1, &vao);  //������������������״̬
	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);  //��vbo�е����ݴ��͵�shader��ȥ
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBindVertexArray(0);
}


void init()
{ 
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDrawBuffer(GL_BACK);
	load_shaders();  //��ʼ��
	init_buffer();
	init_vertexArray();
}

void render(void)
{

	glBindBuffer(GL_UNIFORM_BUFFER, uboTransform); //��tranform��ubo����ʼ������
	glm::mat4 *matrix = (glm::mat4*) glMapBufferRange(GL_UNIFORM_BUFFER,
		0, sizeof(glm::mat4), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
	    //mvp�任����
	    glm::mat4 proj_matrix = glm::perspective(45.0f, static_cast<float>(Width) / Height, 0.1f, 100.0f);
		*matrix = proj_matrix;
	glUnmapBuffer(GL_UNIFORM_BUFFER);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glm::mat4 mvp_matrix = glm::perspective(45.0f, 1.0f, 1.0f, 1000.0f);
	glUniformMatrix4fv(glGetUniformLocation(program, "mvp_matrix"),1, GL_FALSE, &mvp_matrix[0][0]);
	glViewport(0, 0, static_cast<GLsizei>(Width), static_cast<GLsizei>(Height));
	glClearBufferfv(GL_COLOR, 0, &glm::vec4(1.0f)[0]);

	glUseProgram(program);  //��uniformֵ����shader
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, uboTransform);  //���������󶨵��󶨵���
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, uboMaterial);

	glBindVertexArray(vao);  //����ǰ��vaoѡ��������ݣ�����ʵ����������
	glDrawElementsInstancedBaseVertex(GL_TRIANGLES, IndiceCount, GL_UNSIGNED_SHORT, nullptr, 1, 0);
	glutSwapBuffers();
}

void end()
{
	glDeleteBuffers(1, &vbo); //���״̬
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