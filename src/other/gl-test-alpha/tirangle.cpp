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
	//glEnable( GL_DEPTH_TEST );         // 禁用深度缓存

 

//glDepthMask( GL_FALSE );              // 深度缓存为 只读

glDepthMask( GL_TRUE );               // 深度缓存为 读/写
	//源混合参数（A，A,A,A)， 目的混合参数（1-A, 1-A, 1-A, 1-A)；
	//最后绘制出来的颜色是（AR源, AG, ,AB ,AA）+（（1-A）R目）即是背景色
	//glGenBuffers(1, &VBO);  //生成VBO， 绑定，分配内存
	//glBindBuffer(GL_ARRAY_BUFFER, VBO);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(vectex), vectex, GL_STATIC_DRAW);
	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	glClearColor(0.3, 0.3, 0.0, 0.0);
}

void display(void)
{


//	glClearBufferfv(GL_COLOR, 0, color);
	glClear(GL_COLOR_BUFFER_BIT);  //使用设置好的背景色
//	glBindBuffer(GL_ARRAY_BUFFER, VBO); //绑定这个VBO用以绘制
	//glEnableVertexAttribArray(0); //顶点属性数组0，与shader交互
//	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glDrawArrays(GL_POINTS, 0, 1); //绘制函数
	glPointSize(40.0f);
	//glutSolidTeapot(1.0);
	//glDisableVertexAttribArray(0); //不需要时关闭顶点属性数组
	glutSwapBuffers(); //强制绘图，去掉会变白色
}

void reshape(int w, int h) {
	glViewport(0, 0, w, h);  //视口调整
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
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE); //显示模式，重要
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
	glutMainLoop(); //循环调用注册函数display
	return 0;
}