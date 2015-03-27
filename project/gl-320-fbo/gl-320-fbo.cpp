#include<gl/glew.h>
#include<GL/freeglut.h>
#include<glm/glm.hpp>
#include<shader.h>
#include<gli/gli.hpp>
#include<glm/gtc/matrix_transform.hpp>

#define BUFFER_OFFSET(offset) (GLvoid*)(NULL + offset)

const GLsizei VertexCount(8);
const GLsizeiptr VertexSize = VertexCount * sizeof(glm::vec2);
const glm::vec2 VertexData[VertexCount] = {
	glm::vec2(-1.0f,-1.0f), glm::vec2(0.0f, 1.0f),
	glm::vec2( 1.0f,-1.0f), glm::vec2(1.0f, 1.0f),
	glm::vec2( 1.0f, 1.0f), glm::vec2(1.0f, 0.0f),
	glm::vec2(-1.0f, 1.0f), glm::vec2(0.0f, 0.0f)
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

namespace Texture {
	enum type {
		DIFFUSE,
		COLORBUFFER,
		RENDERBUFFER,
		MAX
	};
}

Shader splashShader("splash");
Shader textureShader("texture");
std::vector<GLuint> buffer(Buffer::MAX);
std::vector<GLuint> texture(Texture::MAX);
GLuint splash_vao, texture_vao, fbo;
GLuint vertex_loc, tc_loc, transform_loc;
GLuint texture_program, splash_program;
GLuint splash_diffuse_loc, texture_diffuse_loc;

void init_shader()
{
	textureShader.init();
	textureShader.attach(GL_VERTEX_SHADER, "texture-2d.vert");
	textureShader.attach(GL_FRAGMENT_SHADER, "texture-2d.frag");
	textureShader.link();
	texture_program = textureShader.program;
	transform_loc = glGetUniformBlockIndex(texture_program, "transform");
	texture_diffuse_loc = glGetUniformLocation(texture_program, "Diffuse");
	glUseProgram(texture_program);
	glUniform1i(texture_diffuse_loc, 0);
	glUniformBlockBinding(texture_program, transform_loc, 3);

	splashShader.init();
	splashShader.attach(GL_VERTEX_SHADER, "splash.vert");
	splashShader.attach(GL_FRAGMENT_SHADER, "splash.frag");
	splashShader.link();
	splash_program = splashShader.program;
	splash_diffuse_loc = glGetUniformLocation(splash_program, "Diffuse");
	glUseProgram(splash_program);
	glUniform1i(splash_diffuse_loc, 0);

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
		assert(!Texture.empty());
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		glGenTextures(Texture::MAX, &texture[0]);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture[Texture::DIFFUSE]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, GLint(Texture.levels() - 1));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		for(std::size_t Level = 0; Level < Texture.levels(); ++Level)
		{
			glCompressedTexImage2D(GL_TEXTURE_2D,
				GLint(Level),
				GL_COMPRESSED_RGB_S3TC_DXT1_EXT,
				GLsizei(Texture[Level].dimensions().x), 
				GLsizei(Texture[Level].dimensions().y), 
				0, 
				GLsizei(Texture[Level].size()), 
				Texture[Level].data());
		}
	
		glm::ivec2 WindowSize(200.0, 200.0);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture[Texture::COLORBUFFER]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, GLint(0), GL_RGBA8, GLsizei(WindowSize.x), GLsizei(WindowSize.y), 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture[Texture::RENDERBUFFER]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
		glTexImage2D(GL_TEXTURE_2D, GLint(0), GL_DEPTH_COMPONENT24, GLsizei(WindowSize.x), GLsizei(WindowSize.y), 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

}

void init_vertexArray()
{
	glGenVertexArrays(1, &texture_vao);
	glBindVertexArray(texture_vao);
	glBindBuffer(GL_ARRAY_BUFFER, buffer[Buffer::VERTEX]);
	vertex_loc = glGetAttribLocation(texture_program, "Position");
	tc_loc = glGetAttribLocation(texture_program, "TexCoord");
	glVertexAttribPointer(vertex_loc, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), 0);
	glVertexAttribPointer(tc_loc, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), BUFFER_OFFSET(sizeof(glm::vec2)));
	glEnableVertexAttribArray(vertex_loc);
	glEnableVertexAttribArray(tc_loc);
	glBindVertexArray(0);

	glGenVertexArrays(1, &splash_vao);
	glBindVertexArray(splash_vao);
	glBindVertexArray(0);
}

void init_fbo()
{
	glGenFramebuffers(1, &fbo); //创建fbo保存深度和颜色信息
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture[Texture::COLORBUFFER], 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texture[Texture::RENDERBUFFER], 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void init()
{
	 init_shader();
	 init_buffer();
	 init_vertexArray();
	 init_texture();
}

void render()
{
	glBindBuffer(GL_UNIFORM_BUFFER, buffer[Buffer::TRANSFORM]);
	glm::mat4 *Pointer = (glm::mat4*)glMapBufferRange(
		GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4),
		GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

	glm::mat4 proj_matrix  = glm::perspective(glm::pi<float>() * 0.25f, 720.0f / 640.0f, 0.1f, 100.0f);
	glm::mat4 model_matrix = glm::mat4(1.0f);
	glm::mat4 view_matrix = glm::lookAt(glm::vec3(0.0, 0.0, 5.0), glm::vec3(0.0, 0.0, 0.0),
		                                glm::vec3(0.0, 1.0, 0.0));

	*Pointer = proj_matrix * view_matrix * model_matrix;

    glUnmapBuffer(GL_UNIFORM_BUFFER);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glViewport(0, 0, 200, 200); //绘制fbo的图像作为纹理
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	static const float depth(1.0f);
	glClearBufferfv(GL_DEPTH, 0, &depth);
	glClearBufferfv(GL_COLOR, 0, &glm::vec4(1.0f, 0.5f, 0.0f, 1.0f)[0]);
	glUseProgram(texture_program);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture[Texture::DIFFUSE]);
	glBindVertexArray(texture_vao);
	glBindBufferBase(GL_UNIFORM_BUFFER, transform_loc, buffer[Buffer::TRANSFORM]);
	glDrawElementsInstancedBaseVertex(GL_TRIANGLES, ElementCount, GL_UNSIGNED_SHORT, 0, 2, 0);
	
	glDisable(GL_DEPTH_TEST); //使用上面的fbo作为纹理
	glViewport(0, 0, 720, 640);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glUseProgram(splash_program);
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(splash_vao);
	glBindTexture(GL_TEXTURE_2D, texture[Texture::COLORBUFFER]);
	glDrawArraysInstanced(GL_TRIANGLES, 0, 3, 1);
}


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