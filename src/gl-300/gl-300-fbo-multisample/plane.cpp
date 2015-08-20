#include "Plane.h"
#include <gli/gli.hpp>

namespace byhj
{

const int g_FboWidth = 800;
const int g_FboHeight = 600;

void Plane::Init()
{
	init_shader();
	init_buffer();
	init_vertexArray();
	init_texture();
	init_fbo();
}

void Plane::Render()
{
	//Pass 1, render the scene in a multisampled framebuffer
	glUseProgram(m_Program);
	glEnable(GL_MULTISAMPLE);
	glViewport(0, 0, g_FboWidth, g_FboHeight);

	glBindFramebuffer(GL_FRAMEBUFFER, m_RenderFbo);
	glClearColor(0.0f, 0.5f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_ImageTex);

	glBindVertexArray(m_Vao);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);

	glBindTexture(GL_TEXTURE_2D, 0);

	glDisable(GL_MULTISAMPLE);
	glUseProgram(0);

	////////////////////////////////////////////////////////////////

	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_RenderFbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_TargetFbo);
	glBlitFramebuffer(
		0, 0, g_FboWidth, g_FboHeight,
		0, 0, g_FboWidth, g_FboHeight,
		GL_COLOR_BUFFER_BIT, GL_LINEAR
		);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	////////////////////////////////////////////////////////////////

	//Pass 2, render the colorbuffer frorm multisampled framebuffer
	glUseProgram(m_Program);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_ColorTex);

	glBindVertexArray(m_Vao);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);

	glUseProgram(0);

}

void Plane::Shutdown()
{

}


void Plane::init_shader()
{
	m_PlaneShader.init();
	m_PlaneShader.attach(GL_VERTEX_SHADER, "image-2d.vert");
	m_PlaneShader.attach(GL_FRAGMENT_SHADER, "image-2d.frag");
	m_PlaneShader.link();
	m_PlaneShader.info();
	m_Program = m_PlaneShader.GetProgram();

	m_Uniform.tex_loc = glGetUniformLocation(m_Program, "tex");
}

void Plane::init_buffer()
{
	GLsizei const VertexCount(6);
	GLsizeiptr const VertexSize = VertexCount * sizeof(Vertex);

	Plane::Vertex const VertexData[VertexCount] =
	{
		Vertex(glm::vec2(-2.0f, -1.5f), glm::vec2(0.0f, 0.0f)),
		Vertex(glm::vec2( 2.0f, -1.5f), glm::vec2(1.0f, 0.0f)),
		Vertex(glm::vec2( 2.0f, 1.5f ), glm::vec2(1.0f, 1.0f)),
		Vertex(glm::vec2( 2.0f, 1.5f ), glm::vec2(1.0f, 1.0f)),
		Vertex(glm::vec2(-2.0f, 1.5f ), glm::vec2(0.0f, 1.0f)),
		Vertex(glm::vec2(-2.0f, -1.5f), glm::vec2(0.0f, 0.0f))
	};

	glGenBuffers(1, &m_Vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_Vbo);    //load the vertex data
	glBufferData(GL_ARRAY_BUFFER, VertexSize, VertexData, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Plane::init_vertexArray()
{
	glGenVertexArrays(1, &m_Vao);
	glBindVertexArray(m_Vao);

	glBindBuffer(GL_ARRAY_BUFFER, m_Vbo);   //bind the vbo to vao, send the data to shader
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), BUFFER_OFFSET( sizeof(glm::vec2) ) );

	glBindVertexArray(0);
	glDisableVertexAttribArray(0);
}

void Plane::init_texture()
{

	glGenTextures(1, &m_ImageTex);  //创建纹理对象
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_ImageTex);

	gli::texture2D Texture( gli::load_dds("../../../media/textures/kueken3-bgr8.dds") );
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
	glBindTexture(GL_TEXTURE_2D, m_ImageTex);

}
void Plane::init_fbo()
{
	//Create the render buffer
	glGenRenderbuffers(1, &m_ColorRbo);
	glBindRenderbuffer(GL_RENDERBUFFER, m_ColorRbo);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, 8, GL_RGBA8, g_FboWidth, g_FboHeight);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

    //Create the render frambuffer, attach the render buffer
	glGenFramebuffers(1, &m_RenderFbo);
	glBindFramebuffer(GL_FRAMEBUFFER, m_RenderFbo);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_ColorRbo);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//Create a texture to store the framebuffer color buffer
	glGenTextures(1, &m_ColorTex);
	glBindTexture(GL_TEXTURE_2D, m_ColorTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, g_FboWidth, g_FboWidth, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

    //Create the target framebuffer, attach the the texture
	glGenFramebuffers(1, &m_TargetFbo);
	glBindFramebuffer(GL_FRAMEBUFFER, m_TargetFbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ColorTex, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


}

}

