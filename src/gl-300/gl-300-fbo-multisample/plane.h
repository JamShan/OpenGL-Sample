#ifndef WINDOW_H
#define WINDOW_H

#include "ogl/App.h"
#include "ogl/Shader.h"
#include "ogl/Utility.h"

#include <glm/glm.hpp>

namespace byhj
{

class Plane
{
public:
	Plane() = default;
	~Plane() = default;

public:

	void Init()	   ;
	void Render()  ;
	void Shutdown();

private:

	void init_buffer();
	void init_shader();
	void init_vertexArray();
	void init_texture();
	void init_fbo();


private:
	struct Vertex
	{
		Vertex(glm::vec2 const & pos, glm::vec2 const &tex)
		:Position(pos), TexCoord(tex) {}

		glm::vec2 Position;
		glm::vec2 TexCoord;
	};

	struct UniformLoc
	{
		GLuint tex_loc;
	}m_Uniform;

	GLuint m_RenderFbo = ogl::VALUE;
	GLuint m_TargetFbo = ogl::VALUE;
	GLuint m_ColorTex  = ogl::VALUE;
	GLuint m_ColorRbo  = ogl::VALUE;
	GLuint m_ImageTex  = ogl::VALUE;
	GLuint m_Vbo       = ogl::VALUE;
	GLuint m_Vao       = ogl::VALUE;
	GLuint m_Program   = ogl::VALUE;

	ogl::Shader m_PlaneShader = { "Plane Shader" };
};

}

#endif