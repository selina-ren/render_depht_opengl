#include "OpenGL_depth.hpp"

// Shader Names
char *vertexFileName = "color.vert";
char *fragmentFileName = "color.frag";

//camera parameters
float fov = 92.47;

//timer count
std::clock_t start;
int timer_count = 0;


// Vertex Array Objects Identifiers
GLuint vao;

// Vertex Attribute Locations
GLuint vertexLoc, colorLoc;

// vertex list
std::vector<glm::vec3> vertices_list;

// Uniform variable Locations
GLuint projMatrixLoc, viewMatrixLoc;

// Program and Shader Identifiers
GLuint p, v, f;
GLuint p_s, v_s, f_s;


// storage for Matrices
float projMatrix[16];
float viewMatrix[16];

// depth image
int dep_W, dep_H;
unsigned char* pixel_data;

int width, height;
int lfcam_num_width, lfcam_num_height;
float lfcam_fov;
float zFar;
float lf_slab_scale;
float slab_center[3];


//buffer 
GLuint vertexbuffer;
GLuint uvbuffer;

float obj_center[3];




// ----------------------------------------------------
// VECTOR STUFF
//

// res = a cross b;
void crossProduct(float *a, float *b, float *res) {

	res[0] = a[1] * b[2] - b[1] * a[2];
	res[1] = a[2] * b[0] - b[2] * a[0];
	res[2] = a[0] * b[1] - b[0] * a[1];
}

// Normalize a vec3
void normalize(float *a) {

	float mag = sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]);

	a[0] /= mag;
	a[1] /= mag;
	a[2] /= mag;
}

// ----------------------------------------------------
// MATRIX STUFF
//

// sets the square matrix mat to the identity matrix,
// size refers to the number of rows (or columns)
void setIdentityMatrix(float *mat, int size) {

	// fill matrix with 0s
	for (int i = 0; i < size * size; ++i)
		mat[i] = 0.0f;

	// fill diagonal with 1s
	for (int i = 0; i < size; ++i)
		mat[i + i * size] = 1.0f;
}

//
// a = a * b;
//
void multMatrix(float *a, float *b) {

	float res[16];

	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			res[j * 4 + i] = 0.0f;
			for (int k = 0; k < 4; ++k) {
				res[j * 4 + i] += a[k * 4 + i] * b[j * 4 + k];
			}
		}
	}
	memcpy(a, res, 16 * sizeof(float));

}

// Defines a transformation matrix mat with a translation
void setTranslationMatrix(float *mat, float x, float y, float z) {

	setIdentityMatrix(mat, 4);
	mat[12] = x;
	mat[13] = y;
	mat[14] = z;
}

// ----------------------------------------------------
// Projection Matrix
//

void buildProjectionMatrix(float fov, float ratio, float nearP, float farP) {

	float f = 1.0f / tan(fov * (M_PI / 360.0));

	setIdentityMatrix(projMatrix, 4);

	projMatrix[0] = f / ratio;
	projMatrix[1 * 4 + 1] = f;
	projMatrix[2 * 4 + 2] = (farP + nearP) / (nearP - farP);
	projMatrix[3 * 4 + 2] = (2.0f * farP * nearP) / (nearP - farP);
	projMatrix[2 * 4 + 3] = -1.0f;
	projMatrix[3 * 4 + 3] = 0.0f;
}






void changeSize(int w, int h) 
{

	float ratio;
	// Prevent a divide by zero, when window is too short
	// (you cant make a window of zero width).
	if (h == 0)
		h = 1;

	// Set the viewport to be the entire window
	glViewport(0, 0, w, h);

	ratio = (1.0f * w) / h;
	buildProjectionMatrix(92.794, ratio, 1.0f, 1000.0f);


	dep_W = w;
	dep_H = h;

	pixel_data = new unsigned char[4 * dep_W*dep_H];
}



int printOglError(char *file, int line)
{
	//
	// Returns 1 if an OpenGL error occurred, 0 otherwise.
	//
	GLenum glErr;
	int    retCode = 0;

	glErr = glGetError();
	while (glErr != GL_NO_ERROR)
	{
		printf("glError in file %s @ line %d: %s\n", file, line, gluErrorString(glErr));
		retCode = 1;
		glErr = glGetError();
	}
	return retCode;
}

