/* Lab 6 base code - transforms using matrix stack built on glm 
	CPE 471 Cal Poly Z. Wood + S. Sueda
*/
#include <iostream>
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "GLSL.h"
#include "Program.h"
#include "MatrixStack.h"
#include "Shape.h"
#include "Texture.h"

// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace glm;

/* to use glee */
#define GLEE_OVERWRITE_GL_FUNCTIONS
#include "glee.hpp"

GLFWwindow *window; // Main application window
string RESOURCE_DIR = ""; // Where the resources are loaded from
shared_ptr<Program> shadow;
shared_ptr<Program> mat_prog;
shared_ptr<Program> tex_prog;
shared_ptr<Program> bt_tex_prog;
shared_ptr<Program> water;
shared_ptr<Shape> sphere;
shared_ptr<Shape> cube;
shared_ptr<Shape> bunny;
shared_ptr<Texture> shadowTex;
shared_ptr<Texture> stone;
shared_ptr<Texture> waterTex;

double theta = 0.0;
double phi = 0.0;
float mov_w = 0;
float mov_v = 0;
float mov_u = 0;
vec3 eye_pt = vec3(0,0,0);
float jump_time = 0.0f;
float pillar = 0;
bool takeBunny = false;
float flow_out = 0;
float flow_up = 0;

GLuint shadowFB;

GLfloat light[] = {5.0f, 4.0f, 0.0f};
GLfloat Lc[] = {1.0f, 1.0f, 1.0f, 1.0f};

static void error_callback(int error, const char *description)
{
	cerr << description << endl;
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if(action == GLFW_PRESS) {
      switch(key) {
         case GLFW_KEY_ESCAPE:
         glfwSetWindowShouldClose(window, GL_TRUE);
         break;
         case GLFW_KEY_W:
            mov_w = 0.1f;
         break;
         case GLFW_KEY_S:
            mov_w = -0.1f;
         break;
         case GLFW_KEY_A:
            mov_v = -0.1f;
         break;
         case GLFW_KEY_D:
            mov_v = 0.1f;
         break;
         case GLFW_KEY_SPACE:
            if(jump_time < 0.01f) {
               jump_time = 0.011f;
            }
         break;
         case GLFW_KEY_Q:
            light[0] -= 0.2f;
            cout << *light << endl;
         break;
         case GLFW_KEY_E:
            light[0] += 0.2f;
            cout << *light << endl;
         break;
      }
	}

	if(action == GLFW_RELEASE) {
      switch(key) {
         case GLFW_KEY_W:
         case GLFW_KEY_S:
            mov_w = 0.0f;
         break;
         case GLFW_KEY_A:
         case GLFW_KEY_D:
            mov_v = 0.0f;
         break;
         case GLFW_KEY_SPACE:
            mov_u = 0.0f;
         break;
      }
	}

}

static void mouse_callback(GLFWwindow *window, int button, int action, int mods)
{
   if (action == GLFW_PRESS) {
      takeBunny = true;
      
	}
}

static void scroll_callback(GLFWwindow * window, double deltaX, double deltaY) {
   int width, height;

	glfwGetFramebufferSize(window, &width, &height);

   theta += (deltaX/width) * 6.28;
   phi += (deltaY/height) * 6.28;

   if( phi > 1.57) {
      phi = 1.57;
   }
   else if (phi < -1.57) {
      phi = -1.57;
   }
}

static void resize_callback(GLFWwindow *window, int width, int height) {
   glViewport(0, 0, width, height);
}

