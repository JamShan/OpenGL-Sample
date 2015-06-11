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
//顶点数据数组
glm::vec2 const VertexData[VertexCount] = { //纹理怎么运用到上面
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

void load_shaders() //加载shader文件
{
	uniformShader.init();
	uniformShader.attach(GL_VERTEX_SHADER, "buffer-uniform.vert");
	uniformShader.attach(GL_FRAGMENT_SHADER, "buffer-uniform.frag");
	uniformShader.link();
	uniformShader.use();
	program = uniformShader.program;  
	mvp_loc = glGetUniformLocation(program, "mvp_matrix");
	uTransform_loc = glGetUniformBlockIndex(program, "transform");//获取uniform在shader中的位置索引
	uMaterial_loc  = glGetUniformBlockIndex(program, "material");  //这个索引指示shader内ub的位置

	glUniformBlockBinding(program, uTransform_loc, 0); //将位置索引绑定到绑定点 即layout内的location,与缓存对象交互接口
	glUniformBlockBinding(program, uMaterial_loc, 1);

}

void init_buffer()
{
	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, IndiceSize, indices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glGenBuffers(1, &vbo);  //创建数据缓存对象
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, VertexSize, VertexData, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0); //传完数据解除绑定
	//check Error
	GLint UniformBlockSize = 0; //查询uniform块在shader中占的空间大小
	glGetActiveUniformBlockiv(program, uTransform_loc, GL_UNIFORM_BLOCK_DATA_SIZE, &UniformBlockSize);
	glGenBuffers(1, &uboTransform);  //创建存储tranform块的缓存对象
	glBindBuffer(GL_UNIFORM_BUFFER, uboTransform);
	glBufferData(GL_UNIFORM_BUFFER, UniformBlockSize, 0, GL_DYNAMIC_DRAW); //根据shader占据的空间分配缓存对象空间
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glm::vec4 Diffuse(1.0f, 0.5f, 0.0f, 1.0f);
	glGetActiveUniformBlockiv(program, uMaterial_loc, GL_UNIFORM_BLOCK_DATA_SIZE, &UniformBlockSize);
	glGenBuffers(1, &uboMaterial);   //创建存储material块的缓存对象
	glBindBuffer(GL_UNIFORM_BUFFER, uboMaterial);
	glBufferData(GL_UNIFORM_BUFFER, UniformBlockSize, &Diffuse[0], GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}


void init_vertexArray()
{
	glGenVertexArrays(1, &vao);  //创建顶点数组对象管理状态
	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);  //将vbo中的数据传送到shader中去
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBindVertexArray(0);
}


void init()
{ 
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDrawBuffer(GL_BACK);
	load_shaders();  //初始化
	init_buffer();
	init_vertexArray();
}

void render(void)
{

	glBindBuffer(GL_UNIFORM_BUFFER, uboTransform); //绑定tranform的ubo，初始化数据
	glm::mat4 *matrix = (glm::mat4*) glMapBufferRange(GL_UNIFORM_BUFFER,
		0, sizeof(glm::mat4), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
	    //mvp变换矩阵
	    glm::mat4 proj_matrix = glm::perspective(45.0f, static_cast<float>(Width) / Height, 0.1f, 100.0f);
		*matrix = proj_matrix;
	glUnmapBuffer(GL_UNIFORM_BUFFER);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glm::mat4 mvp_matrix = glm::perspective(45.0f, 1.0f, 1.0f, 1000.0f);
	glUniformMatrix4fv(glGetUniformLocation(program, "mvp_matrix"),1, GL_FALSE, &mvp_matrix[0][0]);
	glViewport(0, 0, static_cast<GLsizei>(Width), static_cast<GLsizei>(Height));
	glClearBufferfv(GL_COLOR, 0, &glm::vec4(1.0f)[0]);

	glUseProgram(program);  //将uniform值传入shader
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, uboTransform);  //将缓存对象绑定到绑定点上
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, uboMaterial);

	glBindVertexArray(vao);  //绘制前绑定vao选择绘制数据，基于实例索引绘制
	glDrawElementsInstancedBaseVertex(GL_TRIANGLES, IndiceCount, GL_UNSIGNED_SHORT, nullptr, 1, 0);
	glutSwapBuffers();
}

void end()
{
	glDeleteBuffers(1, &vbo); //清除状态
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