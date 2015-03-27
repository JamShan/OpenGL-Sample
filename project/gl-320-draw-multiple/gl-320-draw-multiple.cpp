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
//顶点数据数组
glm::vec3 const VertexData[VertexCount] = { //纹理怎么运用到上面
		glm::vec3(-1.0f,-1.0f, 0.5f),
		glm::vec3( 1.0f,-1.0f, 0.5f),
		glm::vec3( 1.0f, 1.0f, 0.5f),
		glm::vec3(-1.0f, 1.0f, 0.5f),
		glm::vec3(-0.5f,-1.0f,-0.5f),
		glm::vec3( 0.5f,-1.0f,-0.5f),
		glm::vec3( 1.5f, 1.0f,-0.5f),
		glm::vec3(-1.5f, 1.0f,-0.5f)
};

GLsizei const ElementCount(6);
GLsizeiptr const ElementSize = ElementCount * sizeof(glm::uint32);
glm::uint32 const ElementData[ElementCount] = {
		0, 1, 2,
		0, 2, 3
};

GLsizei const Count[2] = {ElementCount, ElementCount};
GLint const BaseVertex[2] = {0, 4};

void load_shaders() //加载shader文件
{
	uniformShader.init();
	uniformShader.attach(GL_VERTEX_SHADER, vs_shader_code);
	uniformShader.attach(GL_FRAGMENT_SHADER, fs_shader_code);
	uniformShader.link();
	uniformShader.use();
	program = uniformShader.program;  
	mvp_loc = glGetUniformLocation(program, "mvp_matrix");
	uTransform_loc = glGetUniformBlockIndex(program, "transform");//获取uniform在shader中的位置索引
	glUniformBlockBinding(program, uTransform_loc, 0); //将位置索引绑定到绑定点 即layout内的location,与缓存对象交互接口

}

void init_buffer()
{
	glGenBuffers(Buffer::MAX, &buffer[0]);	 //创建数据缓存对象

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer[Buffer::ELEMENT]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, ElementSize, ElementData, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, buffer[Buffer::POSITION]);
	glBufferData(GL_ARRAY_BUFFER, VertexSize, VertexData, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0); //传完数据解除绑定


	GLint UniformBlockSize = 0; //查询uniform块在shader中占的空间大小
	glGetActiveUniformBlockiv(program, uTransform_loc, GL_UNIFORM_BLOCK_DATA_SIZE, &UniformBlockSize);
	glBindBuffer(GL_UNIFORM_BUFFER, buffer[Buffer::TRANSFORM]);
	glBufferData(GL_UNIFORM_BUFFER, UniformBlockSize, 0, GL_DYNAMIC_DRAW); //根据shader占据的空间分配缓存对象空间
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

}


void init_vertexArray()
{
	glGenVertexArrays(1, &vao);  //创建顶点数组对象管理状态
	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, buffer[Buffer::POSITION]);  //将vbo中的数据传送到shader中去
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer[Buffer::ELEMENT]);
	glBindVertexArray(0);
}


void init()
{ 
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	load_shaders();  //初始化
	init_buffer();
	init_vertexArray();
}

void render(void)
{
	glBindBuffer(GL_UNIFORM_BUFFER, buffer[Buffer::TRANSFORM]); //绑定tranform的ubo，初始化数据
	glm::mat4 *matrix = (glm::mat4*) glMapBufferRange(GL_UNIFORM_BUFFER,
		0, sizeof(glm::mat4), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
	    //mvp变换矩阵
	    glm::mat4 proj_matrix = glm::perspective(glm::pi<float>() * 0.25f, static_cast<float>(Width) / Height, 0.1f, 100.0f);
		glm::mat4 model_matrix = glm::mat4(1.0f);
		glm::mat4 view_matrix = glm::lookAt(glm::vec3(0.0, 0.0, 5.0), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
		glm::mat4 mvp_matrix= proj_matrix * view_matrix * model_matrix;
		*matrix = mvp_matrix;
	glUnmapBuffer(GL_UNIFORM_BUFFER);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glViewport(0, 0, static_cast<GLsizei>(Width), static_cast<GLsizei>(Height));
	const static float Depth(1.0f);
	glClearBufferfv(GL_DEPTH, 0, &Depth);
	glClearBufferfv(GL_COLOR, 0, &glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)[0]);

	glUseProgram(program);  //将uniform值传入shader
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, buffer[Buffer::TRANSFORM]);  //将缓存对象绑定到绑定点上
	glBindVertexArray(vao);  //绘制前绑定vao选择绘制数据，基于实例索引绘制

		// Bug fix for cross platform build...
#		if defined(WIN32)// || defined(__APPLE__)
#			define CONV(x)		x
#		else
#			define CONV(x)		(GLvoid **)x
#		endif

		GLvoid const * Indexes[2] = {0, 0};
		glMultiDrawElementsBaseVertex(
			GL_TRIANGLES, //mode
			Count,    //count
			GL_UNSIGNED_INT,  //type
			Indexes,//CONV(Indexes),
			2,   //number
			BaseVertex);
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