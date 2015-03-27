#include<gl/glew.h>
#include<GL/freeglut.h>
#include<glm/glm.hpp>
#include<shader.h>
#include<gli/gli.hpp>
#include<glm/gtc/matrix_transform.hpp>

#define BUFFER_OFFSET(offset) (GLvoid*)(NULL + offset)

const GLsizei VertexCount(12);
const GLsizeiptr VertexSize = VertexCount * sizeof(glm::vec2);
const glm::vec2 VertexData[VertexCount] = {
		glm::vec2(-1.5f,-1.5f), glm::vec2(0.0f, 0.0f),
		glm::vec2( 1.5f,-1.5f), glm::vec2(1.0f, 0.0f),
		glm::vec2( 1.5f, 1.5f), glm::vec2(1.0f, 1.0f),
        glm::vec2( 1.5f, 1.5f), glm::vec2(1.0f, 1.0f),
		glm::vec2(-1.5f, 1.5f), glm::vec2(0.0f, 1.0f),
		glm::vec2(-1.5f,-1.5f), glm::vec2(0.0f, 0.0f)
};

const GLsizei ElementCount(6);
const GLsizeiptr ElementSize = ElementCount * sizeof(GLushort); 
const GLushort ElementData[ElementCount] = {
	    0, 1, 2,
		2, 3, 0
};

namespace Buffer {
	enum type {
		VERTEX,
		ELEMENT,
		TRANSFORM,
		MAX
	};
}
namespace Framebuffer {
		enum type
		{
			RENDER,
			RESOLVE,
			MAX
		};
}//namespace framebuffer

namespace Texture {
		enum type {
			DIFFUSE,
			COLORBUFFER,
			MAX
		};
}//namespace

Shader splashShader("splash");
Shader fboBlitShader("fboBlit");
std::vector<GLuint> buffer(Buffer::MAX);
std::vector<GLuint> texture(Texture::MAX);
std::vector<GLuint> fbo(Framebuffer::MAX);
GLuint vao, rbo;
GLuint vertex_loc, tc_loc, transform_loc;
GLuint program;
GLuint diffuse_loc;

void init_shader()
{
	fboBlitShader.init();
	fboBlitShader.attach(GL_VERTEX_SHADER, "fboBlit-2d.vert");
	fboBlitShader.attach(GL_FRAGMENT_SHADER, "fboBlit-2d.frag");
	fboBlitShader.link();
	program = fboBlitShader.program;
	transform_loc = glGetUniformLocation(program, "transform");
	diffuse_loc = glGetUniformLocation(program, "Diffuse");
	glUniform1i(diffuse_loc, 0);
}

void init_buffer()
{
	glGenBuffers(Buffer::MAX, &buffer[0]);

	glBindBuffer(GL_ARRAY_BUFFER, buffer[Buffer::VERTEX]);
	glBufferData(GL_ARRAY_BUFFER, VertexSize, VertexData, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer[Buffer::ELEMENT]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, ElementSize, ElementData, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	GLint UniformBufferOffset(0);
	glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &UniformBufferOffset);
	GLint UniformBlockSize = glm::max(GLint(sizeof(glm::mat4)), UniformBufferOffset);

	glBindBuffer(GL_UNIFORM_BUFFER, buffer[Buffer::TRANSFORM]);
	glBufferData(GL_UNIFORM_BUFFER, UniformBlockSize, NULL, GL_DYNAMIC_DRAW);\
}

void init_texture()
{
		gli::texture2D Texture(gli::load_dds((getDataDirectory() + TEXTURE_DIFFUSE).c_str()));

		glGenTextures(Texture::MAX, &texture[0]);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture[Texture::DIFFUSE]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, GLint(Texture.levels() - 1));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		for(std::size_t Level = 0; Level < Texture.levels(); ++Level)
		{
			glTexImage2D(GL_TEXTURE_2D,
				GLint(Level),
				GL_RGB8,
				GLsizei(Texture[Level].dimensions().x), GLsizei(Texture[Level].dimensions().y),
				0,
				GL_BGR, GL_UNSIGNED_BYTE,
				Texture[Level].data());
		}
		glGenerateMipmap(GL_TEXTURE_2D); // Allocate all mipmaps memory

		glBindTexture(GL_TEXTURE_2D, texture[Texture::COLORBUFFER]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,520, 520, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

		glBindTexture(GL_TEXTURE_2D, 0);
}

