#include "opengl.h"
#include "standard.h"
#include "core/geometry.h"
#include "transform.h"

using namespace core;

int window_id;

vector<vec3f> vertices;
vector<vec3f> normals;
vector<vec2f> texcoords;
vector<unsigned int> indices;

int levels = 32;
int slices = 32;
float radius = 1.0;

GLuint program;

void init(string working_directory)
{
	glEnable(GL_DEPTH_TEST);

	// Load the shaders
	GLuint vertex = load_shader_file(working_directory + "res/example.vx", GL_VERTEX_SHADER);
	GLuint fragment = load_shader_file(working_directory + "res/example.ft", GL_FRAGMENT_SHADER);

	program = glCreateProgram();
	glAttachShader(program, vertex);
	glAttachShader(program, fragment);
	glLinkProgram(program);

	// Load the geometry of a sphere
	vertices.reserve(2 + (levels-1)*slices);
	normals.reserve(2 + (levels-1)*slices);
	texcoords.reserve(2 + (levels-1)*slices);
	vertices.push_back(vec3f(0.0, radius, 0.0));
	normals.push_back(vec3f(0.0, 1.0, 0.0));
	texcoords.push_back(vec2f(0.5, 0.0));
	for (int i = 1; i < levels; i++)
		for (int j = 0; j < slices; j++)
		{
			vec3f dir(sin(m_pi*(float)i/(float)levels)*cos(2.0*m_pi*(float)j/(float)(slices-1)),
					  cos(m_pi*(float)i/(float)levels),
					  sin(m_pi*(float)i/(float)levels)*sin(2.0*m_pi*(float)j/(float)(slices-1)));
			vertices.push_back(radius*dir);
			normals.push_back(dir);
			texcoords.push_back(vec2f((float)j/(float)(slices-1), (float)i/(float)levels));
		}

	vertices.push_back(vec3f(0.0, -radius, 0.0));
	normals.push_back(vec3f(0.0, -1.0, 0.0));
	texcoords.push_back(vec2f(0.5, 1.0));

	for (int i = 0; i < slices; i++)
	{
		indices.push_back(1 + (i+1)%slices);
		indices.push_back(0);
		indices.push_back(1 + i);
	}

	for (int i = 0; i < levels-2; i++)
		for (int j = 0; j < slices; j++)
		{
			indices.push_back(1 + i*slices + j);
			indices.push_back(1 + (i+1)*slices + j);
			indices.push_back(1 + i*slices + (j+1)%slices);

			indices.push_back(1 + (i+1)*slices + j);
			indices.push_back(1 + (i+1)*slices + (j+1)%slices);
			indices.push_back(1 + i*slices + (j+1)%slices);
		}

	for (int i = 0; i < slices; i++)
	{
		indices.push_back(1 + (levels-1)*slices);
		indices.push_back(1 + (levels-2)*slices + (i+1)%slices);
		indices.push_back(1 + (levels-2)*slices + i);
	}
}

vec3f angle(0.0, 0.0, 0.0);

void keydown(unsigned char key, int x, int y)
{
	if (key == 'w')
		angle[0] += m_pi/20.0;
	else if (key == 's')
		angle[0] -= m_pi/20.0;
	else if (key == 'd')
		angle[1] += m_pi/20.0;
	else if (key == 'a')
		angle[1] -= m_pi/20.0;
	else if (key == 'e')
		angle[2] += m_pi/20.0;
	else if (key == 'q')
		angle[2] -= m_pi/20.0;
	else if (key == 27)
		exit(0);
	glutPostRedisplay();
}

void displayfunc()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Set up the modelview matrix
	glLoadIdentity();
	glTranslatef(0.0, 0.0, -5.0);
	glRotatef(radtodeg(angle[0]), 1.0, 0.0, 0.0);
	glRotatef(radtodeg(angle[1]), 0.0, 1.0, 0.0);
	glRotatef(radtodeg(angle[2]), 0.0, 0.0, 1.0);

	// specify the shader to use
	glUseProgram(program);

	// Find the locations of the vertex, normal, and texcoord variables in the shader
	int brick_size_location = glGetUniformLocation(program, "brick_size");
	int brick_color_location = glGetUniformLocation(program, "brick_color");
	int mortar_color_location = glGetUniformLocation(program, "mortar_color");
	int brick_pct_location = glGetUniformLocation(program, "brick_pct");

	// Pass in the parameters for the brick shader
	glUniform1f(brick_size_location, 0.1);
	glUniform3f(brick_color_location, 0.8, 0.0, 0.0);
	glUniform3f(mortar_color_location, 0.4, 0.4, 0.4);
	glUniform2f(brick_pct_location, 0.9, 0.9);

	// Pass in the data for those variables
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glVertexPointer(3, GL_FLOAT, sizeof(GLfloat)*3, vertices.data());
	glNormalPointer(GL_FLOAT, sizeof(GLfloat)*3, normals.data());
	glTexCoordPointer(2, GL_FLOAT, sizeof(GLfloat)*2, texcoords.data());

	// Draw the triangles
	glDrawElements(GL_TRIANGLES, (int)indices.size(), GL_UNSIGNED_INT, indices.data());

	// Clean up
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	glUseProgram(0);

	glutSwapBuffers();
}

void reshapefunc(int w, int h)
{
	glViewport(0, 0, w, h);
	// calculate the projection matrix
	float aspect = (float)h/(float)w;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-1.0/aspect, 1.0/aspect, -1.0, 1.0, 2.0, 20.0);
	glMatrixMode(GL_MODELVIEW);
	glutPostRedisplay();
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	int display_mode = GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE;
#ifdef OSX_CORE3
	display_mode |= GLUT_3_2_CORE_PROFILE;
#endif
	glutInitDisplayMode(display_mode);

	glutInitWindowSize(750, 750);
	glutInitWindowPosition(0, 0);
	window_id = glutCreateWindow("Assignment");

#ifdef __GLEW_H__
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		cerr << "Error: " << glewGetErrorString(err) << endl;
		exit(1);
	}
	cout << "Status: Using GLEW " << glewGetString(GLEW_VERSION) << endl;
#endif

	cout << "Status: Using OpenGL " << glGetString(GL_VERSION) << endl;
	cout << "Status: Using GLSL " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;


	init(string(argv[0]).substr(0, string(argv[0]).find_last_of("/\\")) + "/");

	glutReshapeFunc(reshapefunc);
	glutDisplayFunc(displayfunc);

	glutKeyboardFunc(keydown);

	glutMainLoop();
}