static void init()
{
   int width, height;

	GLSL::checkVersion();

	// Set background color.
	glClearColor(.12f, .34f, .56f, 1.0f);
	// Enable z-buffer test.
	glEnable(GL_DEPTH_TEST);
   glEnable(GL_CULL_FACE);

	glfwGetFramebufferSize(window, &width, &height);

	// Initialize mesh.
	sphere = make_shared<Shape>();
	sphere->loadMesh(RESOURCE_DIR + "hi-res-sphere.obj");
	sphere->resize();
	sphere->init();

	cube = make_shared<Shape>();
	cube->loadMesh(RESOURCE_DIR + "cube.obj");
	cube->resize();
	cube->init();

	bunny = make_shared<Shape>();
	bunny->loadMesh(RESOURCE_DIR + "bunny.obj");
	bunny->resize();
	bunny->init();

   glGenFramebuffers(1, &shadowFB);

   shadowTex = make_shared<Texture>();
   shadowTex->setUnit(0);
   shadowTex->initShadow(shadowFB);

   stone = make_shared<Texture>();
   stone->setFilename(RESOURCE_DIR + "stone_wall_texture.bmp");
   stone->init();
   stone->setUnit(1);
   stone->setWrapModes(GL_REPEAT, GL_REPEAT);

   waterTex = make_shared<Texture>();
   waterTex->setFilename(RESOURCE_DIR + "water.jpeg");
   waterTex->init();
   waterTex->setUnit(2);
   waterTex->setWrapModes(GL_REPEAT, GL_REPEAT);

	shadow = make_shared<Program>();
	shadow->setVerbose(true);
	shadow->setShaderNames(RESOURCE_DIR + "shadow_vert.glsl",
    RESOURCE_DIR + "shadow_frag.glsl");
	shadow->init();
	shadow->addUniform("P");
	shadow->addUniform("M");
	shadow->addUniform("V");
	shadow->addAttribute("vertPos");

   water = make_shared<Program>();
   water->setVerbose(true);
   water->setShaderNames(RESOURCE_DIR + "tex_vert.glsl", RESOURCE_DIR + "tex_frag.glsl");
   water->init();
   water->addUniform("P");
   water->addUniform("V");
   water->addUniform("M");
   water->addUniform("tex");
   water->addUniform("light");
   water->addUniform("Lc");
	water->addAttribute("vertPos");
	water->addAttribute("vertNor");

	// Initialize the GLSL program.
	mat_prog = make_shared<Program>();
	mat_prog->setVerbose(true);
	mat_prog->setShaderNames(RESOURCE_DIR + "simple_vert.glsl", RESOURCE_DIR + "simple_frag.glsl");
	mat_prog->init();
	mat_prog->addUniform("P");
	mat_prog->addUniform("M");
	mat_prog->addUniform("V");
	mat_prog->addUniform("Ps");
	mat_prog->addUniform("Vs");
   mat_prog->addUniform("Kd");
   mat_prog->addUniform("Ks");
   mat_prog->addUniform("Ka");
   mat_prog->addUniform("shine");
   mat_prog->addUniform("light");
   mat_prog->addUniform("shadowTex");
   mat_prog->addUniform("Lc");
	mat_prog->addAttribute("vertPos");
	mat_prog->addAttribute("vertNor");

	tex_prog = make_shared<Program>();
	tex_prog->setVerbose(true);
	tex_prog->setShaderNames(RESOURCE_DIR + "tex_vert.glsl", RESOURCE_DIR + "tex_frag.glsl");
	tex_prog->init();
	tex_prog->addUniform("P");
	tex_prog->addUniform("M");
	tex_prog->addUniform("V");
   tex_prog->addUniform("light");
   tex_prog->addUniform("tex");
   tex_prog->addUniform("Lc");
	tex_prog->addAttribute("vertPos");
	tex_prog->addAttribute("vertNor");

	bt_tex_prog = make_shared<Program>();
	bt_tex_prog->setVerbose(true);
	bt_tex_prog->setShaderNames(RESOURCE_DIR + "bt_tex_vert.glsl", RESOURCE_DIR + "bt_tex_frag.glsl");
	bt_tex_prog->init();
	bt_tex_prog->addUniform("P");
	bt_tex_prog->addUniform("M");
	bt_tex_prog->addUniform("V");
	bt_tex_prog->addUniform("Ps");
	bt_tex_prog->addUniform("Vs");
   bt_tex_prog->addUniform("light");
   bt_tex_prog->addUniform("stone");
   bt_tex_prog->addUniform("shadowTex");
   bt_tex_prog->addUniform("Lc");
	bt_tex_prog->addAttribute("vertPos");
	bt_tex_prog->addAttribute("vertNor");
}

