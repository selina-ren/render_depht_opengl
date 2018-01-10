// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <memory.h>
#include <math.h>
#include <iostream>
#include <cstdio>


#include <fstream>
#include <algorithm>
#include <string>

// Include GLEW
#include <GL/glew.h>
#include <GL/glut.h>

// Include GLM
#include <glm.hpp>

// Include Timer
#include <ctime>

#include "objloader.hpp"
#include "textfile.h"

// Include opencv for image output
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>


#define M_PI       3.14159265358979323846

using namespace std;
using namespace cv;


void changeSize(int w, int h);
GLuint setupShaders();
void printProgramInfoLog(GLuint obj);
void printShaderInfoLog(GLuint obj);
int printOglError(char *file, int line);
void setupBuffers();
void setUniforms();
void renderScene(void);
void setCamera(float* cam_pos, float* cam_dir, float* up_dir);
void load_geo(string objName, std::vector<glm::vec3>* vertices);
int loadConfigfile(std::string configName, std::string *objName, int* width, int* height);
void render_depth(int width, int height);



void trans_cam(float* cam_pos, float* cam_dir, float* cam_up, int face_num, int sub_cam_row, int sub_cam_col);
void render_sub_depth(float* cam_pos, float* cam_dir, float* cam_up);







