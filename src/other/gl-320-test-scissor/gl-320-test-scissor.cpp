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

#define BUFFER_OFFSET(offset) ((GLvoid*) (NULL + offset)) //λ��ָ��

const int posX(300), posY(0);
const int Width(720), Height(640);
const char *Title = "gl-320-test-scissor";
const char *vs_shader_code("test-scissor.vert");
const char *fs_shader_code("test-scissor.frag");
GLuint program = 0, vao = 0, texture;
GLuint mvp_loc = 0;
GLuint vertex_loc, tc_loc;
GLuint uTransform_loc = 0, uMaterial_loc;
GLuint uboTransform, uboMaterial;
Shader textureShader("test-scissor");

namespace Buffer
{
	enum type 
	{
		VERTEX,  //����
		TRANSFORM, //uniform ��
		MAX,   //����
	};
}
array<GLuint, Buffer::MAX> buffer;  //���������������

// With DDS textures, v texture coordinate are reversed, from top to bottom
GLsizei const VertexCount(12);  
GLsizeiptr const VertexSize = VertexCount * sizeof(glm::vec2);
//������������
glm::vec2 const VertexData[VertexCount] = { //������ô���õ�����
		glm::vec2(-1.0f,-1.0f), glm::vec2(0.0f, 1.0f),
		glm::vec2( 1.0f,-1.0f), glm::vec2(1.0f, 1.0f),
		glm::vec2( 1.0f, 1.0f), glm::vec2(1.0f, 0.0f),
		glm::vec2(-1.0f, 1.0f), glm::vec2(0.0f, 0.0f)
};

GLsizei const ElementCount(6); //������Ϣ
GLsizeiptr const ElementSize = ElementCount * sizeof(glm::uint32);
glm::uint32 const ElementData[ElementCount] = {
		0, 1, 2,
		0, 2, 3
};


void load_shaders() //����shader�ļ�
{
	textureShader.init();
	textureShader.attach(GL_VERTEX_SHADER, vs_shader_code);
	textureShader.attach(GL_FRAGMENT_SHADER, fs_shader_code);
	textureShader.link();
	textureShader.use();
	program = textureShader.program; 
	vertex_loc = glGetAttribLocation(program, "position");
	tc_loc = glGetAttribLocation(program, "texCoord");

	uTransform_loc = glGetUniformBlockIndex(program, "transform");//��ȡuniform��shader�е�λ������
	glUniformBlockBinding(program, uTransform_loc, 3); //��λ�������󶨵��󶨵� ��layout�ڵ�location,�뻺����󽻻��ӿ�

}

void init_buffer()
{
	glGenBuffers(Buffer::MAX, &buffer[0]);	 //�������ݻ������

	glBindBuffer(GL_ARRAY_BUFFER, buffer[Buffer::VERTEX]);
	glBufferData(GL_ARRAY_BUFFER, VertexSize, VertexData, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0); //�������ݽ����

	GLint UniformBufferOffset(0);
	glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &UniformBufferOffset);
	GLint UniformBlockSize = glm::max(GLint(sizeof(glm::mat4)), UniformBufferOffset);
	glBindBuffer(GL_UNIFORM_BUFFER, buffer[Buffer::TRANSFORM]);
	glBufferData(GL_UNIFORM_BUFFER, UniformBlockSize, 0, GL_DYNAMIC_DRAW); //����shaderռ�ݵĿռ���仺�����ռ�
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

}

void init_texture()
{
		gli::texture2D Texture;//(gli::load_dds((getDataDirectory() + TEXTURE_DIFFUSE).c_str()));
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1); //���ش洢��ʽ

		glGenTextures(1, &texture);  //�������
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, GLint(Texture.levels() - 1));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, Texture.levels() == 1 ? GL_NEAREST : GL_NEAREST_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		for(gli::texture2D::size_type Level = 0; Level < Texture.levels(); ++Level) {
			glTexImage2D(GL_TEXTURE_2D,
				GLint(Level),
				gli::internal_format(Texture.format()),
				GLsizei(Texture[Level].dimensions().x),
				GLsizei(Texture[Level].dimensions().y),
				0,
				gli::external_format(Texture.format()),
				gli::type_format(Texture.format()),
				Texture[Level].data());
		}

		if(Texture.levels() == 1)
			glGenerateMipmap(GL_TEXTURE_2D);
	
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

}

void init_vertexArray()
{
	glGenVertexArrays(1, &vao);  //������������������״̬
	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, buffer[Buffer::VERTEX]);  //��vbo�е����ݴ��͵�shader��ȥ
	glVertexAttribPointer(vertex_loc, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2) * 2, BUFFER_OFFSET(0)); 
	glVertexAttribPointer(tc_loc, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2) * 2 ,  BUFFER_OFFSET(sizeof(glm::vec2)));
	                       //index, size, type, normalized, stride, pointer
	glEnableVertexAttribArray(vertex_loc);
	glEnableVertexAttribArray(tc_loc);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void init()
{ 
	load_shaders();  //��ʼ��
	init_buffer();
	init_texture();
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

	glm::vec3 MinScissor(10000.0f);
	glm::vec3 MaxScissor(-10000.0f);
	glm::mat4 Ortho = glm::ortho(0.0f, 0.0f, float(720), float(640));
	for(GLsizei i = 0; i < VertexCount; ++i)
			{
				glm::vec3 Projected = glm::project(
					glm::vec3(VertexData[i].Position, 0.0f), 
					View * Model, 
					Projection, 
					glm::ivec4(0, 0, WindowSize.x, WindowSize.y));

				MinScissor = glm::min(MinScissor, glm::vec3(Projected));
				MaxScissor = glm::max(MaxScissor, glm::vec3(Projected));
			}
	}
	glViewport(0, 0, static_cast<GLsizei>(Width), static_cast<GLsizei>(Height));
	glClearBufferfv(GL_COLOR, 0, &glm::vec4(1.0f, 0.5f, 0.0f, 1.0f)[0]);
	glScissor(GLint(MinScissor.x), GLint(MinScissor.y), GLsizei(MaxScissor.x - MinScissor.x), GLsizei(MaxScissor.y - MinScissor.y));
    glEnable(GL_SCISSOR_TEST);
	glClearBufferfv(GL_COLOR, 0, &glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)[0]);

	glUseProgram(program);  //��uniformֵ����shader
	glBindBufferBase(GL_UNIFORM_BUFFER, 3, buffer[Buffer::TRANSFORM]);  //���������󶨵��󶨵���
	glBindVertexArray(vao);  //����ǰ��vaoѡ��������ݣ�����ʵ����������
    glActiveTexture(GL_TEXTURE0); //����GL_TEXTURE_2D������0��Ԫ
	glBindTexture(GL_TEXTURE_2D, texture);
	glDrawArraysInstanced(GL_TRIANGLES, 0 , VertexCount, 1);
	glDisable(GL_SCISSOR_TEST);

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