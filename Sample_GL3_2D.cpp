#include <bits/stdc++.h>
#include <cmath>
#include <fstream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

struct VAO {
  GLuint VertexArrayID;
  GLuint VertexBuffer;
  GLuint ColorBuffer;

  GLenum PrimitiveMode;
  GLenum FillMode;
  int NumVertices;
};
typedef struct VAO VAO;

struct GLMatrices {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
	GLuint MatrixID;
} Matrices;

GLuint programID;

/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open())
	{
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::string Line = "";
		while(getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> VertexShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

	// Link the program
	fprintf(stdout, "Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
	glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
	fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

static void error_callback(int error, const char* description)
{
  fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
  glfwDestroyWindow(window);
  glfwTerminate();
  exit(EXIT_SUCCESS);
}


/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
  struct VAO* vao = new struct VAO;
  vao->PrimitiveMode = primitive_mode;
  vao->NumVertices = numVertices;
  vao->FillMode = fill_mode;

    // Create Vertex Array Object
    // Should be done after CreateWindow and before any other GL calls
    glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
    glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
    glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

    glBindVertexArray (vao->VertexArrayID); // Bind the VAO 
    glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
    glVertexAttribPointer(
                          0,                  // attribute 0. Vertices
                          3,                  // size (x,y,z)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
    glVertexAttribPointer(
                          1,                  // attribute 1. Color
                          3,                  // size (r,g,b)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    return vao;
  }

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
  struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
  {
    GLfloat* color_buffer_data = new GLfloat [3*numVertices];
    for (int i=0; i<numVertices; i++) {
      color_buffer_data [3*i] = red;
      color_buffer_data [3*i + 1] = green;
      color_buffer_data [3*i + 2] = blue;
    }

    return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
  }

/* Render the VBOs handled by VAO */
  void draw3DObject (struct VAO* vao)
  {
    // Change the Fill Mode for this object
    glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

    // Bind the VAO to use
    glBindVertexArray (vao->VertexArrayID);

    // Enable Vertex Attribute 0 - 3d Vertices
    glEnableVertexAttribArray(0);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

    // Enable Vertex Attribute 1 - Color
    glEnableVertexAttribArray(1);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

    // Draw the geometry !
    glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
  }

/**************************
 * Customizable functions *
 **************************/

 float triangle_rot_dir = 1;
 float rectangle_rot_dir = 1;
 bool triangle_rot_status = true;
 bool rectangle_rot_status = true;
 float x_man, y_men;
 bool initial = true;
 bool change = false;

/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
 void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
 {
     // Function is called first on GLFW_PRESS.

  if (action == GLFW_RELEASE) {
    switch (key) {
      case GLFW_KEY_RIGHT:
      x_man += .5;
      initial = false;
      break;
      case GLFW_KEY_LEFT:
      x_man -= .5;
      initial = false;
      break;
      case GLFW_KEY_UP:
      y_men -= .5;
      initial = false;
      break;
      case GLFW_KEY_DOWN:
      y_men += .5;
      initial = false;
      break;
      case GLFW_KEY_C:
      change = !change;
      break;
      default:
      break;
    }
  }
  else if (action == GLFW_PRESS) {
    switch (key) {
      case GLFW_KEY_ESCAPE:
      quit(window);
      break;
      default:
      break;
    }
  }
}

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
	switch (key) {
		case 'Q':
		case 'q':
    quit(window);
    break;
    default:
    break;
  }
}

/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
  switch (button) {
    case GLFW_MOUSE_BUTTON_LEFT:
    if (action == GLFW_RELEASE)
      triangle_rot_dir *= -1;
    break;
    case GLFW_MOUSE_BUTTON_RIGHT:
    if (action == GLFW_RELEASE) {
      rectangle_rot_dir *= -1;
    }
    break;
    default:
    break;
  }
}


/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
  int fbwidth=width, fbheight=height;
    /* With Retina display on Mac OS X, GLFW's FramebufferSize
     is different from WindowSize */
  glfwGetFramebufferSize(window, &fbwidth, &fbheight);

  GLfloat fov = 90.0f;

	// sets the viewport of openGL renderer
  glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

	// set the projection matrix as perspective
	/* glMatrixMode (GL_PROJECTION);
	   glLoadIdentity ();
	   gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
	// Store the projection matrix in a variable for future use
    // Perspective projection for 3D views
     Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

    // Ortho projection for 2D views
    // Matrices.projection = glm::ortho(-4.0f, 4.0f, -4.0f, 4.0f, 0.1f, 500.0f);
   }

   VAO *triangle, *rectangle, *border, *circle, *line;

// Creates the triangle object used in this sample code
   void createSquare()
   {
  /* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */

  /* Define vertex array as used in glBegin (GL_TRIANGLES) */
    static const GLfloat vertex_buffer_data [] = {
    // first face
      -0.5, -0.5, 0,
      -0.5,  0.5, 0,
      0.5,  0.5, 0,
      0.5,  0.5, 0,
      0.5,  -0.5, 0,
      -0.5, -0.5, 0,
    };

    static const GLfloat color_buffer_data [] = {
    0.3424,0.242,0.24234, // color 1
    0.2424,0.2344,0.242, // color 2
    0.2424,0.2424,0.3424, // color 3

    0.2424,0.2424,0.3424, // color 3
    0.213123,0.6788,0.44456, // color 4
    0.3424,0.242,0.24234, // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  triangle = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

// Creates the rectangle object used in this sample code
void createCuboid()
{
  /* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */

  /* Define vertex array as used in glBegin (GL_TRIANGLES) */
  static const GLfloat vertex_buffer_data [] = {
    // first face
    -0.45, -0.45, -.45,
    -0.45,  0.45, -.45,
    0.45,  0.45, -.45,
    0.45,  0.45, -.45,
    0.45,  -0.45, -.45,
    -0.45, -0.45, -.45,

    // second face
    0.45, -0.45, .45,
    0.45,  0.45, .455,
    -0.45,  0.45, .45,
    -0.45,  0.45, .45,
    -0.45, -0.45, .45,
    0.45, -0.45, .45,

    // third face
    0.45, -0.45, -.45,
    0.45,  0.45, -.45,
    0.45,  0.45,  .45,
    0.45,  0.45,  .45,
    0.45, -0.45,  .45,
    0.45, -0.45, -.45,

    // fourth face
    -0.45, -0.45,  .45,
    -0.45,  0.45,  .45,
    -0.45,  0.45, -.45,
    -0.45,  0.45, -.45,
    -0.45, -0.45, -.45,
    -0.45, -0.45,  .45,

    // fifth face
    0.45,  0.45,  .45,
    0.45,  0.45, -.45,
    -0.45,  0.45, -.45,
    -0.45,  0.45, -.45,
    -0.45,  0.45,  .45,
    0.45,  0.45,  .45,

    // sixth face
    0.45, -0.45, -.45,
    0.45, -0.45,  .45,
    -0.45, -0.45,  .45,
    -0.45, -0.45,  .45,
    -0.45, -0.45, -.45,
    0.45, -0.45, -.45,
  };

  static const GLfloat color_buffer_data [] = {
    1,0,0, // color 1
    1,0,0, // color 2
    1,0,0, // color 3

    1,0,0, // color 3
    1,0,0, // color 4
    1,0,0,  // color 1

    0,1,0, // color 1
    0,1,0, // color 2
    0,1,0, // color 3

    0,1,0, // color 3
    0,1,0, // color 4
    0,1,0,  // color 1

    0,0,1, // color 1
    0,0,1, // color 2
    0,0,1, // color 3

    0,0,1, // color 3
    0,0,1, // color 4
    0,0,1,  // color 1

    .2,.2,.2, // color 1
    .2,.2,.2, // color 2
    .2,.2,.2, // color 3

    .2,.2,.2, // color 3
    .2,.2,.2, // color 4
    .2,.2,.2, // color 1

    0.5,0.5,0.5, // color 1
    0.5,0.5,0.5, // color 2
    0.5,0.5,0.5, // color 3

    0.5,0.5,0.5, // color 3
    0.5,0.5,0.5, // color 4
    0.5,0.5,0.5, // color 1

    0.9,0.9,0.9, // color 1
    0.9,0.9,0.9, // color 2
    0.9,0.9,0.9, // color 3

    0.9,0.9,0.9, // color 3
    0.9,0.9,0.9, // color 4
    0.9,0.9,0.9, // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  rectangle = create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createBorder()
{
  /* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */

  /* Define vertex array as used in glBegin (GL_TRIANGLES) */
  static const GLfloat vertex_buffer_data [] = {
    // first face
    -0.5, -0.5, -5.5,
    -0.5,  0.5, -5.5,
    0.5,  0.5, -5.5,
    0.5,  0.5, -5.5,
    0.5,  -0.5, -5.5,
    -0.5, -0.5, -5.5,

    // second face
    0.5, -0.5, 5.5,
    0.5,  0.5, 5.5,
    -0.5,  0.5, 5.5,
    -0.5,  0.5, 5.5,
    -0.5, -0.5, 5.5,
    0.5, -0.5, 5.5,

    // third face
    0.5, -0.5, -5.5,
    0.5,  0.5, -5.5,
    0.5,  0.5,  5.5,
    0.5,  0.5,  5.5,
    0.5, -0.5,  5.5,
    0.5, -0.5, -5.5,

    // fourth face
    -0.5, -0.5,  5.5,
    -0.5,  0.5,  5.5,
    -0.5,  0.5, -5.5,
    -0.5,  0.5, -5.5,
    -0.5, -0.5, -5.5,
    -0.5, -0.5,  5.5,

    // fifth face
    0.5,  0.5,  5.5,
    0.5,  0.5, -5.5,
    -0.5,  0.5, -5.5,
    -0.5,  0.5, -5.5,
    -0.5,  0.5,  5.5,
    0.5,  0.5,  5.5,

    // sixth face
    0.5, -0.5, -5.5,
    0.5, -0.5,  5.5,
    -0.5, -0.5,  5.5,
    -0.5, -0.5,  5.5,
    -0.5, -0.5, -5.5,
    0.5, -0.5, -5.5,
  };

  static const GLfloat color_buffer_data [] = {
    .2,.2,.2, // color 1
    .2,.2,.2, // color 2
    .2,.2,.2, // color 3

    .2,.2,.2, // color 1
    .2,.2,.2, // color 2
    .2,.2,.2, // color 3

    .2,.2,.2, // color 1
    .2,.2,.2, // color 2
    .2,.2,.2, // color 3

    .2,.2,.2, // color 1
    .2,.2,.2, // color 2
    .2,.2,.2, // color 3

    .2,.2,.2, // color 1
    .2,.2,.2, // color 2
    .2,.2,.2, // color 3

    .2,.2,.2, // color 1
    .2,.2,.2, // color 2
    .2,.2,.2, // color 3

    .2,.2,.2, // color 1
    .2,.2,.2, // color 2
    .2,.2,.2, // color 3

    .2,.2,.2, // color 1
    .2,.2,.2, // color 2
    .2,.2,.2, // color 3

    .2,.2,.2, // color 1
    .2,.2,.2, // color 2
    .2,.2,.2, // color 3

    .2,.2,.2, // color 1
    .2,.2,.2, // color 2
    .2,.2,.2, // color 3

    .2,.2,.2, // color 1
    .2,.2,.2, // color 2
    .2,.2,.2, // color 3

    .2,.2,.2, // color 1
    .2,.2,.2, // color 2
    .2,.2,.2, // color 3
  };

  border = create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createCircle()
{
  GLfloat  PI = 3.141592654;
  GLfloat angle = 0.0;
  int points = 100;
  static GLfloat vertex_buffer_data[300];
  static GLfloat color_buffer_data[300];
  /* code for circle*/
  for(int i = 0; i < 3*points;){
    angle = PI * i / points;
    vertex_buffer_data[i++] = .2*(float)cos(angle);
    vertex_buffer_data[i++] = .2*(float)sin(angle);
    vertex_buffer_data[i++] = .2;
  }
  /* code for circle*/
  for(int i = 0 ; i < 3*points ; ){
    color_buffer_data[i++] = .78;
    color_buffer_data[i++] = .2323;
    color_buffer_data[i++] = .321;
  }
  circle = create3DObject(GL_TRIANGLE_FAN, points, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createLine()
{
  static const GLfloat vertex_buffer_data [] = {
    // first face
    -0.5, -0.5, -1.5,
    -0.5,  0.5, -1.5,
    0.5,  0.5, -1.5,
    0.5,  0.5, -1.5,
    0.5,  -0.5, -1.5,
    -0.5, -0.5, -1.5,

    // second face
    0.5, -0.5, 1.5,
    0.5,  0.5, 1.5,
    -0.5,  0.5, 1.5,
    -0.5,  0.5, 1.5,
    -0.5, -0.5, 1.5,
    0.5, -0.5, 1.5,

    // third face
    0.5, -0.5, -1.5,
    0.5,  0.5, -1.5,
    0.5,  0.5,  1.5,
    0.5,  0.5,  1.5,
    0.5, -0.5,  1.5,
    0.5, -0.5, -1.5,

    // fourth face
    -0.5, -0.5,  1.5,
    -0.5,  0.5,  1.5,
    -0.5,  0.5, -1.5,
    -0.5,  0.5, -1.5,
    -0.5, -0.5, -1.5,
    -0.5, -0.5,  1.5,

    // fifth face
    0.5,  0.5,  1.5,
    0.5,  0.5, -1.5,
    -0.5,  0.5, -1.5,
    -0.5,  0.5, -1.5,
    -0.5,  0.5,  1.5,
    0.5,  0.5,  1.5,

    // sixth face
    0.5, -0.5, -1.5,
    0.5, -0.5,  1.5,
    -0.5, -0.5,  1.5,
    -0.5, -0.5,  1.5,
    -0.5, -0.5, -1.5,
    0.5, -0.5, -1.5,
  };

  static const GLfloat color_buffer_data [] = {
    .78, .2323, .321,
    .78, .2323, .321,
    .78, .2323, .321,

    .78, .2323, .321,
    .78, .2323, .321,
    .78, .2323, .321,

    .78, .2323, .321,
    .78, .2323, .321,
    .78, .2323, .321,

    .78, .2323, .321,
    .78, .2323, .321,
    .78, .2323, .321,

    .78, .2323, .321,
    .78, .2323, .321,
    .78, .2323, .321,

    .78, .2323, .321,
    .78, .2323, .321,
    .78, .2323, .321,

    .78, .2323, .321,
    .78, .2323, .321,
    .78, .2323, .321,

    .78, .2323, .321,
    .78, .2323, .321,
    .78, .2323, .321,

    .78, .2323, .321,
    .78, .2323, .321,
    .78, .2323, .321,

    .78, .2323, .321,
    .78, .2323, .321,
    .78, .2323, .321,

    .78, .2323, .321,
    .78, .2323, .321,
    .78, .2323, .321,

    .78, .2323, .321,
    .78, .2323, .321,
    .78, .2323, .321,
  };
  line = create3DObject(GL_LINES, 2, vertex_buffer_data, color_buffer_data, GL_FILL);
}

float camera_rotation_angle = 90;
float rectangle_rotation = 0;
float triangle_rotation = 0;
bool flag = true;
bool v[10][10];
int vibration=1;

/* Render the scene with openGL */
/* Edit this function according to your assignment */
void draw ()
{
  // clear the color and depth in the frame buffer
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // use the loaded shader program
  // Don't change unless you know what you are doing
  glUseProgram (programID);

  // Eye - Location of camera. Don't change unless you are sure!!
  vec3 eye;
  if(change)
    eye = vec3(0, 0, 5);
  else
    eye = vec3(0, -7, 3);
  // Target - Where is the camera looking at.  Don't change unless you are sure!!
  glm::vec3 target (0, 0, 0);
  // Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
  glm::vec3 up (0, 1, 0);

  // Compute Camera matrix (view)
  Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
  //  Don't change unless you are sure!!
  // Matrices.view = glm::lookAt(glm::vec3(0,0,10), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

  // Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
  //  Don't change unless you are sure!!
  glm::mat4 VP = Matrices.projection * Matrices.view;

  // Send our transformation to the currently bound shader, in the "MVP" uniform
  // For each model you render, since the MVP will be different (at least the M part)
  //  Don't change unless you are sure!!
  glm::mat4 MVP;	// MVP = Projection * View * Model

  // Load identity to model matrix
  Matrices.model = glm::mat4(1.0f);

  /* Render your scene */

  // glm::mat4 translateTriangle = glm::translate (glm::vec3(-2.0f, 0.0f, 0.0f)); // glTranslatef
  // glm::mat4 rotateTriangle = glm::rotate((float)(triangle_rotation*M_PI/180.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
  camera_rotation_angle++;
  // glm::mat4 triangleTransform = translateTriangle * rotateTriangle;
  // Matrices.model *= triangleTransform; 
  MVP = VP * Matrices.model; // MVP = p * V * M

  //  Don't change unless you are sure!!
  // glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  // float i = 0, j = 0;
  if(flag)
  {
    for(int i = 0 ; i < 10 ; i++)
    {
      for(int j = 0 ; j < 10 ; j++)
        v[i][j] = true;
    }

    for(int i = 0 ; i < 10 ; i++)
    {
      int a=rand()%10;
      if(!(a == i && (a != 9 || a != 0)))
        v[i][a] = false;
    }
    flag = false;
  }
  MVP *= translate(vec3(-5.0,-5.0,0.0f));
  for(int row = 0 ; row < 10 ; row++)
  {
    for(int col = 0 ; col < 10 ; col++)
    {
      glm::mat4 translateTriangle = glm::translate (glm::vec3(1, 0, 0.0f)); // glTranslatef
      MVP *= translateTriangle;
      glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
      if(v[row][col])
        draw3DObject(triangle);
    }
    glm::mat4 translateTriangle = glm::translate (glm::vec3(-10, 1, 0.0f)); // glTranslatef
    MVP *= translateTriangle;
  }

  MVP = VP * Matrices.model; // MVP = p * V * M
  MVP *= translate(vec3(-5.0,-5.0,0.5f));
  for(int i = 0 ; i < 10 ; i++)
  {
    mat4 translateRectangle;
    for(int j = 0 ; j < 10 ; j++)
    {
      translateRectangle = glm::translate (glm::vec3(1, 0, .004 * std::sin(vibration*M_PI/180)));        // glTranslatef
      MVP *= translateRectangle;
      glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
      if(v[i][j])
        draw3DObject(rectangle);
    }
    translateRectangle = glm::translate (glm::vec3(-10, 1, 0.0f)); // glTranslatef
    MVP *= translateRectangle;
  }

  MVP = VP * Matrices.model;
  mat4 rotateBorder = rotate((float)(90.0f*M_PI/180.0f), vec3(0,1,0));
  mat4 translateBorder = translate(vec3(0,5,0));
  MVP *= rotateBorder*translateBorder;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(border);

  MVP = VP * Matrices.model;
  rotateBorder = rotate((float)(90.0f*M_PI/180.0f), vec3(0,1,0));
  translateBorder = translate(vec3(0,-6,0.5));
  MVP *= rotateBorder*translateBorder;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(border);

  MVP = VP * Matrices.model;
  rotateBorder = rotate((float)(90.0f*M_PI/180.0f), vec3(1,0,0));
  translateBorder = translate(vec3(6,0,0));
  MVP *= rotateBorder*translateBorder;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(border);

  MVP = VP * Matrices.model;
  rotateBorder = rotate((float)(90.0f*M_PI/180.0f), vec3(1,0,0));
  translateBorder = translate(vec3(-5,0,0));
  MVP *= rotateBorder*translateBorder;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(border);

  if(initial)
  {
    MVP = VP * Matrices.model;
    translateBorder = translate(vec3(-4,-5,1.4 + .004 * sin(vibration*M_PI/180)));
    MVP *= translateBorder;
    x_man = .5; y_men = 4.5;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    draw3DObject(circle);
  }
  else
  {
    if(x_man > 9.5 || x_man < 1 || y_men < -4.5 || y_men > 4.5)
    {
      initial = true;
      if(x_man >= 9 && y_men <= -4)
      {
        initial = true;
        std::cout << "You Win" << '\n';
      }
      else
        std::cout << "You lose" << '\n';
    }
    else if(x_man == 9.5 && y_men == -4.5)
    {
      initial = true;
      std::cout << "You Win" << '\n';
    }
    for(int i = 0 ; i < 10 ; i++)
    {
      for(int j = 0 ; j < 10 ; j++)
      {
        if(!v[i][j] && (std::fabs(i-x_man) < .5 && std::fabs(j-y_men) < .5))
        {
          initial = true;
          std::cout << "You lose" << '\n';
          break;
        }
      }
      if(initial)
        break;
    }
    translateBorder = translate(vec3(x_man,1.4,y_men + .004 * sin(vibration*M_PI/180)));
    MVP *= translateBorder;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    draw3DObject(circle);
  }

  // Increment angles
  float increments = 1;
  vibration++;
  //camera_rotation_angle++; // Simulating camera rotation
  // triangle_rotation = triangle_rotation + increments*triangle_rot_dir*triangle_rot_status;
  // rectangle_rotation = rectangle_rotation + increments*rectangle_rot_dir*rectangle_rot_status;
}

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height)
{
    GLFWwindow* window; // window desciptor/handle

    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
      exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "Sample OpenGL 3.3 Application", NULL, NULL);

    if (!window) {
      glfwTerminate();
      exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval( 1 );

    /* --- register callbacks with GLFW --- */

    /* Register function to handle window resizes */
    /* With Retina display on Mac OS X GLFW's FramebufferSize
     is different from WindowSize */
    glfwSetFramebufferSizeCallback(window, reshapeWindow);
    glfwSetWindowSizeCallback(window, reshapeWindow);

    /* Register function to handle window close */
    glfwSetWindowCloseCallback(window, quit);

    /* Register function to handle keyboard input */
    glfwSetKeyCallback(window, keyboard);      // general keyboard input
    glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

    /* Register function to handle mouse click */
    glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks

    return window;
  }

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
  void initGL (GLFWwindow* window, int width, int height)
  {
    /* Objects should be created before any other gl function and shaders */
	// Create the models
	createSquare (); // Generate the VAO, VBOs, vertices data & copy into the array buffer
	createCuboid ();
  createBorder ();
  createCircle ();
  createLine ();

	// Create and compile our GLSL program from the shaders
  programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
	// Get a handle for our "MVP" uniform
  Matrices.MatrixID = glGetUniformLocation(programID, "MVP");


  reshapeWindow (window, width, height);

    // Background color of the scene
  // glClearColor (0.3f, 0.3f, 0.3f, 0.0f); // R, G, B, A
  glClearColor (0.223529412f, 0.34509803921f, 0.47450980392f, 0.38431f); // R, G, B, A
	glClearDepth (1.0f);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);

  std::cout << "VENDOR: " << glGetString(GL_VENDOR) << std::endl;
  std::cout << "RENDERER: " << glGetString(GL_RENDERER) << std::endl;
  std::cout << "VERSION: " << glGetString(GL_VERSION) << std::endl;
  std::cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
}

int main (int argc, char** argv)
{
	int width = 1280;
	int height = 720;

  GLFWwindow* window = initGLFW(width, height);

  initGL (window, width, height);

  double last_update_time = glfwGetTime(), current_time;

    /* Draw in loop */
  while (!glfwWindowShouldClose(window)) {

        // OpenGL Draw commands
    draw();

        // Swap Frame Buffer in double buffering
    glfwSwapBuffers(window);

        // Poll for Keyboard and mouse events
    glfwPollEvents();

        // Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
        current_time = glfwGetTime(); // Time in seconds
        if ((current_time - last_update_time) >= 0.5) { // atleast 0.5s elapsed since last frame
            // do something every 0.5 seconds ..
          last_update_time = current_time;
        }
      }

      glfwTerminate();
      exit(EXIT_SUCCESS);
    }
