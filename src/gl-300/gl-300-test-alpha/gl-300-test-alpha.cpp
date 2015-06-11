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
const char *Title = "gl-300-test-alpha";

GLuint program = 0;
GLuint mvp_loc = 0;
GLuint vbo = 0, tex = 0, vao = 0;
GLuint sampler_loc = 0, vertex_loc, tc_loc;
const string textureName("../media/kueken2-bgra8.dds"); 
Shader alphaShader("test-alpha");

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
		Vertex(glm::vec2(-1.0f,-1.0f), glm::vec2(0.0f, 1.0f)),
		Vertex(glm::vec2( 1.0f,-1.0f), glm::vec2(1.0f, 1.0f)),
		Vertex(glm::vec2( 1.0f, 1.0f), glm::vec2(1.0f, 0.0f)),
		Vertex(glm::vec2( 1.0f, 1.0f), glm::vec2(1.0f, 0.0f)),
		Vertex(glm::vec2(-1.0f, 1.0f), glm::vec2(0.0f, 0.0f)),
		Vertex(glm::vec2(-1.0f,-1.0f), glm::vec2(0.0f, 1.0f))
};


void load_shaders() //����shader�ļ�
{
	alphaShader.init();
	alphaShader.attach(GL_VERTEX_SHADER, "image-2d.vert");
	alphaShader.attach(GL_FRAGMENT_SHADER, "image-2d.frag");
	alphaShader.link();
	alphaShader.use();
	program = alphaShader.program;  //��ȡuniform��λ������
	mvp_loc = glGetUniformLocation(program, "mvp_matrix");
	sampler_loc = glGetUniformLocation(program, "sampler");

}

void init_buffer()
{
	glGenBuffers(1, &vbo);  //�������ݻ������
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, VertexSize, VertexData, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0); //�������ݽ����
	//check Error
}

void init_textures()
{
	glGenTextures(1, &tex);  //�������
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);   //ѡ�������˲���ʽ
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	
	gli::texture2D Texture( gli::load_dds(textureName.c_str()));
	for(std::size_t Level = 0; Level < Texture.levels(); ++Level) { //չ��ÿһ���
		glTexImage2D(
				GL_TEXTURE_2D, 
				GLint(Level), 
				GL_RGBA8, 
				GLsizei(Texture[Level].dimensions().x), 
				GLsizei(Texture[Level].dimensions().y), 
				0,  
				GL_BGRA, 
				GL_UNSIGNED_BYTE, 
				Texture[Level].data());
	}

}

void init_vertexArray()
{
	glGenVertexArrays(1, &vao);  //������������������״̬
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);  //��vbo�е����ݴ��͵�shader��ȥ
	vertex_loc = glGetAttribLocation(program, "Position");
	tc_loc = glGetAttribLocation(program, "TexCoord");
	glVertexAttribPointer(vertex_loc, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), BUFFER_OFFSET(0));
	glVertexAttribPointer(tc_loc, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), BUFFER_OFFSET(sizeof(glm::vec2)));
	glEnableVertexAttribArray(vertex_loc);
	glEnableVertexAttribArray(tc_loc);
	glBindVertexArray(0);
}


void init()
{ 
	 //glEnable(GL_ALPHA_TEST); //�ɵ�api
    // glAlphaFunc(GL_GREATER, 0.2f);
    //To test alpha blending:
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//Դ��ϲ�����A��A,A,A)�� Ŀ�Ļ�ϲ�����1-A, 1-A, 1-A, 1-A)��
	//�����Ƴ�������ɫ�ǣ�ARԴ, AG, ,AB ,AA��+����1-A��RĿ�����Ǳ���ɫ
	load_shaders();  //��ʼ��
	init_buffer();
	init_textures();
	init_vertexArray();
}

void render(void)
{
	    //mvp�任����
	    glm::mat4 proj_matrix = glm::perspective(glm::pi<float>() * 0.25f, static_cast<float>(Width) / Height, 0.1f, 100.0f);
		glm::mat4 model_matrix = glm::mat4(1.0f);
		glm::mat4 view_matrix = glm::lookAt(glm::vec3(0.0, 0.0, 3.0),glm::vec3(0.0, 0.0, 0.0),
			                                glm::vec3(0.0, 1.0, 0.0));
		glm::mat4 mvp_matrix= proj_matrix * model_matrix * view_matrix;

		glViewport(0, 0, static_cast<GLsizei>(Width), static_cast<GLsizei>(Height));
		glClearColor(1.0f, 0.5f, 0.0f, 1.0f); //���ñ���ɫ
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(program);  //��uniformֵ����shader
		glUniform1i(sampler_loc, 0);
		glUniformMatrix4fv(mvp_loc, 1, GL_FALSE, &mvp_matrix[0][0]);

		glActiveTexture(GL_TEXTURE0);  //ѡ��ǰ�洴��������
		glBindTexture(GL_TEXTURE_2D, tex);

		glBindVertexArray(vao);  //����ǰ��vaoѡ���������
		glDrawArrays(GL_TRIANGLES, 0, VertexCount);  //����������ٸ���
		glutSwapBuffers();
}

void end()
{
	glDeleteBuffers(1, &vbo); //���״̬
	glDeleteProgram(program);
	glDeleteTextures(1, &tex);
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