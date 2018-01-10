#define _CRT_SECURE_NO_DEPRECATE
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include <GL/glew.h>
#include <GL/glut.h>



#include "OpenGL_depth.hpp"
#include "objloader.hpp"

#include <glm.hpp>

using namespace std;







int main(int argc, char **argv) {

	string objName;

	int win_width;
	int win_height;

	loadConfigfile("config.txt", &objName, &win_width, &win_height);
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(win_width, win_height);
	glutCreateWindow("Obj_Depth");

	glutDisplayFunc(renderScene);
	glutIdleFunc(renderScene);
	glutReshapeFunc(changeSize);
	//glutKeyboardFunc(processNormalKeys);

	glewInit();
	if (glewIsSupported("GL_VERSION_3_3"))
		printf("Ready for OpenGL 3.3\n");
	else {
		printf("OpenGL 3.3 not supported\n");
		exit(1);
	}

	std::vector<glm::vec3> vertices;

	std::vector<glm::vec2> uvs;
	//std::vector<glm::vec3> normals; // Won't be used at the moment.
	//bool res = loadOBJ_wo_tex_norm(objName, vertices);

	//bool res = loadOBJ_wo_tex("obj_files//newbedrm.obj", vertices,  normals);
	//bool res = loadOBJ("obj_files//cube.obj", vertices, uvs, normals);

	float center[3];
	load_geo(objName, &vertices);

	glEnable(GL_DEPTH_TEST);
	glClearColor(1.0, 1.0, 1.0, 1.0);

	GLuint p = setupShaders();
	setupBuffers();

	//glutMainLoop();

	render_depth(win_width, win_height);

	return(0);
}