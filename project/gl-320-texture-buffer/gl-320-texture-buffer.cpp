#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <string>

using namespace std;
#include <shader.h>

#define BUFFER_OFFSET(offset) ((GLvoid*) (NULL + offset))

struct Window {
	char *title;
	int width, height;
	int posX, posY;
	Window(char *t, const int &w, const int &h, const int &px, const int &py) 
		  :title(t), width(w), height(h), posX(px), posY(py){}
};

const char* vert_shader_code("texture-buffer.vert");
const char* frag_shader_code("texture-buffer.frag");
Window window("texture-buffer", 720, 640, 300, 0);

GLsizei const VertexCount(6);
GLsizeiptr const VertexSize = VertexCount * sizeof(glm::vec2);
glm::vec2 const VertexData[VertexCount] =  {
		glm::vec2(-1.0f,-1.0f),
		glm::vec2( 1.0f,-1.0f),
		glm::vec2( 1.0f, 1.0f),
		glm::vec2( 1.0f, 1.0f),
		glm::vec2(-1.0f, 1.0f),
		glm::vec2(-1.0f,-1.0f)
};

GLuint vao = 0;
GLuint program = 0;
GLuint vbo, diffuseTexture, displacementTexture;
GLint mvp_matrix_loc;
GLint diffuse_loc;
GLint displacement_loc;
Shader texture3dShader("texture-3d");
namespace Buffer
{
		enum type
		{
			VERTEX,
			DISPLACEMENT,
			DIFFUSE,
			MAX
		};
}//namespace buffer
vector<GLuint> buffer(Buffer::MAX);

void load_shader()
{
	texture3dShader.init();
	texture3dShader.attach(GL_VERTEX_SHADER, vert_shader_code);
	texture3dShader.attach(GL_FRAGMENT_SHADER, frag_shader_code);
	texture3dShader.link();
	texture3dShader.use();
	program = texture3dShader.program;
	mvp_matrix_loc = glGetUniformLocation(program, "mvp_matrix");
	diffuse_loc = glGetUniformLocation(program, "diffuse");
	displacement_loc =glGetUniformLocation(program, "Displacement");
}

void init_buffer()
{
	glGenBuffers(Buffer::MAX, &buffer[0]); 

	glBindBuffer(GL_ARRAY_BUFFER, buffer[Buffer::VERTEX]);
	glBufferData(GL_ARRAY_BUFFER, VertexSize, VertexData, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glm::vec4 Position[5] = {
			glm::vec4( 0.1f, 0.3f,-1.0f, 1.0f), 
			glm::vec4(-0.5f, 0.0f,-0.5f, 1.0f),
			glm::vec4(-0.2f,-0.2f, 0.0f, 1.0f),
			glm::vec4( 0.3f, 0.2f, 0.5f, 1.0f),
			glm::vec4( 0.1f,-0.3f, 1.0f, 1.0f)
	};
	glBindBuffer(GL_ARRAY_BUFFER, buffer[Buffer::DISPLACEMENT]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Position), Position, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glm::u8vec4 Diffuse[5] = 
	{
			glm::u8vec4(255,   0,   0, 255),
			glm::u8vec4(255, 127,   0, 255),
			glm::u8vec4(255, 255,   0, 255),
			glm::u8vec4(  0, 255,   0, 255),
			glm::u8vec4(  0,   0, 255, 255)
	};	
	GLint MaxTextureBufferSize(0);
	glGetIntegerv(GL_MAX_TEXTURE_BUFFER_SIZE, &MaxTextureBufferSize);

	glBindBuffer(GL_TEXTURE_BUFFER, buffer[Buffer::DIFFUSE]);
	glBufferData(GL_TEXTURE_BUFFER, 500000, NULL, GL_STATIC_DRAW);
		//glBufferData(GL_TEXTURE_BUFFER, sizeof(Diffuse), Diffuse, GL_STATIC_DRAW);
	glBufferSubData(GL_TEXTURE_BUFFER, 0, sizeof(Diffuse), Diffuse);
	glBindBuffer(GL_TEXTURE_BUFFER, 0);


}

void init_texture()
{
		glGenTextures(1, &displacementTexture);
		glBindTexture(GL_TEXTURE_BUFFER, displacementTexture);
		glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, buffer[Buffer::DISPLACEMENT]);
		glBindTexture(GL_TEXTURE_BUFFER, 0);

		glGenTextures(1, &diffuseTexture);
		glBindTexture(GL_TEXTURE_BUFFER, diffuseTexture);
		glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA8, buffer[Buffer::DIFFUSE]);
		glBindTexture(GL_TEXTURE_BUFFER, 0);	
}

void init_vertexArray()
{
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2) * 2, nullptr);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);
}

void init()
{
	glEnable(GL_DEPTH_TEST);
	load_shader();
	init_buffer();
	init_texture();
	init_vertexArray();
}

void end()
{
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
	glDeleteProgram(program);
}

void render()
{
  	glm::mat4 Projection = glm::perspective(glm::pi<float>() * 0.25f, 4.0f / 3.0f, 0.1f, 100.0f);
	glm::mat4 Model = glm::mat4(1.0f);
	glm::mat4 MVP = Projection  * Model;
    glViewport(0, 0, window.width, window.height);
		// Clear color buffer with black
    float Depth(1.0f);
    glClearBufferfv(GL_DEPTH, 0, &Depth);
    glClearBufferfv(GL_COLOR, 0, &glm::vec4(1.0f)[0]);

    glUseProgram(program);
	glUniformMatrix4fv(mvp_matrix_loc, 1, GL_FALSE, &MVP[0][0]);
	glUniform1i(displacement_loc, 0);
	glUniform1i(diffuse_loc, 1);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_BUFFER, displacementTexture);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_BUFFER, diffuseTexture);

	glBindVertexArray(vao);
	glDrawArraysInstanced(GL_TRIANGLES, 0, VertexCount, 5);
};

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowPosition(window.posX, window.posY);
	glutInitWindowSize(window.width, window.height);
	glutCreateWindow(window.title);
	GLenum err = glewInit();
	if (err = GLEW_OK)
		cerr << glewGetErrorString(err) << endl;
	init();
	glutDisplayFunc(render);
	glutMainLoop();
}
