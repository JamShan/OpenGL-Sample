#include "Plane.h"
#include <gli/gli.hpp>

namespace byhj
{

void Plane::Init()
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	init_shader();
	init_buffer();
	init_vertexArray();
	init_texture();
}

void Plane::Render()
{

	glUseProgram(m_Program);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_ImageTex);
	glUniform1i(m_Uniform.tex_loc, 0);

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
		Vertex(glm::vec2(-1.0f, -1.0f), glm::vec2(0.0f, 0.0f)),
		Vertex(glm::vec2( 1.0f, -1.0f), glm::vec2(1.0f, 0.0f)),
		Vertex(glm::vec2( 1.0f,  1.0f), glm::vec2(1.0f, 1.0f)),
		Vertex(glm::vec2( 1.0f,  1.0f), glm::vec2(1.0f, 1.0f)),
		Vertex(glm::vec2(-1.0f,  1.0f), glm::vec2(0.0f, 1.0f)),
		Vertex(glm::vec2(-1.0f, -1.0f), glm::vec2(0.0f, 0.0f))
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

	gli::texture2D Texture( gli::load_dds("../../../media/textures/kueken2-bgra8.dds") );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	//mipmap
	for (std::size_t Level = 0; Level < Texture.levels(); ++Level) {
		glTexImage2D(GL_TEXTURE_2D,
			GLint(Level),
			GL_RGBA8,
			GLsizei(Texture[Level].dimensions().x),
			GLsizei(Texture[Level].dimensions().y),
			0,
			GL_BGRA, GL_UNSIGNED_BYTE,
			Texture[Level].data());
	}
	glBindTexture(GL_TEXTURE_2D, 0);
}


}