void SetMaterial(shared_ptr<Program> prog, int i) {
   switch (i) {
      case 0: //shiny blue plastic
         glUniform3f(prog->getUniform("Ka"), 0.02, 0.04, 0.2);
         glUniform3f(prog->getUniform("Kd"), 0.0, 0.16, 0.9);
         glUniform3f(prog->getUniform("Ks"), 0.14, 0.2, 0.8);
         glUniform1f(prog->getUniform("shine"), 120.0);
         break;
      case 1: // flat grey
         glUniform3f(prog->getUniform("Ka"), 0.13, 0.13, 0.14);
         glUniform3f(prog->getUniform("Kd"), 0.3, 0.3, 0.4);
         glUniform3f(prog->getUniform("Ks"), 0.3, 0.3, 0.4);
         glUniform1f(prog->getUniform("shine"), 4.0);
         break;
      case 2: //brass
         glUniform3f(prog->getUniform("Ka"), 0.3294, 0.2235, 0.02745);
         glUniform3f(prog->getUniform("Kd"), 0.7804, 0.5686, 0.11373);
         glUniform3f(prog->getUniform("Ks"), 0.9922, 0.941176, 0.80784);
         glUniform1f(prog->getUniform("shine"), 27.9);
         break;
      case 3: //copper
         glUniform3f(prog->getUniform("Ka"), 0.1913, 0.0735, 0.0225);
         glUniform3f(prog->getUniform("Kd"), 0.7038, 0.27048, 0.0828);
         glUniform3f(prog->getUniform("Ks"), 0.257, 0.1376, 0.08601);
         glUniform1f(prog->getUniform("shine"), 12.8);
         break;
      case 4: //emerald
         glUniform3f(prog->getUniform("Ka"),0.0215, 0.1745, 0.0215);
         glUniform3f(prog->getUniform("Kd"), 0.07568, 0.61424, 0.07568);
         glUniform3f(prog->getUniform("Ks"), 0.633, 0.727811, 0.633);
         glUniform1f(prog->getUniform("shine"), 0.6);
         break;
      case 5: //chrome
         glUniform3f(prog->getUniform("Ka"), 0.25, 0.25, 0.25);
         glUniform3f(prog->getUniform("Kd"), 0.4, 0.4, 0.4);
         glUniform3f(prog->getUniform("Ks"), 0.774597, 0.774597, 0.774597);
         glUniform1f(prog->getUniform("shine"), 0.6);
         break;
   }
}

static void drawScene(shared_ptr<Program> prog, bool fullPass) {
   auto M = make_shared<MatrixStack>();

   M->pushMatrix();
      M->translate(vec3(0, -1, 0));
      glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix()));
      if(!takeBunny) {
         if(fullPass) {
            SetMaterial(prog, 4);
            bunny->draw(prog);
         }
         else {
            bunny->shadow(prog);
         }
      }
   M->popMatrix();
   M->pushMatrix();
      M->translate(vec3(0, -32 + pillar, 0));
      M->scale(vec3(2, 30, 2));
      glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix()));
      if(fullPass) {
         SetMaterial(prog, 3);
         cube->draw(prog);
      }
      else {
         cube->shadow(prog);
      }
   M->popMatrix();
   if(!fullPass) {
      M->pushMatrix();
         M->translate(eye_pt);
         glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix()));
         sphere->shadow(prog);
      M->popMatrix();
   }
   if(fullPass) {
      M->pushMatrix();
         M->translate(vec3(light[0], light[1], light[2]));
         glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix()));
         //sphere->draw(prog);
      M->popMatrix();
   }
}

static void setupDraw(shared_ptr<Program> prog, vec3 lookAt_pt) {
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
   float aspect = width/(float)height;

   auto P = make_shared<MatrixStack>();
   auto V = make_shared<MatrixStack>();

   glUniform3fv(prog->getUniform("light"), 1, light);
   glUniform4fv(prog->getUniform("Lc"), 1, Lc);

   P->pushMatrix();
      P->perspective(45.0f, aspect, 0.01f, 100.0f);
      glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, value_ptr(P->topMatrix()));
   P->popMatrix();

   V->pushMatrix();
      V->lookAt(eye_pt, lookAt_pt + eye_pt, vec3(0,1,0));
      glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, value_ptr(V->topMatrix()));
   V->popMatrix();
}

