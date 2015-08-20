#include <GL/glew.h>
#include "RenderSystem.h"

namespace byhj
{

namespace ogl
{


RenderSystem::RenderSystem()
{

}

RenderSystem::~RenderSystem()
{

}

void RenderSystem::v_InitInfo()
{
	windowInfo.title += "Fbo-MultiSample";
}

void RenderSystem::v_Init()
{
	const static GLfloat bgColor[4] = { 0.2f, 0.3f, 0.4f, 1.0f };
	glClearBufferfv(GL_COLOR, 0, bgColor);

	m_Plane.Init();
}

void RenderSystem::v_Render()
{
	m_Plane.Render();

}

void RenderSystem::v_Shutdown()
{
	m_Plane.Shutdown();
}


}

}