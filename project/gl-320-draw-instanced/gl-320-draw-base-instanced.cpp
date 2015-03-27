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

GLuint program = 0, vao = 0;
GLuint mvp_loc = 0;
GLuint uTransform_loc = 0;
GLuint uboTransform;
Shader instanceShader("test-uniform");

namespace Buffer
{
	enum type 
	{
		POSITION,
		TRANSFORM,
		MAX,
	};
}
array<GLuint, Buffer::MAX> buffer;

// With DDS textures, v texture coordinate are reversed, from top to bottom
GLsizei const VertexCount(6);
GLsizeiptr const VertexSize = VertexCount * sizeof(glm::vec2);
glm::vec2 const VertexData[VertexCount] = { //纹理怎么运用到上面
		glm::vec2(-1.0f,-1.0f),
		glm::vec2( 1.0f,-1.0f),
		glm::vec2( 1.0f, 1.0f),
		glm::vec2( 1.0f, 1.0f),
		glm::vec2(-1.0f, 1.0f),
		glm::vec2(-1.0f,-1.0f)
};


void load_shaders() //加载shader文件
{
	instanceShader.init();
	instanceShader.attach(GL_VERTEX_SHADER, "draw-instanced.vert");
	instanceShader.attach(GL_FRAGMENT_SHADER, "draw-instanced.frag");
	instanceShader.link();
	instanceShader.use();
	program = instanceShader.program;  

	uTransform_loc = glGetUniformBlockIndex(program, "transform");//获取uniform在shader中的位置索引
	glUniformBlockBinding(program, uTransform_loc, 0); //将位置索引绑定到绑定点0， 即layout内的location,与缓存对象交互接口

}

void init_buffer()
{
	glGenBuffers(Buffer::MAX, &buffer[0]);	 //创建数据缓存对象

	glBindBuffer(GL_ARRAY_BUFFER, buffer[Buffer::POSITION]);
	glBufferData(GL_ARRAY_BUFFER, VertexSize, VertexData, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0); //传完数据解除绑定

	GLint UniformBufferOffset(0);
	glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &UniformBufferOffset);
	GLint UniformBlockSize = glm::max(GLint(sizeof(glm::mat4) * 2), UniformBufferOffset);
	glBindBuffer(GL_UNIFORM_BUFFER, buffer[Buffer::TRANSFORM]);
	glBufferData(GL_UNIFORM_BUFFER, UniformBlockSize, NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

}


void init_vertexArray()
{
	glGenVertexArrays(1, &vao);  //创建顶点数组对象管理状态
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, buffer[Buffer::POSITION]);  //将vbo中的数据传送到shader中去
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);
}


void init()
{ 
	load_shaders();  //初始化
	init_buffer();
	init_vertexArray();
}

void render(void)
{
	glBindBuffer(GL_UNIFORM_BUFFER, buffer[Buffer::TRANSFORM]); //绑定tranform的ubo，初始化数据
	glm::mat4 *Pointer = (glm::mat4*) glMapBufferRange(
		GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4) * 2,
		GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
	    //mvp变换矩阵
	    glm::mat4 proj_matrix = glm::perspective(30.0f, static_cast<float>(Width) / Height, 0.1f, 100.0f);
		glm::mat4 view	=  glm::lookAt(glm::vec3(0.0, 0.0, 3.0), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
		glm::mat4 ModelA = glm::translate(glm::mat4(1.0f), glm::vec3(-1.1f, 0.0f, 0.0f));
		glm::mat4 ModelB = glm::translate(glm::mat4(1.0f), glm::vec3(1.1f, 0.0f, 0.0f));
		
		*(Pointer + 0) = proj_matrix * view * ModelA;
		*(Pointer + 1) = proj_matrix * view * ModelB;
	glUnmapBuffer(GL_UNIFORM_BUFFER);

	glViewport(0, 0, static_cast<GLsizei>(Width), static_cast<GLsizei>(Height));
	const static float Depth(1.0f);
	glClearBufferfv(GL_DEPTH, 0, &Depth);
	glClearBufferfv(GL_COLOR, 0, &glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)[0]);

	glUseProgram(program);  //将uniform值传入shader
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, buffer[Buffer::TRANSFORM]);  //将缓存对象绑定到绑定点上
	glBindVertexArray(vao);  //绘制前绑定vao选择绘制数据，基于实例索引绘制

    glDrawArraysInstanced(GL_TRIANGLES, 0, VertexCount, 5);
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