void printShaderInfoLog(GLuint obj)
{
	int infologLength = 0;
	int charsWritten = 0;
	char *infoLog;

	glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &infologLength);

	if (infologLength > 0)
	{
		infoLog = (char *)malloc(infologLength);
		glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog);
		printf("%s\n", infoLog);
		free(infoLog);
	}
}

void printProgramInfoLog(GLuint obj)
{
	int infologLength = 0;
	int charsWritten = 0;
	char *infoLog;

	glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &infologLength);

	if (infologLength > 0)
	{
		infoLog = (char *)malloc(infologLength);
		glGetProgramInfoLog(obj, infologLength, &charsWritten, infoLog);
		printf("%s\n", infoLog);
		free(infoLog);
	}
}

GLuint setupShaders() {

	char *vs = NULL, *fs = NULL, *fs2 = NULL;

	//unsigned int p_temp, v_temp, f_temp;

	v_s = glCreateShader(GL_VERTEX_SHADER);
	f_s = glCreateShader(GL_FRAGMENT_SHADER);

	char *vertexFileName;
	char *fragmentFileName;
	vertexFileName = "color.vert";
	fragmentFileName = "color.frag";

	vs = textFileRead(vertexFileName);
	fs = textFileRead(fragmentFileName);

	const char * vv = vs;
	const char * ff = fs;

	glShaderSource(v_s, 1, &vv, NULL);
	glShaderSource(f_s, 1, &ff, NULL);

	free(vs); free(fs);

	glCompileShader(v_s);
	glCompileShader(f_s);

	printShaderInfoLog(v_s);
	printShaderInfoLog(f_s);

	p_s = glCreateProgram();
	glAttachShader(p_s, v_s);
	glAttachShader(p_s, f_s);

	glBindFragDataLocation(p_s, 0, "outputF");
	glLinkProgram(p_s);
	printProgramInfoLog(p_s);

	vertexLoc = glGetAttribLocation(p_s, "position");
	//colorLoc = glGetAttribLocation(p, "uv");

	projMatrixLoc = glGetUniformLocation(p_s, "projMatrix");
	viewMatrixLoc = glGetUniformLocation(p_s, "viewMatrix");
	p = p_s;

	return(p_s);
}

void setupBuffers() {


	glGenVertexArrays(1, &vao);

	glBindVertexArray(vao);
	// Generate two slots for the vertex and color buffers

	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	
	glBufferData(GL_ARRAY_BUFFER, vertices_list.size() * sizeof(glm::vec3), &vertices_list[0], GL_STATIC_DRAW);

	/*
	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);
	*/


	

}


void setUniforms() {

	glUniformMatrix4fv(projMatrixLoc, 1, false, projMatrix);
	glUniformMatrix4fv(viewMatrixLoc, 1, false, viewMatrix);
}


void renderScene(void) {

	////////////////// timer //////////////////
	
	
	if (timer_count == 0)
	{
		start = std::clock();
	}
	////////////////// timer //////////////////
	int nview_res = dep_H;

	changeSize(nview_res, nview_res);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//setCamera(0, 0, 3, 0, 0, 0);
	//setCamera(obj_center[0], obj_center[1], obj_center[2] + 1, obj_center[0], obj_center[1], obj_center[2]);

	glUseProgram(p_s);
	setUniforms();


	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glVertexAttribPointer(
		0,                  // attribute
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
		);

	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, vertices_list.size());
	glutSwapBuffers();

	glFlush();
	//int err = clEnqueueAcquireGLObjects(commandQueue, 1, &depth_map, 0, 0, 0);
	glReadPixels(0, 0, nview_res, nview_res, GL_RGBA, GL_UNSIGNED_BYTE, pixel_data);

	FILE *f = fopen("out.ppm", "wb");
	fprintf(f, "P6\n%i %i 255\n", nview_res, nview_res);
	for (int y = 0; y<nview_res; y++)
		for (int x = 0; x<nview_res; x++)
		{
			fputc(pixel_data[x * nview_res * 4 + y * 4 + 0], f);   // 0 .. 255
			fputc(pixel_data[x * nview_res * 4 + y * 4 + 1], f); // 0 .. 255
			fputc(pixel_data[x * nview_res * 4 + y * 4 + 2], f);  // 0 .. 255
		}
	fclose(f);






	////////////////// timer //////////////////
	

	timer_count++;

	if (timer_count == 1000)
	{
		double duration;
		duration = (std::clock() - start);
		std::cout << "computation time for 1000 frame: " << duration << '\n';
		timer_count = 0;
	}

	////////////////// timer //////////////////
	
}