void init_vertexArray()
{
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, buffer[Buffer::VERTEX]);
	vertex_loc = glGetAttribLocation(program, "Position");
	tc_loc = glGetAttribLocation(program, "TexCoord");
	glVertexAttribPointer(vertex_loc, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), 0);
	glVertexAttribPointer(tc_loc, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), BUFFER_OFFSET(sizeof(glm::vec2)));
	glEnableVertexAttribArray(vertex_loc);
	glEnableVertexAttribArray(tc_loc);
	glBindVertexArray(0);
}

void init_fbo()
{
	glGenFramebuffers(Framebuffer::MAX, &fbo[0]); //创建fbo保存深度和颜色信息
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, 520, 520);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo[Framebuffer::RENDER]);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
		                      GL_RENDERBUFFER, rbo);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, fbo[Framebuffer::RESOLVE]);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
		                 fbo[Framebuffer::RESOLVE], 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void init()
{
	 init_shader();
	 init_buffer();
	 init_vertexArray();
	 init_fbo();
}

void render_fbo()
{
	glm::mat4 proj_matrix  = glm::perspective(glm::pi<float>() * 0.25f, 720.0f / 640.0f, 0.1f, 100.0f);
	glm::mat4 model_matrix = glm::mat4(1.0f);
	glm::mat4 view_matrix = glm::lookAt(glm::vec3(0.0, 0.0, 5.0), glm::vec3(0.0, 0.0, 0.0),
		                                glm::vec3(0.0, 1.0, 0.0));

	glm::mat4 mvp = proj_matrix * view_matrix * model_matrix;
	glUniformMatrix4fv(transform_loc, 1, GL_FALSE, &mvp[0][0]);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture[Texture::DIFFUSE]);
	glBindVertexArray(vao);
	static const float depth(1.0f);
	glClearBufferfv(GL_DEPTH, 0, &depth);
	glClearBufferfv(GL_COLOR, 0, &glm::vec4(1.0f, 0.5f, 0.0f, 1.0f)[0]);
	glDrawArraysInstanced(GL_TRIANGLES, 0, VertexCount, 1);

}

void render_fb()
{
		glm::mat4 Perspective = glm::perspective(glm::pi<float>() * 0.25f, 720.0f / 640.0f, 0.1f, 100.0f);
		glm::mat4 Model = glm::mat4(1.0f);
		glm::mat4 MVP = Perspective * Model;
		glUniformMatrix4fv(transform_loc, 1, GL_FALSE, &MVP[0][0]);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture[Texture::COLORBUFFER]);
		glBindVertexArray(vao);
		glDrawArraysInstanced(GL_TRIANGLES, 0, VertexCount, 1);
}

void render()
{
		glm::vec2 WindowSize(720.0, 640.0);

		glUseProgram(program);
		glUniform1i(diffuse_loc, 0);

		// Pass 1
		glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName[framebuffer::RENDER]);
		glViewport(0, 0, FRAMEBUFFER_SIZE.x, FRAMEBUFFER_SIZE.y);
		glClearBufferfv(GL_COLOR, 0, &glm::vec4(0.0f, 0.5f, 1.0f, 1.0f)[0]);
		render_fbo();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Generate FBO mipmaps
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TextureName[texture::COLORBUFFER]);
		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);

		// Blit framebuffers
		GLint const Border = 2;
		int const Tile = 4;
		glBindFramebuffer(GL_READ_FRAMEBUFFER, FramebufferName[framebuffer::RENDER]);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, FramebufferName[framebuffer::RESOLVE]);
		glClearBufferfv(GL_COLOR, 0, &glm::vec4(1.0f, 0.5f, 0.0f, 1.0f)[0]);
		
		for(int j = 0; j < Tile; ++j)
		for(int i = 0; i < Tile; ++i)
		{
			if((i + j) % 2)
				continue;

			glBlitFramebuffer(0, 0, FRAMEBUFFER_SIZE.x, FRAMEBUFFER_SIZE.y,
				FRAMEBUFFER_SIZE.x / Tile * (i + 0) + Border, 
				FRAMEBUFFER_SIZE.x / Tile * (j + 0) + Border, 
				FRAMEBUFFER_SIZE.y / Tile * (i + 1) - Border, 
				FRAMEBUFFER_SIZE.y / Tile * (j + 1) - Border, 
				GL_COLOR_BUFFER_BIT, GL_LINEAR);
		}

		// Pass 2
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, static_cast<GLsizei>(WindowSize.x), static_cast<GLsizei>(WindowSize.y));
		glClearBufferfv(GL_COLOR, 0, &glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)[0]);
		render_fb();
};

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowPosition(300, 150);
	glutInitWindowSize(720, 640);
	glutCreateWindow("Fbo");
	glewInit();
	init();
	glutDisplayFunc(render);
	glutMainLoop();
}