#ifndef RENDERSYSTEM_H
#define RENDERSYSTEM_H

#include "ogl/App.h"
#include "plane.h"

namespace byhj
{

namespace ogl
{

class RenderSystem : public ogl::Application
{
public:
	RenderSystem();
	~RenderSystem();

public:
	void v_InitInfo();
	void v_Init();
	void v_Render();
	void v_Shutdown();

private:

	Plane m_Plane;
};

}

}

#endif