// ----------------------------------------------------
// View Matrix
//
// note: it assumes the camera is not tilted,
// i.e. a vertical up vector (remmeber gluLookAt?)
//

void setCamera(float* cam_pos, float* cam_dir, float* up_dir) 
{

	float dir[3], right[3], up[3];

	/*
	up[0] = cam_up[0];
	up[1] = cam_up[1];
	up[2] = cam_up[2];
	*/

	up[0] = up_dir[0];
	up[1] = up_dir[1];
	up[2] = up_dir[2];

	//up[0] = 0.0f;   up[1] = 1.0f;   up[2] = 0.0f;

	dir[0] = cam_dir[0];
	dir[1] = cam_dir[1];
	dir[2] = cam_dir[2];
	normalize(dir);

	crossProduct(dir, up, right);
	normalize(right);

	crossProduct(right, dir, up);
	normalize(up);

	float aux[16];
	viewMatrix[0] = right[0];
	viewMatrix[4] = right[1];
	viewMatrix[8] = right[2];
	viewMatrix[12] = 0.0f;

	viewMatrix[1] = up[0];
	viewMatrix[5] = up[1];
	viewMatrix[9] = up[2];
	viewMatrix[13] = 0.0f;

	viewMatrix[2] = -dir[0];
	viewMatrix[6] = -dir[1];
	viewMatrix[10] = -dir[2];
	viewMatrix[14] = 0.0f;

	viewMatrix[3] = 0.0f;
	viewMatrix[7] = 0.0f;
	viewMatrix[11] = 0.0f;
	viewMatrix[15] = 1.0f;

	setTranslationMatrix(aux, -cam_pos[0], -cam_pos[1], -cam_pos[2]);

	multMatrix(viewMatrix, aux);
}

void load_geo(string objName, std::vector<glm::vec3>* vertices)
{
	bool res = loadOBJ_wo_tex_norm(objName, vertices_list);
	
	*vertices = vertices_list;
}


void render_depth(int width, int height) {

	

	changeSize(width, height);

	float cam_pos[3];
	float cam_dir[3];
	float cam_up[3];


	for (int face_num = 1; face_num <= 6; face_num++)
	{
		for (int sub_cam_row = 1; sub_cam_row <= lfcam_num_height; sub_cam_row++)
		{
			
			for (int sub_cam_col = 1; sub_cam_col <= lfcam_num_width; sub_cam_col++)
			{
				trans_cam(cam_pos, cam_dir,cam_up,face_num,sub_cam_row,sub_cam_col);
				render_sub_depth(cam_pos, cam_dir, cam_up);

				glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixel_data);
				


				///// output image /////
				char file_name[100];
				char file_num[10];
				strcpy(file_name, "H://");
				_itoa(face_num, file_num, 10);
				strcat(file_name, file_num);
				strcat(file_name, "_");

				_itoa(sub_cam_row, file_num, 10);
				strcat(file_name, file_num);
				strcat(file_name, "_");

				_itoa(sub_cam_col, file_num, 10);
				strcat(file_name, file_num);
				strcat(file_name, ".bmp");

				Mat image(Size(width, height), CV_8UC4, pixel_data, Mat::AUTO_STEP);
				
				flip(image, image, 0);

 				imwrite(file_name, image);
				/*
				FILE *f = fopen(file_name, "wb");
				fprintf(f, "P6\n%i %i 255\n", width, height);
				for (int y = 0; y<width; y++)
					for (int x = 0; x<height; x++)
					{
						fputc(pixel_data[y * height * 4 + x * 4 + 0], f);   // 0 .. 255
						fputc(pixel_data[y * height * 4 + x * 4 + 1], f); // 0 .. 255
						fputc(pixel_data[y * height * 4 + x * 4 + 2], f);  // 0 .. 255
					}
				fclose(f);*/
				///// output image /////
			}
		}
	}	

}

