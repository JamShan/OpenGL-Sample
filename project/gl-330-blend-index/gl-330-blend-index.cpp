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
const char *Title = "gl-330-blend";

GLuint program = 0;
GLuint mvp_loc = 0;
GLuint vbo = 0, tex = 0, vao = 0;
GLuint sampler_loc = 0, vertex_loc, tc_loc;
const string textureName("kueken1-bgr8.dds"); 
Shader blendShader("test-blend");

struct Vertex {
	Vertex(glm::vec2 const & pos, glm::vec2 const &tex)
		:Position(pos), TexCoord(tex) {}  //初始化列表

	glm::vec2 Position;  
	glm::vec2 TexCoord;
};

// With DDS textures, v texture coordinate are reversed, from top to bottom
GLsizei const VertexCount(6);
GLsizeiptr const VertexSize = VertexCount * sizeof(Vertex);

//顶点数据数组
Vertex const VertexData[VertexCount] = { //纹理怎么运用到上面
		Vertex(glm::vec2(-1.0f,-1.0f), glm::vec2(0.0f, 1.0f)),
		Vertex(glm::vec2( 1.0f,-1.0f), glm::vec2(1.0f, 1.0f)),
		Vertex(glm::vec2( 1.0f, 1.0f), glm::vec2(1.0f, 0.0f)),
		Vertex(glm::vec2( 1.0f, 1.0f), glm::vec2(1.0f, 0.0f)),
		Vertex(glm::vec2(-1.0f, 1.0f), glm::vec2(0.0f, 0.0f)),
		Vertex(glm::vec2(-1.0f,-1.0f), glm::vec2(0.0f, 1.0f))
};


void load_shaders() //加载shader文件
{
	blendShader.init();
	blendShader.attach(GL_VERTEX_SHADER, "blend-index.vert");
	blendShader.attach(GL_FRAGMENT_SHADER, "blend-index.frag");
	blendShader.link();
	blendShader.use();
	program = blendShader.program;  //获取uniform的位置索引
	mvp_loc = glGetUniformLocation(program, "mvp_matrix");
	sampler_loc = glGetUniformLocation(program, "sampler");

}

void init_buffer()
{
	glGenBuffers(1, &vbo);  //创建数据缓存对象
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, VertexSize, VertexData, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0); //传完数据解除绑定
	//check Error
}

void init_textures()
{
	glGenTextures(1, &tex);  //纹理对象
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);   //选择纹理滤波方式
	
	gli::texture2D Texture( gli::load_dds("../media/kueken2-bgra8.dds"));
	for(std::size_t Level = 0; Level < Texture.levels(); ++Level) { //展开每一层次
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
	glGenVertexArrays(1, &vao);  //创建顶点数组对象管理状态
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);  //将vbo中的数据传送到shader中去
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
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_SRC1_COLOR);
	load_shaders();  //初始化
	init_buffer();
	init_textures();
	init_vertexArray();
}

void render(void)
{
	    //mvp变换矩阵
	    glm::mat4 proj_matrix = glm::perspective(glm::pi<float>() * 0.25f, static_cast<float>(Width) / Height, 0.1f, 100.0f);
		glm::mat4 model_matrix = glm::mat4(1.0f);
		glm::mat4 mvp_matrix= proj_matrix * model_matrix;

		glViewport(0, 0, static_cast<GLsizei>(Width), static_cast<GLsizei>(Height));
		glClearColor(1.0f, 0.5f, 0.0f, 1.0f); //设置背景色
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(program);  //将uniform值传入shader
		glUniform1i(sampler_loc, 0);
		glUniformMatrix4fv(mvp_loc, 1, GL_FALSE, &mvp_matrix[0][0]);

		glActiveTexture(GL_TEXTURE0);  //选择前面创建的纹理
		glBindTexture(GL_TEXTURE_2D, tex);

		glBindVertexArray(vao);  //绘制前绑定vao选择绘制数据
		glDrawArraysInstanced(GL_TRIANGLES, 0, VertexCount, 1);
		glutSwapBuffers();
}

void end()
{
	glDeleteBuffers(1, &vbo); //清除状态
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