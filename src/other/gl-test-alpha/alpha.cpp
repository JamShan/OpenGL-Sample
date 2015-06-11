#include <gl/glew.h>
#include <gl/freeglut.h>
#include <iostream>

using namespace std;

#include <shader.h>

enum VAO_IDs { Triangles, NumVAOs };  //VAO����
enum Buffer_IDs { ArrayBuffer, NumBuffers }; //VBO����
enum Attrib_IDs { vPosition = 0 };  //��������

GLuint VAOs[NumVAOs];   //������������
GLuint Buffers[NumBuffers];  //�������
const GLuint NumVertices = 6; //������
Shader triangleShader("triangle shader");  //shader����
const int Width = 720, Height = 640; //���ڿ��

void load_shaders()
{
  GLint compile_status = GL_FALSE, link_status = GL_FALSE;
  GLuint vs = glCreateShader(GL_VERTEX_SHADER);

  const char *vs_source =  textFileRead("triangle.vert");

   glShaderSource(vs, 1, &vs_source, NULL);
   glCompileShader(vs);
   glGetShaderiv(vs, GL_COMPILE_STATUS, &compile_status);
   if (!compile_status) {
	   cerr << "Error in vertex shader" << endl;
   }
   GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);

  
     const char *fs_source = textFileRead("triangle.frag");

	 glShaderSource(fs, 1, &fs_source, NULL);
	 glCompileShader(fs);
	 glGetShaderiv(fs, GL_COMPILE_STATUS, &compile_status);
	 if (!compile_status) {
		 cerr << "fragment shader Error" << endl;
	 }
	 program = glCreateProgram();
	 glAttachShader(program, vs);
	 glAttachShader(program, fs);
	 glLinkProgram(program);
	 glGetProgramiv(program, GL_LINK_STATUS, &link_status);
	 if (!link_status) {
		 cerr << "Link shader Error" << endl;
	 }

	 const char *attribute_name = "position";
	 positionLocation = glGetAttribLocation(program, attribute_name);
	 if (positionLocation == -1) {
		 cout << "cannot bind attribute " << endl;
	 }
}
}

void init(void)
{
	load_shaders();

	glClearColor(0.0, 0.0, 0.0, 0.0); //������ɫ����Ϊ��ɫ

	//glGenVertexArrays(NumVAOs, VAOs);  //����VAO����״̬
	//glBindVertexArray(VAOs[Triangles]);

	GLfloat vertices[NumVertices][2] = {  //��������
		{ -0.90f, -0.90f }, // Triangle 1
		{ 0.85f, -0.90f },
	    { -0.90f, 0.85f },

		{ 0.90f, -0.85f }, // Triangle 2
		{ 0.90f, 0.90f },
		{ -0.85f, 0.90f }
		};

	glGenBuffers(NumBuffers, Buffers); //����VBO�����������ݴ���
	glBindBuffer(GL_ARRAY_BUFFER, Buffers[ArrayBuffer]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	//ָ���������ԣ�shader�����ݽӿ�
    glVertexAttribPointer(vPosition, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(vPosition);

}

void reshape(int w, int h)
{
	glViewport(0, 0, w, h); //�ӿڱ任

}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT); //�����ɫ����Ϊǰ������ֵ

	//glBindVertexArray(VAOs[0]);  //ʹ�����VAO��������״̬����
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);  //����������

	glFlush();  //ǿ��ִ�����ˢ��
	//ʹ��glFinish�ŵȴ�ȫ���������
}


int main(int argc,char ** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA);//The display mode is necessary
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(Width, Height);
	//glutInitContextVersion(4, 3);
	//glutInitContextProfile(GLUT_CORE_PROFILE);
	glutCreateWindow("Triangle");
	GLenum res = glewInit();  //glewInit() use first in order using other function
	if (res != GLEW_OK) 
		cout << "Init the glew Error" << glewGetErrorString(res) << endl;
	init(); //��ʼ����Դ
	glutDisplayFunc(display);  //����ѭ��������ʾ����
	glutReshapeFunc(reshape);  //�ӿڱ仯����
	glutMainLoop();    //ִ��ѭ��
	return 0;
}


