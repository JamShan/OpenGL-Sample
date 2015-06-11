#include <GL/glew.h>
#include <GL/freeglut.h>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <gli/gli.hpp>
#include <iostream>
#include <string>

using namespace std;
#include <caps.hpp>

#define BUFFER_OFFSET(offset) ((GLvoid*) (NULL + offset))

const int posX(300), posY(0);
const int Width(720), Height(640);

const char *Title = "gl-320-caps";

caps Caps;

void init()
{ 
	bool Validated = true;
	Validated = Validated && Caps.Limits.MAX_VERTEX_UNIFORM_BLOCKS >= 12;
		Validated = Validated && Caps.Limits.MAX_GEOMETRY_UNIFORM_BLOCKS >= 12;
		Validated = Validated && Caps.Limits.MAX_FRAGMENT_UNIFORM_BLOCKS >= 12;

		Validated = Validated && Caps.Limits.MAX_VERTEX_UNIFORM_COMPONENTS >= 1024;
		Validated = Validated && Caps.Limits.MAX_GEOMETRY_UNIFORM_COMPONENTS >= 1024;
		Validated = Validated && Caps.Limits.MAX_FRAGMENT_UNIFORM_COMPONENTS >= 1024;

		Validated = Validated && Caps.Limits.MAX_COMBINED_UNIFORM_BLOCKS >= 36;
		Validated = Validated && Caps.Limits.MAX_UNIFORM_BUFFER_BINDINGS >= 36;
		Validated = Validated && Caps.Limits.MAX_UNIFORM_BLOCK_SIZE >= 16384;

		Validated = Validated && Caps.Limits.MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS == ((Caps.Limits.MAX_VERTEX_UNIFORM_BLOCKS * Caps.Limits.MAX_UNIFORM_BLOCK_SIZE / 4) + Caps.Limits.MAX_VERTEX_UNIFORM_COMPONENTS);
		Validated = Validated && Caps.Limits.MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENTS == ((Caps.Limits.MAX_GEOMETRY_UNIFORM_BLOCKS * Caps.Limits.MAX_UNIFORM_BLOCK_SIZE / 4) + Caps.Limits.MAX_GEOMETRY_UNIFORM_COMPONENTS);
		Validated = Validated && Caps.Limits.MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS == ((Caps.Limits.MAX_FRAGMENT_UNIFORM_BLOCKS * Caps.Limits.MAX_UNIFORM_BLOCK_SIZE / 4) + Caps.Limits.MAX_FRAGMENT_UNIFORM_COMPONENTS);
}



void render()
{
	glClearBufferfv(GL_COLOR, 0, &glm::vec4(1.0f, 0.5f, 0.0f, 1.0f)[0]);
	glViewport(0, 0, Width, Height);
};


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
	return 0;
}