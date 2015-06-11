#include "common.h"
#include <GL/glew.h>
#include <gl/freeglut.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

using std::endl;
using std::cout;

GLuint VBO;

void init(void)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable( GL_POLYGON_SMOOTH );
	//glEnable( GL_DEPTH_TEST );         // ������Ȼ���

 

//glDepthMask( GL_FALSE );              // ��Ȼ���Ϊ ֻ��

glDepthMask( GL_TRUE );               // ��Ȼ���Ϊ ��/д
	//Դ��ϲ�����A��A,A,A)�� Ŀ�Ļ�ϲ�����1-A, 1-A, 1-A, 1-A)��
	//�����Ƴ�������ɫ�ǣ�ARԴ, AG, ,AB ,AA��+����1-A��RĿ�����Ǳ���ɫ
	//glGenBuffers(1, &VBO);  //����VBO�� �󶨣������ڴ�
	//glBindBuffer(GL_ARRAY_BUFFER, VBO);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(vectex), vectex, GL_STATIC_DRAW);
	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	glClearColor(0.3, 0.3, 0.0, 0.0);
}

void display(void)
{


//	glClearBufferfv(GL_COLOR, 0, color);
	glClear(GL_COLOR_BUFFER_BIT);  //ʹ�����úõı���ɫ
//	glBindBuffer(GL_ARRAY_BUFFER, VBO); //�����VBO���Ի���
	//glEnableVertexAttribArray(0); //������������0����shader����
//	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glDrawArrays(GL_POINTS, 0, 1); //���ƺ���
	glPointSize(40.0f);
	//glutSolidTeapot(1.0);
	//glDisableVertexAttribArray(0); //����Ҫʱ�رն�����������
	glutSwapBuffers(); //ǿ�ƻ�ͼ��ȥ������ɫ
}

void reshape(int w, int h) {
	glViewport(0, 0, w, h);  //�ӿڵ���
	/*
	glMatrixMode(GL_PROJECTION);
	gluPerspective(45.0, 1.0, 1.0, 1000.0f);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	gluLookAt(0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
	glLoadIdentity();
	*/
}

void idle(void )
{
	glutPostRedisplay();
}
int main(int argc, char ** argv)
{

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE); //��ʾģʽ����Ҫ
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(640, 480);
	glutCreateWindow("Tutorial 4 - Shaders");
	GLenum res = glewInit();
	if (res != GLEW_OK)
		cout << "Fail to initial the glew:" << glewGetString(res) << endl;
	init();
	initShader("triangle.vert", "triangle.frag");
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	//glutIdleFunc(idle);
	glutMainLoop(); //ѭ������ע�ắ��display
	return 0;
}