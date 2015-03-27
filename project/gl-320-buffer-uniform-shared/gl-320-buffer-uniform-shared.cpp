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
GLsizei const VertexCount(6);
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
	uniformShader.attach(GL_VERTEX_SHADER, "buffer-uniform-shared.vert");
	uniformShader.attach(GL_FRAGMENT_SHADER, "buffer-uniform-shared.frag");
	uniformShader.link();
	uniformShader.use();
	program = uniformShader.program;  
	mvp_loc = glGetUniformLocation(program, "mvp_matrix");
	uTransform_loc = glGetUniformBlockIndex(program, "transform");//��ȡuniform��shader�е�λ������
	uMaterial_loc  = glGetUniformBlockIndex(program, "material");  //�������ָʾshader��ub��λ��

	glUniformBlockBinding(program, uTransform_loc, 3); //��λ�������󶨵��󶨵� ��layout�ڵ�location,�뻺����󽻻��ӿ�
	glUniformBlockBinding(program, uMaterial_loc, 4);

}

GLint UniformBlockSizeTransform;
GLint UniformBlockSizeMaterial;

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
	GLint UniformBufferOffset(0);
	glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &UniformBufferOffset);

	glGetActiveUniformBlockiv(program, uTransform_loc, GL_UNIFORM_BLOCK_DATA_SIZE, &UniformBlockSizeTransform);
	UniformBlockSizeTransform = ((UniformBlockSizeTransform / UniformBufferOffset) + 1) * UniformBufferOffset;
	glGetActiveUniformBlockiv(program, uMaterial_loc, GL_UNIFORM_BLOCK_DATA_SIZE, &UniformBlockSizeMaterial);
	UniformBlockSizeMaterial = ((UniformBlockSizeMaterial / UniformBufferOffset) + 1) * UniformBufferOffset;

	glGenBuffers(1, &ubo);  //�����洢tranform��Ļ������
	glBindBuffer(GL_UNIFORM_BUFFER, ubo);
	glBufferData(GL_UNIFORM_BUFFER,  UniformBlockSizeTransform + 
		         UniformBlockSizeMaterial, NULL, GL_DYNAMIC_DRAW); //����shaderռ�ݵĿռ���仺�����ռ�
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
	glBindBuffer(GL_ARRAY_BUFFER, 0);
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
	glm::mat4 proj_matrix = glm::perspective(glm::pi<float>() * 0.25f, static_cast<float>(Width) / Height, 0.1f, 100.0f);
	glm::mat4 model_matrix = glm::mat4(1.0f);
	glm::mat4 mvp_matrix= proj_matrix * model_matrix;
	glm::vec4 Diffuse(1.0f, 0.5f, 0.0f, 1.0f);

	glBindBuffer(GL_UNIFORM_BUFFER, ubo); //��tranform��ubo����ʼ������
	glm::byte* Pointer = (glm::byte*)glMapBufferRange(
				GL_UNIFORM_BUFFER, 0,	UniformBlockSizeTransform + sizeof(glm::vec4),
				GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

	*(glm::mat4*)(Pointer + 0) = mvp_matrix;
	*(glm::vec4*)(Pointer + UniformBlockSizeTransform) = Diffuse;
	glUnmapBuffer(GL_UNIFORM_BUFFER);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glViewport(0, 0, static_cast<GLsizei>(Width), static_cast<GLsizei>(Height));
	glClearBufferfv(GL_COLOR, 0, &glm::vec4(1.0f)[0]);

	glUseProgram(program);  //��uniformֵ����shader
	glUniformBlockBinding(program, uboTransform, 3);
	glUniformBlockBinding(program, uboMaterial, 4);
	glBindBufferRange(GL_UNIFORM_BUFFER, 3, ubo, 0, UniformBlockSizeTransform); //����ʽ
	// Attach the buffer to UBO binding point semantic::uniform::MATERIAL
	glBindBufferRange(GL_UNIFORM_BUFFER, 4, ubo, UniformBlockSizeTransform, UniformBlockSizeMaterial);
	// Bind vertex array & draw
	glBindVertexArray(vao);
	glDrawElementsInstancedBaseVertex(GL_TRIANGLES, IndiceCount, GL_UNSIGNED_SHORT, NULL, 1, 0);
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