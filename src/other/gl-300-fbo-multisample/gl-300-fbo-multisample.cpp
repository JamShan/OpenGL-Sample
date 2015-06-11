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
const int fboWidth(720), fboHeight(640);

const char *Title = "gl-300-fbo-multisample";

GLuint program = 0;
GLuint mvp_loc = 0;
GLuint vbo = 0, tex = 0, vao = 0;
GLuint sampler_loc = 0;
GLuint colorBuffer, fboRender, fboTarget, colorTexture;
const string textureName("kueken2-bgr8.dds"); 
Shader fboShader("fbo-multisample");

struct Vertex {
	Vertex(glm::vec2 const & pos, glm::vec2 const &tex)
		:Position(pos), TexCoord(tex) {}  //��ʼ���б�
	glm::vec2 Position;  
	glm::vec2 TexCoord;
};

// With DDS textures, v texture coordinate are reversed, from top to bottom
GLsizei const VertexCount(6);
GLsizeiptr const VertexSize = VertexCount * sizeof(Vertex);
//������������
Vertex const VertexData[VertexCount] = { //������ô���õ�����
		Vertex( glm::vec2(-2.0f,-1.5f), glm::vec2(0.0f, 0.0f) ),
		Vertex( glm::vec2( 2.0f,-1.5f), glm::vec2(1.0f, 0.0f) ),
		Vertex( glm::vec2( 2.0f, 1.5f), glm::vec2(1.0f, 1.0f) ),
		Vertex( glm::vec2( 2.0f, 1.5f), glm::vec2(1.0f, 1.0f) ),
		Vertex( glm::vec2(-2.0f, 1.5f), glm::vec2(0.0f, 1.0f) ),
		Vertex( glm::vec2(-2.0f,-1.5f), glm::vec2(0.0f, 0.0f) )
};


void load_shaders() //����shader�ļ�
{
	fboShader.init();
	fboShader.attach(GL_VERTEX_SHADER, "image-2d.vert");
	fboShader.attach(GL_FRAGMENT_SHADER, "image-2d.frag");
	fboShader.link();
	fboShader.use();
	program = fboShader.program;  //��ȡuniform��λ������
	mvp_loc = glGetUniformLocation(program, "mvp_matrix");
	sampler_loc = glGetUniformLocation(program, "sampler");

}

void init_buffer()
{
	glGenBuffers(1, &vbo);  //�������ݻ������
	glBindBuffer(GL_ARRAY_BUFFER, vbo);  //���������ݴ���
	glBufferData(GL_ARRAY_BUFFER, VertexSize, VertexData, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0); //�������ݽ����
	//check Error
}

void init_textures()
{
	glGenTextures(1, &tex);  //�����������
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);   

	gli::texture2D Texture( gli::load_dds("../media/kueken3-bgr8.dds"));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0); //ѡ�������˲���ʽ
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, GLint(Texture.levels() - 1));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //mipmap
	for (std::size_t Level = 0; Level < Texture.levels(); ++Level) {
			glTexImage2D(GL_TEXTURE_2D,
				GLint(Level),
				GL_RGB8,
				GLsizei(Texture[Level].dimensions().x),
				GLsizei(Texture[Level].dimensions().y),
				0,
				GL_BGR, GL_UNSIGNED_BYTE,
				Texture[Level].data());
		}

}

void init_vertexArray()
{
	glGenVertexArrays(1, &vao);  //������������������״̬
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);  //��vbo�е����ݴ��͵�shader��ȥ
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), BUFFER_OFFSET(0));
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), BUFFER_OFFSET(sizeof(glm::vec2)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);
}