void setsubCamera(float* pos, float* look_dir, float* up_dir)
{

	float dir[3], right[3], up[3];

	/*
	up[0] = cam_up[0];
	up[1] = cam_up[1];
	up[2] = cam_up[2];
	*/

	up[0] = up_dir[0];
	up[1] = up_dir[1];
	up[2] = up_dir[2];

	//up[0] = 0.0f;   up[1] = 1.0f;   up[2] = 0.0f;

	dir[0] = look_dir[0];
	dir[1] = look_dir[1];
	dir[2] = look_dir[2];
	normalize(dir);

	crossProduct(dir, up, right);
	normalize(right);

	crossProduct(right, dir, up);
	normalize(up);

	float aux[16];
	viewMatrix[0] = right[0];
	viewMatrix[4] = right[1];
	viewMatrix[8] = right[2];
	viewMatrix[12] = 0.0f;

	viewMatrix[1] = up[0];
	viewMatrix[5] = up[1];
	viewMatrix[9] = up[2];
	viewMatrix[13] = 0.0f;

	viewMatrix[2] = -dir[0];
	viewMatrix[6] = -dir[1];
	viewMatrix[10] = -dir[2];
	viewMatrix[14] = 0.0f;

	viewMatrix[3] = 0.0f;
	viewMatrix[7] = 0.0f;
	viewMatrix[11] = 0.0f;
	viewMatrix[15] = 1.0f;

	setTranslationMatrix(aux, -pos[0], -pos[1], -pos[2]);

	multMatrix(viewMatrix, aux);

}

void render_sub_depth(float* cam_pos,float* cam_dir, float* cam_up)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//setCamera(0, 0, 3, 0, 0, 0);
	//setCamera(obj_center[0], obj_center[1], obj_center[2] + 1, obj_center[0], obj_center[1], obj_center[2]);
	setsubCamera( cam_pos, cam_dir, cam_up);
	glUseProgram(p_s);
	setUniforms();


	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glVertexAttribPointer(
		0,                  // attribute
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
		);

	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, vertices_list.size());
	//glutSwapBuffers();

	glFlush();


}

void processNormalKeys(unsigned char key, int x, int y) {

	if (key == 27) {
		glDeleteProgram(p);
		glDeleteShader(v);
		glDeleteShader(f);
		exit(0);
	}
}

