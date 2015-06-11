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

GLuint program = 0, texture = 0, vao=0;
Shader uniformShader("test-uniform");


void load_shaders() //加载shader文件
{
	uniformShader.init();
	uniformShader.attach(GL_VERTEX_SHADER, "draw-image-space.vert");
	uniformShader.attach(GL_FRAGMENT_SHADER, "draw-image-space.frag");
	uniformShader.link();
	uniformShader.use();
	program = uniformShader.program;  

}

void init_textures()
{
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		gli::texture2D Texture(gli::load_dds("../media/kueken3-bgr8.dds"));
		glGenTextures(1, &texture);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, GLint(Texture.levels() - 1));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		for(gli::texture2D::size_type Level = 0; Level < Texture.levels(); ++Level)
		{
			glTexImage2D(GL_TEXTURE_2D,
				GLint(Level),
				GL_RGBA8,
				GLsizei(Texture[Level].dimensions().x),
				GLsizei(Texture[Level].dimensions().y),
				0,
				GL_BGR,
				GL_UNSIGNED_BYTE,
				Texture[Level].data());
		}
	
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

}

void init_vertexArray()
{
	glGenVertexArrays(1, &vao);  //创建顶点数组对象管理状态
	glBindVertexArray(vao);
	glBindVertexArray(0);
}


void init()
{ 
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	load_shaders();  //初始化
	init_vertexArray();
}

void render(void)
{

	glViewport(0, 0, static_cast<GLsizei>(Width), static_cast<GLsizei>(Height));

	glUseProgram(program);  //将uniform值传入shader
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	//glBindBufferBase(GL_UNIFORM_BUFFER, 3, buffer[Buffer::TRANSFORM]);  //将缓存对象绑定到绑定点上
	glBindVertexArray(vao);  //绘制前绑定vao选择绘制数据，基于实例索引绘制
	glDrawArraysInstanced(GL_TRIANGLES, 0, 3, 1);
}

void end()
{
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
	GLenum status = glewInit();  //初始化glew，扩展功能
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