static void drawRoom(shared_ptr<Program> prog, bool fullPass) {
   auto M = make_shared<MatrixStack>();

   M->pushMatrix();
      M->translate(vec3(0,0,19.9));
      M->scale(vec3(20, 20, 0.0001));
      glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix()));
   M->popMatrix();

   if(fullPass) {
      cube->draw(prog);
   }
   else {
      cube->shadow(prog);
   }

   M->pushMatrix();
      M->translate(vec3(0,0,-19.9));
      M->scale(vec3(20, 20, 0.0001));
      glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix()));
   M->popMatrix();

   if(fullPass) {
      cube->draw(prog);
   }
   else {
      cube->shadow(prog);
   }

   M->pushMatrix();
      M->translate(vec3(19.9,0,0));
      M->rotate(1.57, vec3(0, 1, 0));
      M->scale(vec3(20, 20, 0.0001));
      glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix()));
   M->popMatrix();

   if(fullPass) {
      cube->draw(prog);
   }
   else {
      cube->shadow(prog);
   }

   M->pushMatrix();
      M->translate(vec3(-19.9,0,0));
      M->rotate(1.57, vec3(0, 1, 0));
      M->scale(vec3(20, 20, 0.0001));
      glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix()));
   M->popMatrix();

   if(fullPass) {
      cube->draw(prog);
   }
   else {
      cube->shadow(prog);
   }

   M->pushMatrix();
      M->translate(vec3(0,19.9,0));
      M->rotate(1.57, vec3(1, 0, 0));
      M->scale(vec3(20, 20, 0.0001));
      glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix()));
   M->popMatrix();

   if(fullPass) {
      cube->draw(prog);
   }
   else {
      cube->shadow(prog);
   }
}

static void drawFloor(shared_ptr<Program> prog, bool fullPass) {
   auto M = make_shared<MatrixStack>();

   M->pushMatrix();
      M->translate(vec3(0,-4,0));
      M->rotate(1.57, vec3(1, 0, 0));
      M->scale(vec3(20, 20, 0.01));
      glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix()));
   M->popMatrix();

   if(fullPass) {
      cube->draw(prog);
   }
   else {
      cube->shadow(prog);
   }
}

static void drawWater(shared_ptr<Program> prog, bool fullPass) {
   auto M = make_shared<MatrixStack>();

   M->pushMatrix();
      M->translate(vec3(0, -3.9, 0));
      M->scale(vec3(1+flow_out,0.01+flow_up,1+flow_out));
      M->rotate(1.57, vec3(1, 0, 0));
      glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix()));
   M->popMatrix();

   if(fullPass) {
      cube->draw(prog);
   }
   else {
      cube->shadow(prog);
   }
}