void trans_cam(float* cam_pos, float* cam_dir, float* cam_up, int face_num, int sub_cam_row, int sub_cam_col)
{
	float cam_right[3];
	
	if (face_num == 1)
	{
		cam_dir[0] = -1;
		cam_dir[1] = 0;
		cam_dir[2] = 0;

		cam_up[0] = 0;
		cam_up[1] = 1;
		cam_up[2] = 0;

	}
	else if (face_num == 2)
	{
		cam_dir[0] = 0;
		cam_dir[1] = 0;
		cam_dir[2] = 1;

		cam_up[0] = 0;
		cam_up[1] = 1;
		cam_up[2] = 0;
	}
	else if (face_num == 3)
	{
		cam_dir[0] = 1;
		cam_dir[1] = 0;
		cam_dir[2] = 0;

		cam_up[0] = 0;
		cam_up[1] = 1;
		cam_up[2] = 0;
	}
	else if (face_num == 4)
	{
		cam_dir[0] = 0;
		cam_dir[1] = 0;
		cam_dir[2] = -1;

		cam_up[0] = 0;
		cam_up[1] = 1;
		cam_up[2] = 0;
	}
	else if (face_num == 5)
	{
		cam_dir[0] = 0;
		cam_dir[1] = 1;
		cam_dir[2] = 0;

		cam_up[0] = 1;
		cam_up[1] = 0;
		cam_up[2] = 0;
	}
	else if (face_num == 6)
	{
		cam_dir[0] = 0;
		cam_dir[1] = -1;
		cam_dir[2] = 0;

		cam_up[0] = -1;
		cam_up[1] = 0;
		cam_up[2] = 0;
	}

	crossProduct(cam_dir, cam_up, cam_right);

	float lf_slab_st[3];

	lf_slab_st[0] = slab_center[0] + cam_dir[0] * lf_slab_scale + cam_up[0] * lf_slab_scale * 3.0 - cam_right[0] * lf_slab_scale * 3.0;
	lf_slab_st[1] = slab_center[1] + cam_dir[1] * lf_slab_scale + cam_up[1] * lf_slab_scale * 3.0 - cam_right[1] * lf_slab_scale * 3.0;
	lf_slab_st[2] = slab_center[2] + cam_dir[2] * lf_slab_scale + cam_up[2] * lf_slab_scale * 3.0 - cam_right[2] * lf_slab_scale * 3.0;

	cam_pos[0] = lf_slab_st[0] + cam_right[0] * lf_slab_scale * (float)(sub_cam_col - 1.0) * 6.0 / 9.0 - cam_up[0] * lf_slab_scale * (float)(sub_cam_row - 1.0) * 6.0 / 9.0;
	cam_pos[1] = lf_slab_st[1] + cam_right[1] * lf_slab_scale * (float)(sub_cam_col - 1.0) * 6.0 / 9.0 - cam_up[1] * lf_slab_scale * (float)(sub_cam_row - 1.0) * 6.0 / 9.0;
	cam_pos[2] = lf_slab_st[2] + cam_right[2] * lf_slab_scale * (float)(sub_cam_col - 1.0) * 6.0 / 9.0 - cam_up[2] * lf_slab_scale * (float)(sub_cam_row - 1.0) * 6.0 / 9.0;

	
}

int loadConfigfile(std::string configName, std::string *objName, int* width, int* height)
{
	std::ifstream in;
	std::string str;



	in.open(configName);


	string kk;

	while (!in.eof())
	{
		while (getline(in, str))
		{
			std::string::size_type begin = str.find_first_not_of(" \f\t\v");
			//Skips blank lines
			if (begin == std::string::npos)
				continue;
			std::string firstWord;
			try {
				firstWord = str.substr(0, str.find(" "));
			}
			catch (std::exception& e) {
				firstWord = str.erase(str.find_first_of(" "), str.find_first_not_of(" "));
			}
			std::transform(firstWord.begin(), firstWord.end(), firstWord.begin(), ::toupper);
			if (firstWord == "OBJ_FILE")
				*objName = str.substr(str.find(" ") + 1, str.length());
			if (firstWord == "IMAGE_WIDTH")
				*width = stoi(str.substr(str.find(" ") + 1, str.length()));
			if (firstWord == "IMAGE_HEIGHT")
				*height = stoi(str.substr(str.find(" ") + 1, str.length()));
			if (firstWord == "HORIZONTAL_CAMERA_NUMBERS")
				lfcam_num_height = stoi(str.substr(str.find(" ") + 1, str.length()));
			if (firstWord == "VERTICAL_CAMERA_NUMBERS")
				lfcam_num_width = stoi(str.substr(str.find(" ") + 1, str.length()));
			if (firstWord == "LFCAM_FOV")
				lfcam_fov = stof(str.substr(str.find(" ") + 1, str.length()));
			if (firstWord == "SLAB_CENTER_X")
				slab_center[0] = stof(str.substr(str.find(" ") + 1, str.length()));
			if (firstWord == "SLAB_CENTER_Y")
				slab_center[1] = stof(str.substr(str.find(" ") + 1, str.length()));
			if (firstWord == "SLAB_CENTER_Z")
				slab_center[2] = stof(str.substr(str.find(" ") + 1, str.length()));
			if (firstWord == "Z_FAR")
				zFar = stof(str.substr(str.find(" ") + 1, str.length()));
			if (firstWord == "LIGHT_FIELD_SLAB_SCALE")
				lf_slab_scale = stof(str.substr(str.find(" ") + 1, str.length()));

		}


	}

	pixel_data = new unsigned char[*width * *height * 4];

	return 0;
}