void init_framebuffer()
{
	glGenRenderbuffers(1, &colorBuffer); //������Ⱦ�������
	glBindRenderbuffer(GL_RENDERBUFFER, colorBuffer);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, 8, GL_RGBA8, fboWidth, fboHeight);
	//�����洢ͼ����Ϣ�Ŀռ�

	glGenFramebuffers(1, &fboRender);  //����֡�������
	glBindFramebuffer(GL_FRAMEBUFFER, fboRender);  //�����洴������Ⱦ�����������ǰ֡������󣬼�����Ⱦ��������Ⱦ��֡����
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glGenTextures(1, &colorTexture); //��������洢����֡�������ɵ�ͼ��
	glBindTexture(GL_TEXTURE_2D, colorTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, fboWidth, fboHeight , 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

	glGenFramebuffers(1, &fboTarget);  //������ʾ�ڴ��ڵ�fbo������Ϊǰһ��fbo����Ⱦͼ��
	glBindFramebuffer(GL_FRAMEBUFFER, fboTarget);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void init()
{ 

	load_shaders();  //��ʼ��
	init_buffer();
	init_textures();
	init_vertexArray();
	init_framebuffer();
}

void renderFBO(GLuint Framebuffer)
{
		glBindFramebuffer(GL_FRAMEBUFFER, Framebuffer);
		glClearColor(0.0f, 0.5f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

	    //mvp�任����
	    glm::mat4 proj_matrix = glm::perspective(glm::pi<float>() * 0.25f, static_cast<float>(Width) / Height, 0.1f, 100.0f);
		glm::mat4 model_matrix = glm::mat4(1.0f);
		glm::mat4 mvp_matrix= proj_matrix * model_matrix;

		glViewport(0, 0, static_cast<GLsizei>(fboWidth), static_cast<GLsizei>(fboHeight));
		glClearColor(0.0f, 0.5f, 1.0f, 1.0f); //���ñ���ɫ
		glClear(GL_COLOR_BUFFER_BIT);

		glActiveTexture(GL_TEXTURE0);  //Ӧ�ü���ͼ������
		glBindTexture(GL_TEXTURE_2D, tex);

		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, VertexCount);

}

void renderFB(GLuint Texture2DName)
{
	    //mvp�任����
	    glm::mat4 proj_matrix = glm::perspective(glm::pi<float>() * 0.25f, static_cast<float>(Width) / Height, 0.1f, 100.0f);
		glm::mat4 model_matrix = glm::mat4(1.0f);
		glm::mat4 mvp_matrix= proj_matrix * model_matrix;

		glActiveTexture(GL_TEXTURE0); //Ӧ��fbo���ɵ�ͼ����Ϊ����
		glBindTexture(GL_TEXTURE_2D, Texture2DName);

		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, VertexCount);
	
}

void render()
{
		// Clear the framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearBufferfv(GL_COLOR, 0, &glm::vec4(1.0f, 0.5f, 0.0f, 1.0f)[0]);

		glUseProgram(program);
		glUniform1i(sampler_loc, 0);

		// Pass 1
		// Render the scene in a multisampled framebuffer
		glEnable(GL_MULTISAMPLE);  //���Ȼ�������fbo����
		renderFBO(fboRender);
		glDisable(GL_MULTISAMPLE);

		// Resolved multisampling
		glBindFramebuffer(GL_READ_FRAMEBUFFER, fboRender); //�󶨻���fbo
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboTarget);
		glBlitFramebuffer(
			0, 0, fboWidth, fboHeight, 
			0, 0, fboWidth, fboHeight, 
			GL_COLOR_BUFFER_BIT, GL_LINEAR);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Pass 2
		// Render the colorbuffer from the multisampled framebuffer
		glViewport(0, 0, Width, Height);
		renderFB(colorTexture);  //����������ʾͼ��
		glutSwapBuffers();
};

void end()
{
	glDeleteBuffers(1, &vbo); //���״̬
	glDeleteProgram(program);
	glDeleteTextures(1, &tex);
	glDeleteVertexArrays(1, &vao);
	glDeleteFramebuffers(1, &fboRender);
	glDeleteFramebuffers(1, &fboTarget);
	glDeleteRenderbuffers(1, &colorBuffer);
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