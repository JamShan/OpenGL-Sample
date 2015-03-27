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
const char *Title = "gl-320-glsl-builtin-blocks";
const char *vs_shader_code("glsl-discard.vert");
const char *fs_shader_code("glsl-discard.frag");
GLuint program = 0, vao = 0, texture;
GLuint mvp_loc = 0;
GLuint uMaterial_loc;
GLuint uboTransform, uboMaterial;
Shader discardShader("glsl-builtin-blocks");

namespace Buffer
{
	enum type 
	{
		VERTEX,  //����
		ELEMENT,   //����
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
		glm::vec2( 1.0f, 1.0f), glm::vec2(1.0f, 0.0f),
		glm::vec2(-1.0f, 1.0f), glm::vec2(0.0f, 0.0f),
		glm::vec2(-1.0f,-1.0f), glm::vec2(0.0f, 1.0f)
};

GLsizei const ElementCount(6); //������Ϣ
GLsizeiptr const ElementSize = ElementCount * sizeof(glm::uint32);
glm::uint32 const ElementData[ElementCount] = {
		0, 1, 2,
		0, 2, 3
};


void load_shaders() //����shader�ļ�
{
	discardShader.init();
	discardShader.attach(GL_VERTEX_SHADER, vs_shader_code);
	discardShader.attach(GL_FRAGMENT_SHADER, fs_shader_code);
	discardShader.link();
	discardShader.use();
	program = discardShader.program; 
	mvp_loc = glGetUniformLocation(program, "mvp_matrix");
	uMaterial_loc = glGetUniformLocation(program, "diffuse");
	glUniform1i(uMaterial_loc, 0);
}

void init_buffer()
{
	glGenBuffers(Buffer::MAX, &buffer[0]);	 //�������ݻ������

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer[Buffer::ELEMENT]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, ElementSize, ElementData, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, buffer[Buffer::VERTEX]);
	glBufferData(GL_ARRAY_BUFFER, VertexSize, VertexData, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0); //�������ݽ����

}

void init_texture()
{
		gli::texture2D Texture(gli::load_dds("../media/kueken2-bgra8.dds"));
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1); //���ش洢��ʽ

		glGenTextures(1, &texture);  //�������
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, GLint(Texture.levels() - 1));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
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
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2) * 2, BUFFER_OFFSET(0)); 
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2) * 2 ,  BUFFER_OFFSET(sizeof(glm::vec2)));
	                       //index, size, type, normalized, stride, pointer
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer[Buffer::ELEMENT]);
	glBindVertexArray(0);
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
    glm::mat4 proj_matrix = glm::perspective(glm::pi<float>() * 0.25f, static_cast<float>(Width) / Height, 0.1f, 100.0f);
    glm::mat4 model_matrix = glm::mat4(1.0f);
	glm::mat4 view_matrix = glm::lookAt(glm::vec3(0.0, 0.0, 5.0), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
    glm::mat4 mvp_matrix= proj_matrix * view_matrix * model_matrix;
    glUniformMatrix4fv(mvp_loc, 1, GL_FALSE, &mvp_matrix[0][0]);
	
	glViewport(0, 0, static_cast<GLsizei>(Width), static_cast<GLsizei>(Height));
	glClearBufferfv(GL_COLOR, 0, &glm::vec4(1.0f, 0.5f, 0.0f, 1.0f)[0]);

	glUseProgram(program);  //��uniformֵ����shader
	glBindVertexArray(vao);  //����ǰ��vaoѡ��������ݣ�����ʵ����������
    glActiveTexture(GL_TEXTURE0); //����GL_TEXTURE_2D������0��Ԫ
	glBindTexture(GL_TEXTURE_2D, texture);
	glDrawArraysInstanced(GL_TRIANGLES, 0, VertexCount, 1);
	glutSwapBuffers();
	glutPostRedisplay();
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