static void render()
{
	// Get current frame buffer size.
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);

	// Clear framebuffer.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   // Create the matrix stacks - please leave these alone for now
   auto P = make_shared<MatrixStack>();
   auto V = make_shared<MatrixStack>();

   P->pushMatrix();
   V->pushMatrix();
      P->ortho(-30, 30, -30, 30, -10, 15);
      V->lookAt(vec3(light[0], light[1], light[2]), vec3(0,0,0), vec3(0,1,0));

      shadow->bind();
         glUniformMatrix4fv(shadow->getUniform("P"), 1, GL_FALSE, value_ptr(P->topMatrix()));
         glUniformMatrix4fv(shadow->getUniform("V"), 1, GL_FALSE, value_ptr(V->topMatrix()));
      shadow->unbind();
      mat_prog->bind();
         glUniformMatrix4fv(mat_prog->getUniform("Ps"), 1, GL_FALSE, value_ptr(P->topMatrix()));
         glUniformMatrix4fv(mat_prog->getUniform("Vs"), 1, GL_FALSE, value_ptr(V->topMatrix()));
      mat_prog->unbind();
      bt_tex_prog->bind();
         glUniformMatrix4fv(bt_tex_prog->getUniform("Ps"), 1, GL_FALSE, value_ptr(P->topMatrix()));
         glUniformMatrix4fv(bt_tex_prog->getUniform("Vs"), 1, GL_FALSE, value_ptr(V->topMatrix()));
      bt_tex_prog->unbind();
   V->popMatrix();
   P->popMatrix();

   glCullFace(GL_FRONT);
   glBindFramebuffer(GL_FRAMEBUFFER, shadowFB);
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   shadow->bind();
   drawWater(shadow, false);
   drawFloor(shadow, false);
   drawScene(shadow, false);
   shadow->unbind();

   vec3 lookAt_pt = vec3(0,0,0);
   lookAt_pt.x = 100 * cos(phi) * cos(theta);
   lookAt_pt.y = 100 * sin(phi);
   lookAt_pt.z = 100 * cos(phi) * cos(1.57 - theta);

   glCullFace(GL_BACK);
   glBindFramebuffer(GL_FRAMEBUFFER, 0);
   glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glViewport(0, 0, width, height);

   bt_tex_prog->bind();
   stone->bind(bt_tex_prog->getUniform("stone"));
   shadowTex->bind(bt_tex_prog->getUniform("shadowTex"));

   setupDraw(bt_tex_prog, lookAt_pt);
   drawFloor(bt_tex_prog, true);

   bt_tex_prog->unbind();

   tex_prog->bind();
   stone->bind(tex_prog->getUniform("tex"));

   setupDraw(tex_prog, lookAt_pt);
   drawRoom(tex_prog, true);

   tex_prog->unbind();

	mat_prog->bind();
   shadowTex->bind(mat_prog->getUniform("shadowTex"));

   setupDraw(mat_prog, lookAt_pt);
   drawScene(mat_prog, true);

   mat_prog->unbind();

   glDisable(GL_CULL_FACE);
   water->bind();
   waterTex->bind(tex_prog->getUniform("tex"));

   setupDraw(water, lookAt_pt);
   drawWater(water, true);

   water->unbind();
   glEnable(GL_CULL_FACE);

   vec3 w = normalize(lookAt_pt);
   vec3 v = cross(w, vec3(0,1,0));
   vec3 u = -1.0f * cross(w,v);

   eye_pt += mov_w * w;
   eye_pt += mov_v * v;
   eye_pt += mov_u * u;

   eye_pt.y = 1.0f + 4.0 * sin(2.0f * jump_time);

   if(sin(2.0f * jump_time) < 0.01f ) {
      jump_time = 0.0f;
   }
   else {
      jump_time += 0.02f;
   }

   if(takeBunny && pillar < 6) {
      pillar += 0.01f;
   }

   if(takeBunny && pillar >= 6 && flow_out < 18) {
      flow_out += 0.025f;
   }

   if(takeBunny && flow_out >= 18 && flow_up < 6) {
      flow_up += 0.01f;
   }

   if(takeBunny && flow_up >= 6 && Lc[0] > 0) {
      Lc[0] -= 0.0025f;
      Lc[1] -= 0.0025f;
      Lc[2] -= 0.0025f;
   }
}

int main(int argc, char **argv)
{
	if(argc < 2) {
		cout << "Please specify the resource directory." << endl;
		return 0;
	}
	RESOURCE_DIR = argv[1] + string("/");

	// Set error callback.
	glfwSetErrorCallback(error_callback);
	// Initialize the library.
	if(!glfwInit()) {
		return -1;
	}
   //request the highest possible version of OGL - important for mac
   glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
   glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);

	// Create a windowed mode window and its OpenGL context.
	window = glfwCreateWindow(640, 480, "A Whole New World", NULL, NULL);
	if(!window) {
		glfwTerminate();
		return -1;
	}
	// Make the window's context current.
	glfwMakeContextCurrent(window);
	// Initialize GLEW.
	glewExperimental = true;
	if(glewInit() != GLEW_OK) {
		cerr << "Failed to initialize GLEW" << endl;
		return -1;
	}
	//weird bootstrap of glGetError
   glGetError();
	cout << "OpenGL version: " << glGetString(GL_VERSION) << endl;
   cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

	// Set vsync.
	glfwSwapInterval(1);
	// Set keyboard callback.
	glfwSetKeyCallback(window, key_callback);
   //set the mouse call back
   glfwSetMouseButtonCallback(window, mouse_callback);
   glfwSetScrollCallback(window, scroll_callback);
   //set the window resize call back
   glfwSetFramebufferSizeCallback(window, resize_callback);

	// Initialize scene. Note geometry initialized in init now
	init();

	// Loop until the user closes the window.
	while(!glfwWindowShouldClose(window)) {
		// Render scene.
		render();
		// Swap front and back buffers.
		glfwSwapBuffers(window);
		// Poll for and process events.
		glfwPollEvents();
	}
	// Quit program.
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
