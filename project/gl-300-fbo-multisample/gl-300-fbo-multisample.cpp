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
		:Position(pos), TexCoord(tex) {}  //初始化列表
	glm::vec2 Position;  
	glm::vec2 TexCoord;
};

// With DDS textures, v texture coordinate are reversed, from top to bottom
GLsizei const VertexCount(6);
GLsizeiptr const VertexSize = VertexCount * sizeof(Vertex);
//顶点数据数组
Vertex const VertexData[VertexCount] = { //纹理怎么运用到上面
		Vertex( glm::vec2(-2.0f,-1.5f), glm::vec2(0.0f, 0.0f) ),
		Vertex( glm::vec2( 2.0f,-1.5f), glm::vec2(1.0f, 0.0f) ),
		Vertex( glm::vec2( 2.0f, 1.5f), glm::vec2(1.0f, 1.0f) ),
		Vertex( glm::vec2( 2.0f, 1.5f), glm::vec2(1.0f, 1.0f) ),
		Vertex( glm::vec2(-2.0f, 1.5f), glm::vec2(0.0f, 1.0f) ),
		Vertex( glm::vec2(-2.0f,-1.5f), glm::vec2(0.0f, 0.0f) )
};


void load_shaders() //加载shader文件
{
	fboShader.init();
	fboShader.attach(GL_VERTEX_SHADER, "image-2d.vert");
	fboShader.attach(GL_FRAGMENT_SHADER, "image-2d.frag");
	fboShader.link();
	fboShader.use();
	program = fboShader.program;  //获取uniform的位置索引
	mvp_loc = glGetUniformLocation(program, "mvp_matrix");
	sampler_loc = glGetUniformLocation(program, "sampler");

}

void init_buffer()
{
	glGenBuffers(1, &vbo);  //创建数据缓存对象
	glBindBuffer(GL_ARRAY_BUFFER, vbo);  //将顶点数据传入
	glBufferData(GL_ARRAY_BUFFER, VertexSize, VertexData, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0); //传完数据解除绑定
	//check Error
}

void init_textures()
{
	glGenTextures(1, &tex);  //创建纹理对象
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);   

	gli::texture2D Texture( gli::load_dds("../media/kueken3-bgr8.dds"));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0); //选择纹理滤波方式
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
	glGenVertexArrays(1, &vao);  //创建顶点数组对象管理状态
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);  //将vbo中的数据传送到shader中去
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), BUFFER_OFFSET(0));
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), BUFFER_OFFSET(sizeof(glm::vec2)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);
}


void init_framebuffer()
{
	glGenRenderbuffers(1, &colorBuffer); //创建渲染缓存对象
	glBindRenderbuffer(GL_RENDERBUFFER, colorBuffer);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, 8, GL_RGBA8, fboWidth, fboHeight);
	//创建存储图像信息的空间

	glGenFramebuffers(1, &fboRender);  //创建帧缓存对象
	glBindFramebuffer(GL_FRAMEBUFFER, fboRender);  //将上面创建的渲染缓存关联到当前帧缓存对象，即把渲染的内容渲染到帧缓存
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glGenTextures(1, &colorTexture); //创建纹理存储上面帧缓存生成的图像
	glBindTexture(GL_TEXTURE_2D, colorTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, fboWidth, fboHeight , 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

	glGenFramebuffers(1, &fboTarget);  //创建显示在窗口的fbo，内容为前一个fbo的渲染图像
	glBindFramebuffer(GL_FRAMEBUFFER, fboTarget);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void init()
{ 

	load_shaders();  //初始化
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

	    //mvp变换矩阵
	    glm::mat4 proj_matrix = glm::perspective(glm::pi<float>() * 0.25f, static_cast<float>(Width) / Height, 0.1f, 100.0f);
		glm::mat4 model_matrix = glm::mat4(1.0f);
		glm::mat4 mvp_matrix= proj_matrix * model_matrix;

		glViewport(0, 0, static_cast<GLsizei>(fboWidth), static_cast<GLsizei>(fboHeight));
		glClearColor(0.0f, 0.5f, 1.0f, 1.0f); //设置背景色
		glClear(GL_COLOR_BUFFER_BIT);

		glActiveTexture(GL_TEXTURE0);  //应用加载图像纹理
		glBindTexture(GL_TEXTURE_2D, tex);

		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, VertexCount);

}

void renderFB(GLuint Texture2DName)
{
	    //mvp变换矩阵
	    glm::mat4 proj_matrix = glm::perspective(glm::pi<float>() * 0.25f, static_cast<float>(Width) / Height, 0.1f, 100.0f);
		glm::mat4 model_matrix = glm::mat4(1.0f);
		glm::mat4 mvp_matrix= proj_matrix * model_matrix;

		glActiveTexture(GL_TEXTURE0); //应用fbo生成的图像作为纹理
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
		glEnable(GL_MULTISAMPLE);  //首先绘制离屏fbo内容
		renderFBO(fboRender);
		glDisable(GL_MULTISAMPLE);

		// Resolved multisampling
		glBindFramebuffer(GL_READ_FRAMEBUFFER, fboRender); //绑定绘制fbo
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboTarget);
		glBlitFramebuffer(
			0, 0, fboWidth, fboHeight, 
			0, 0, fboWidth, fboHeight, 
			GL_COLOR_BUFFER_BIT, GL_LINEAR);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Pass 2
		// Render the colorbuffer from the multisampled framebuffer
		glViewport(0, 0, Width, Height);
		renderFB(colorTexture);  //绘制最终显示图像
		glutSwapBuffers();
};

void end()
{
	glDeleteBuffers(1, &vbo); //清除状态
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