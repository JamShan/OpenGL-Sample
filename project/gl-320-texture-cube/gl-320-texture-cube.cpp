#include <GL/glew.h>
#include <GL/freeglut.h>
#include <shader.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <gli/gli.hpp>
#include <string>

using std::string;

struct Window {
	Window(int px, int py, int w, int h, string t):
		posX(px), posY(py), width(w), height(h), title(t){}
	int posX, posY;
	int width, height;
	string title;
};

const GLsizei VertexCount(6);
const GLsizeiptr VertexSize = VertexCount * sizeof(glm::vec2);
const glm::vec2 VertexData[VertexCount] =  {
		glm::vec2(-1.0f,-1.0f),
		glm::vec2( 1.0f,-1.0f),
		glm::vec2( 1.0f, 1.0f),
		glm::vec2( 1.0f, 1.0f),
		glm::vec2(-1.0f, 1.0f),
		glm::vec2(-1.0f,-1.0f)
};

Shader textureCubeShader("texture cube");
GLuint program, vbo, vao, texture;
GLuint mv_matrix_loc, mvp_matrix_loc;
GLuint environment_loc, camera_loc, vertex_loc;
Window window(300, 150, 720, 640, "Texture Cube");

void init_shader()
{
	textureCubeShader.init();
	textureCubeShader.attach(GL_VERTEX_SHADER, "texture-cube.vert");
	textureCubeShader.attach(GL_FRAGMENT_SHADER, "texture-cube.frag");
	textureCubeShader.link();
	textureCubeShader.use();
	program = textureCubeShader.program;
	mv_matrix_loc = glGetUniformLocation(program, "mv_matrix");
	mvp_matrix_loc = glGetUniformLocation(program, "mvp_matrix");
	environment_loc = glGetUniformLocation(program, "environment");
	camera_loc = glGetUniformLocation(program, "camera");
}

void init_buffer()
{
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, VertexSize, VertexData, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void init_vertexArray()
{
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(vertex_loc, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), NULL);
	glEnableVertexAttribArray(vertex_loc);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

}

void init_texture()
{
	gli::textureCube Texture(6, 1, gli::RGBA8_UNORM, 
		                    gli::textureCube::dimensions_type(2) );
	assert(!Texture.empty());
	Texture[0].clear<glm::u8vec4>(glm::u8vec4(255,   0,   0, 255));
	Texture[1].clear<glm::u8vec4>(glm::u8vec4(255, 128,   0, 255));
	Texture[2].clear<glm::u8vec4>(glm::u8vec4(255, 255,   0, 255));
	Texture[3].clear<glm::u8vec4>(glm::u8vec4(  0, 255,   0, 255));
	Texture[4].clear<glm::u8vec4>(glm::u8vec4(  0, 255, 255, 255));
	Texture[5].clear<glm::u8vec4>(glm::u8vec4(  0,   0, 255, 255));
	//生成纹理对象，设置纹理参数
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, GLint(Texture.levels() -1 ));
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameterfv(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BORDER_COLOR, &glm::vec4(0.0f)[0]);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_LOD, -1000.f);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LOD, 1000.f);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_LOD_BIAS, 0.0f);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_MODE, GL_NONE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	//设置纹理数据
	for (gli::textureCube::size_type Face = 0; Face < Texture.faces(); ++Face) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + GLenum(Face), 
			         0, GL_RGBA8,
				     static_cast<GLsizei>(Texture.dimensions().x),
			    	 static_cast<GLsizei>(Texture.dimensions().y),
				     0, GL_RGBA, GL_UNSIGNED_BYTE, Texture[Face].data()
					 );
	}
}
void init()
{
	init_shader();
	init_texture();
	init_buffer();
	init_vertexArray();
}

void render()
{
	glm::mat4 proj_matrix = glm::perspective(glm::pi<float>() * 0.25f, 2.0f / 3.0f, 0.1f, 1000.0f);
	glm::mat4 view_matrix = glm::lookAt(glm::vec3(0.0f, 0.0f, 5.0f),
		                    glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 model_matrix = glm::mat4(1.0f);
	glm::mat4 mv_matrix = view_matrix * model_matrix;
	glm::mat4 mvp_matrix = proj_matrix * mv_matrix;
	glClearBufferfv(GL_COLOR, 0, &glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)[0]);
	glUseProgram(program);
	glUniformMatrix4fv(mv_matrix_loc, 1, GL_FALSE, &mv_matrix[0][0]);
	glUniformMatrix4fv(mvp_matrix_loc, 1, GL_FALSE, &mvp_matrix[0][0]);
	glUniform1i(environment_loc, 0);
	glUniform3fv(camera_loc, 1, &glm::vec3(0.0f, 0.0f, 5.0f)[0]);
	glBindVertexArray(vao);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	glViewport(0, 0, 720, 640);
	glDrawArraysInstanced(GL_TRIANGLES, 0, VertexCount, 1);
}


int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowSize(window.width, window.height);
	glutInitWindowPosition(window.posX, window.posY);
	glutCreateWindow(window.title.c_str());
	glewInit();
	init();
	render();
	glutDisplayFunc(render);
	glutMainLoop();
}
