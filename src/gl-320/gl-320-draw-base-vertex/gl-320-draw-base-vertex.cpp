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
GLuint uTransform_loc = 0, uMaterial_loc;
GLuint uboTransform, uboMaterial;
Shader uniformShader("test-uniform");

namespace Buffer
{
	enum type 
	{
		POSITION,
		COLOR,
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

GLsizeiptr const ColorSize = VertexCount * sizeof(glm::u8vec4);
glm::u8vec4 const ColorData[VertexCount] = {
		glm::u8vec4(255,   0,   0, 255),
		glm::u8vec4(255, 255,   0, 255),
		glm::u8vec4(  0, 255,   0, 255),
		glm::u8vec4(  0,   0, 255, 255),
		glm::u8vec4(255, 128, 128, 255),
		glm::u8vec4(255, 255, 128, 255),
		glm::u8vec4(128, 255, 128, 255),
		glm::u8vec4(128, 128, 255, 255)
	};

GLsizei const IndiceCount(6);
GLsizeiptr const IndiceSize = IndiceCount * sizeof(GLushort);
GLushort const indices[IndiceCount] = {
	0, 1, 2,
	2, 3, 0,
};

void load_shaders() //加载shader文件
{
	uniformShader.init();
	uniformShader.attach(GL_VERTEX_SHADER, "draw-base-vertex.vert");
	uniformShader.attach(GL_FRAGMENT_SHADER, "draw-base-vertex.frag");
	uniformShader.link();
	uniformShader.use();
	program = uniformShader.program;  
	mvp_loc = glGetUniformLocation(program, "mvp_matrix");
	uTransform_loc = glGetUniformBlockIndex(program, "transform");//获取uniform在shader中的位置索引
	uMaterial_loc  = glGetUniformBlockIndex(program, "material");  //这个索引指示shader内ub的位置

	glUniformBlockBinding(program, uTransform_loc, 3); //将位置索引绑定到绑定点 即layout内的location,与缓存对象交互接口
	glUniformBlockBinding(program, uMaterial_loc, 4);

}

void init_buffer()
{
	glGenBuffers(Buffer::MAX, &buffer[0]);	 //创建数据缓存对象

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer[Buffer::ELEMENT]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, IndiceSize, indices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, buffer[Buffer::POSITION]);
	glBufferData(GL_ARRAY_BUFFER, VertexSize, VertexData, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0); //传完数据解除绑定

	glBindBuffer(GL_ARRAY_BUFFER, buffer[Buffer::COLOR]);
	glBufferData(GL_ARRAY_BUFFER, ColorSize, ColorData, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	//check Error
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
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer[Buffer::ELEMENT]);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
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
		glm::mat4 mvp_matrix= proj_matrix * model_matrix;
		*matrix = mvp_matrix;
	glUnmapBuffer(GL_UNIFORM_BUFFER);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glViewport(0, 0, static_cast<GLsizei>(Width), static_cast<GLsizei>(Height));
	const static float Depth(1.0f);
	glClearBufferfv(GL_DEPTH, 0, &Depth);
	glClearBufferfv(GL_COLOR, 0, &glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)[0]);

	glUseProgram(program);  //将uniform值传入shader
	glBindBufferBase(GL_UNIFORM_BUFFER, 3, buffer[Buffer::TRANSFORM]);  //将缓存对象绑定到绑定点上
	glBindVertexArray(vao);  //绘制前绑定vao选择绘制数据，基于实例索引绘制

	glDrawElementsInstancedBaseVertex(GL_TRIANGLES, IndiceCount, GL_UNSIGNED_SHORT, 0, 1, 0);
	glDrawElementsInstancedBaseVertex(GL_TRIANGLES, IndiceCount, GL_UNSIGNED_SHORT, 0, 1, 4);
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