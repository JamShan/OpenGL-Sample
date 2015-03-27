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

#define BUFFER_OFFSET(offset) ((GLvoid*) (NULL + offset)) //位置指针

const int posX(300), posY(0);
const int Width(720), Height(640);
const char *Title = "gl-320-texture-2d";
const char *vs_shader_code("texture-2d.vert");
const char *fs_shader_code("texture-2d.frag");
GLuint program = 0, vao = 0, texture;
GLuint mvp_loc = 0;
GLuint uTransform_loc = 0, uMaterial_loc;
GLuint uboTransform, uboMaterial;
Shader textureShader("texture-2d");

namespace Buffer
{
	enum type 
	{
		VEXTER,  //顶点
		ELEMENT,   //索引
		TRANSFORM, //uniform 块
		MAX,   //数量
	};
}
array<GLuint, Buffer::MAX> buffer;  //缓存对象索引数组

// With DDS textures, v texture coordinate are reversed, from top to bottom
GLsizei const VertexCount(8);  
GLsizeiptr const VertexSize = VertexCount * sizeof(glm::vec2);
//顶点数据数组
glm::vec2 const VertexData[VertexCount] = { //纹理怎么运用到上面
		glm::vec2(-1.0f,-1.0f), glm::vec2(0.0f, 1.0f),
		glm::vec2( 1.0f,-1.0f), glm::vec2(1.0f, 1.0f),
		glm::vec2( 1.0f, 1.0f), glm::vec2(1.0f, 0.0f),
		glm::vec2(-1.0f, 1.0f), glm::vec2(0.0f, 0.0f)
};

GLsizei const ElementCount(6); //索引信息
GLsizeiptr const ElementSize = ElementCount * sizeof(glm::uint32);
glm::uint32 const ElementData[ElementCount] = {
		0, 1, 2,
		0, 2, 3
};


void load_shaders() //加载shader文件
{
	textureShader.init();
	textureShader.attach(GL_VERTEX_SHADER, vs_shader_code);
	textureShader.attach(GL_FRAGMENT_SHADER, fs_shader_code);
	textureShader.link();
	textureShader.use();
	program = textureShader.program; 
	uTransform_loc = glGetUniformBlockIndex(program, "transform");//获取uniform在shader中的位置索引
	glUniformBlockBinding(program, uTransform_loc, 3); //将位置索引绑定到绑定点 即layout内的location,与缓存对象交互接口

}

void init_buffer()
{
	glGenBuffers(Buffer::MAX, &buffer[0]);	 //创建数据缓存对象

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer[Buffer::ELEMENT]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, ElementSize, ElementData, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, buffer[Buffer::VEXTER]);
	glBufferData(GL_ARRAY_BUFFER, VertexSize, VertexData, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0); //传完数据解除绑定

	GLint UniformBufferOffset(0);
	glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &UniformBufferOffset);
	GLint UniformBlockSize = glm::max(GLint(sizeof(glm::mat4)), UniformBufferOffset);
	glBindBuffer(GL_UNIFORM_BUFFER, buffer[Buffer::TRANSFORM]);
	glBufferData(GL_UNIFORM_BUFFER, UniformBlockSize, 0, GL_DYNAMIC_DRAW); //根据shader占据的空间分配缓存对象空间
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

}

void init_texture()
{
	gli::texture2D Texture(gli::RGBA8_UNORM, gli::texture2D::dimensions_type(64));
	gli::texture2D::size_type Level = Texture.levels();

	Texture[0].clear(glm::u8vec4(255, 0, 0, 255));
	Texture[1].clear(glm::u8vec4(255, 128, 0, 255));
	Texture[2].clear(glm::u8vec4(255, 255, 0, 255));
	Texture[3].clear(glm::u8vec4(0, 255, 0, 255));
	Texture[4].clear(glm::u8vec4(0, 255, 255, 255));
	Texture[5].clear(glm::u8vec4(0, 0, 255, 255));
	Texture[6].clear(glm::u8vec4(255, 0, 0, 255));

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glGenTextures(1, &texture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, GLint(Texture.levels() - 1));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	for(gli::texture2D::size_type Level = 0; Level < Texture.levels(); ++Level)
		{
			glTexImage2D(GL_TEXTURE_2D,
				GLint(Level),
				gli::internal_format(Texture.format()),
				GLsizei(Texture[Level].dimensions().x),
				GLsizei(Texture[Level].dimensions().y),
				0,
				gli::external_format(Texture.format()),
				gli::type_format(Texture.format()),
				Texture[Level].data());
	}
	if(Texture.levels() == 1)
		glGenerateMipmap(GL_TEXTURE_2D);
	
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

}

void init_vertexArray()
{
	glGenVertexArrays(1, &vao);  //创建顶点数组对象管理状态
	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, buffer[Buffer::VEXTER]);  //将vbo中的数据传送到shader中去
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2) * 2, BUFFER_OFFSET(0)); 
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2) * 2 ,  BUFFER_OFFSET(sizeof(glm::vec2)));
	                       //index, size, type, normalized, stride, pointer
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer[Buffer::ELEMENT]);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void init()
{ 
	load_shaders();  //初始化
	init_buffer();
	init_texture();
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


	glDrawBuffer(GL_BACK); //four color buffer in framebuffer, GL_BACK for double-buffered contexts.
	glViewport(0, 0, static_cast<GLsizei>(Width), static_cast<GLsizei>(Height));
	glClearBufferfv(GL_COLOR, 0, &glm::vec4(1.0f, 0.5f, 0.0f, 1.0f)[0]);

	glUseProgram(program);  //将uniform值传入shader
	glBindBufferBase(GL_UNIFORM_BUFFER, 3, buffer[Buffer::TRANSFORM]);  //将缓存对象绑定到绑定点上
	glBindVertexArray(vao);  //绘制前绑定vao选择绘制数据，基于实例索引绘制
    glActiveTexture(GL_TEXTURE0); //激活GL_TEXTURE_2D上纹理0单元
	glBindTexture(GL_TEXTURE_2D, texture);
    glDrawElementsInstancedBaseVertex(GL_TRIANGLES, ElementCount, GL_UNSIGNED_SHORT, 0, 1, 0);
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