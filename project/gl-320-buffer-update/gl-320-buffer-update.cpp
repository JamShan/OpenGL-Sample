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
GLuint vboRead, vboCopy, vao = 0, ibo = 0;
GLuint uTransform_loc = 0, uMaterial_loc;
GLuint uboTransform, uboMaterial;
Shader updateShader("test-buffer-update");

// With DDS textures, v texture coordinate are reversed, from top to bottom
GLsizei const VertexCount(6);
GLsizeiptr const VertexSize = VertexCount * sizeof(glm::vec2);
//顶点数据数组
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
	updateShader.init();
	updateShader.attach(GL_VERTEX_SHADER, "buffer-update.vert");
	updateShader.attach(GL_FRAGMENT_SHADER, "buffer-update.frag");
	updateShader.link();
	updateShader.use();
	program = updateShader.program;  
	mvp_loc = glGetUniformLocation(program, "mvp_matrix");
	uTransform_loc = glGetUniformBlockIndex(program, "transform");//获取uniform在shader中的位置索引
	uMaterial_loc  = glGetUniformBlockIndex(program, "material");  //这个索引指示shader内ub的位置

	glUniformBlockBinding(program, uTransform_loc, 3); //将位置索引绑定到绑定点 即layout内的location,与缓存对象交互接口
	glUniformBlockBinding(program, uMaterial_loc, 4);

}

void init_buffer()
{
	glGenBuffers(1, &vboRead);  //创建数据缓存对象
	glBindBuffer(GL_ARRAY_BUFFER, vboRead);
	glBufferData(GL_ARRAY_BUFFER, VertexSize, 0, GL_STATIC_DRAW);
	// Copy the vertex data in the buffer, in this sample for the whole range of data.
		// It doesn't required to be the buffer size but pointers require no memory overlapping.
	GLvoid* Data = glMapBufferRange( //将数据映射到缓存对象区
			GL_ARRAY_BUFFER, 
			0,				// Offset
			VertexSize,	// Size,
			GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_UNSYNCHRONIZED_BIT | GL_MAP_FLUSH_EXPLICIT_BIT);
		memcpy(Data, VertexData, VertexSize);
		// Explicitly send the data to the graphic card.
	 glFlushMappedBufferRange(GL_ARRAY_BUFFER, 0,VertexSize);

	 glUnmapBuffer(GL_ARRAY_BUFFER);
	 glBindBuffer(GL_ARRAY_BUFFER, 0); //传完数据解除绑定
	 	// Copy buffer
	glBindBuffer(GL_ARRAY_BUFFER, vboCopy);  //创建复制缓存对象
	glBufferData(GL_ARRAY_BUFFER, VertexSize, 0, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_COPY_READ_BUFFER, vboRead); //设置读写缓存共享
	glBindBuffer(GL_COPY_WRITE_BUFFER, vboCopy);

	glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER,
			            0, 0, VertexSize); //将read中数据传到copy
	glBindBuffer(GL_COPY_READ_BUFFER, 0);
	glBindBuffer(GL_COPY_WRITE_BUFFER, 0);

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

	glBindBuffer(GL_ARRAY_BUFFER, vboCopy);  //将vbo中的数据传送到shader中去
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void init()
{ 

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
	    glm::mat4 proj_matrix = glm::perspective(glm::pi<float>() * 0.25f, static_cast<float>(Width) / Height, 0.1f, 100.0f);
		glm::mat4 model_matrix = glm::mat4(1.0f);
		glm::mat4 mvp_matrix= proj_matrix * model_matrix;
		*matrix = mvp_matrix;
	glUnmapBuffer(GL_UNIFORM_BUFFER);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glViewport(0, 0, static_cast<GLsizei>(Width), static_cast<GLsizei>(Height));
	glClearBufferfv(GL_COLOR, 0, &glm::vec4(1.0f)[0]);

	glUseProgram(program);  //将uniform值传入shader
	glBindBufferBase(GL_UNIFORM_BUFFER, 3, uboTransform);  //将缓存对象绑定到绑定点上
	glBindBufferBase(GL_UNIFORM_BUFFER, 4, uboMaterial);

	glBindVertexArray(vao);  //绘制前绑定vao选择绘制数据，基于实例索引绘制
	glDrawArraysInstanced(GL_TRIANGLES, 0, VertexCount, 1);
}

void end()
{
	glDeleteBuffers(1, &vboCopy); //清除状态
	glDeleteBuffers(1, &vboRead); //清除状态
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