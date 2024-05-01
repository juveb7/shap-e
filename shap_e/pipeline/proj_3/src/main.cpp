/* Lab 6 base code - transforms using matrix stack built on glm
 * CPE 471 Cal Poly Z. Wood + S. Sueda + I. Dunn
 */

#include <iostream>
#include <glad/glad.h>

#include "GLSL.h"
#include "Program.h"
#include "Shape.h"
#include "MatrixStack.h"
#include "WindowManager.h"
#include "Camera.h"
#include "Texture.h"

#include <tiny_obj_loader/tiny_obj_loader.h>

 // value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace glm;

class Application : public EventCallbacks
{

public:

	WindowManager* windowManager = nullptr;
	Camera camera;
	Texture floor_texture;
	Texture cat_texture;

	// Our shader program
	std::shared_ptr<Program> prog;
	std::shared_ptr<Program> floorProg;
	std::shared_ptr<Program> catProg;

	// Shape to be used (from obj file)
	shared_ptr<Shape> head;
	shared_ptr<Shape> body;
	shared_ptr<Shape> arm_upper;
	shared_ptr<Shape> arm_lower;
	shared_ptr<Shape> leg_upper;
	shared_ptr<Shape> leg_lower;
	shared_ptr<Shape> flower;

	shared_ptr<Shape> floor;
	shared_ptr<Shape> cat;

	vector<shared_ptr<Shape>> flowerMesh;

	// Contains vertex information for OpenGL
	GLuint VertexArrayID;

	// Data necessary to give our triangle to OpenGL
	GLuint VertexBufferID;

	bool animate = false;

	float lightTrans = 0;
	//float gRot = 0;
	//float gTrans = 0;
	float rotAmt = 0;
	float rotInc = 0.01;

	float deg45 = 0.785398;
	float deg90 = 1.5708;
	float deg135 = 2.35619;

	void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		if (key == GLFW_KEY_Z && action == GLFW_PRESS) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		if (key == GLFW_KEY_Z && action == GLFW_RELEASE) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
		if (key == GLFW_KEY_O && action == GLFW_RELEASE) {
			lightTrans--;
		}
		if (key == GLFW_KEY_P && action == GLFW_RELEASE) {
			lightTrans++;
		}
		if (key == GLFW_KEY_A && action == GLFW_REPEAT) {
			camera.moveLeft();
			//gRot -= 0.16;
		}
		if (key == GLFW_KEY_D && action == GLFW_REPEAT) {
			camera.moveRight();
			//gRot += 0.16;
		}
		if (key == GLFW_KEY_W && action == GLFW_REPEAT) {
			camera.moveForward();
			//gTrans++;
		}
		if (key == GLFW_KEY_S && action == GLFW_REPEAT) {
			camera.moveBack();
			//gTrans--;
		}
		if (key == GLFW_KEY_SPACE && action == GLFW_RELEASE) {
			animate = !animate;
		}
	}

	void mouseCallback(GLFWwindow* window, double xpos, double ypos)
	{
		camera.lookAround(window, xpos, ypos);
	}

	void resizeCallback(GLFWwindow* window, int width, int height)
	{
		glViewport(0, 0, width, height);
	}

	void init(const std::string& resourceDirectory)
	{
		GLSL::checkVersion();

		// Set background color.
		glClearColor(.12f, .34f, .56f, 1.0f);
		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);

		// Initialize the GLSL program.
		prog = make_shared<Program>();
		prog->setVerbose(true);
		prog->setShaderNames(resourceDirectory + "/simple_vert.glsl", resourceDirectory + "/simple_frag.glsl");
		prog->init();
		prog->addUniform("P");
		prog->addUniform("V");
		prog->addUniform("M");
		prog->addUniform("MatAmb");
		prog->addUniform("MatDif");
		prog->addUniform("MatSpec");
		prog->addUniform("MatShine");
		prog->addUniform("lightP");
		prog->addUniform("viewP");
		prog->addAttribute("vertPos");
		prog->addAttribute("vertNor");
		prog->addAttribute("vertUV");

		floorProg = make_shared<Program>();
		floorProg->setVerbose(true);
		floorProg->setShaderNames(resourceDirectory + "/floor_vert.glsl", resourceDirectory + "/floor_frag.glsl");
		floorProg->init();
		floorProg->addUniform("P");
		floorProg->addUniform("V");
		floorProg->addUniform("M");
		
		floorProg->addAttribute("vertPos");
		floorProg->addAttribute("vertNor");
		floorProg->addAttribute("vertTex");
		floorProg->addAttribute("textureSampler");

		catProg = make_shared<Program>();
		catProg->setVerbose(true);
		catProg->setShaderNames(resourceDirectory + "/floor_vert.glsl", resourceDirectory + "/floor_frag.glsl");
		catProg->init();
		catProg->addUniform("P");
		catProg->addUniform("V");
		catProg->addUniform("M");
		catProg->addAttribute("vertPos");
		catProg->addAttribute("vertNor");
		catProg->addAttribute("vertTex");
		catProg->addAttribute("textureSampler");
	}

	void initGeom(const std::string& objectDirectory)
	{

		// Initialize meshs.
		head = make_shared<Shape>();
		head->loadMesh(objectDirectory + "/robot/head.obj");
		head->init();

		body = make_shared<Shape>();
		body->loadMesh(objectDirectory + "/robot/body.obj");
		body->init();

		arm_upper = make_shared<Shape>();
		arm_upper->loadMesh(objectDirectory + "/robot/arm_upper.obj");
		arm_upper->init();

		arm_lower = make_shared<Shape>();
		arm_lower->loadMesh(objectDirectory + "/robot/arm_lower.obj");
		arm_lower->init();

		leg_upper = make_shared<Shape>();
		leg_upper->loadMesh(objectDirectory + "/robot/leg_upper.obj");
		leg_upper->init();

		leg_lower = make_shared<Shape>();
		leg_lower->loadMesh(objectDirectory + "/robot/leg_lower.obj");
		leg_lower->init();

		floor = make_shared<Shape>();
		floor->loadMesh(objectDirectory + "/rockwall.obj");
		floor->init();
		floor_texture.setFilename(objectDirectory + "/rockwall_texture.jpg");
		floor_texture.init();
		floor_texture.setUnit(0);
		floor_texture.setWrapModes(GL_REPEAT, GL_REPEAT);

		cat = make_shared<Shape>();
		cat->loadMesh(objectDirectory + "/example_mesh_1.obj");
		cat->init();
		//cat_texture.setFilename(objectDirectory + "/cat_color.jpg");
		//cat_texture.init();
		//cat_texture.setUnit(0);
		//cat_texture.setWrapModes(GL_REPEAT, GL_REPEAT);


		// init multi shape object
		vector<tinyobj::shape_t> TOshapes;
		vector<tinyobj::material_t> objMaterials;
		string errStr;
		// loads object into shapes and stores in TOshapes
		bool rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (objectDirectory + "/cartoon_flower.obj").c_str());
		if (!rc) {
			cerr << errStr << endl;
		}
		else {
			for (int i = 0; i < TOshapes.size(); i++) {
				// Initialize each mesh.
				shared_ptr<Shape> s;
				s = make_shared<Shape>();
				s->loadMultiMesh(TOshapes[i]);
				s->init();

				flowerMesh.push_back(s);
			}
		}
	}

	void SetMaterial(int i) {

		prog->bind();
		switch (i) {
		case 0: // color by normal
			glUniform3f(prog->getUniform("MatAmb"), -1, -1, -1);
			glUniform3f(prog->getUniform("MatDif"), 0.0, 0.16, 0.9);
			glUniform3f(prog->getUniform("MatSpec"), 0.14, 0.2, 0.8);
			glUniform1f(prog->getUniform("MatShine"), 120.0);
			break;
		case 1: // pink
			glUniform3f(prog->getUniform("MatAmb"), -1, -1, -1);
			glUniform3f(prog->getUniform("MatDif"), 0.0, 0.16, 0.9);
			glUniform3f(prog->getUniform("MatSpec"), 0.14, 0.2, 0.8);
			glUniform1f(prog->getUniform("MatShine"), 120.0);
			break;
		case 2: //brass
			glUniform3f(prog->getUniform("MatAmb"), 0.3294, 0.2235, 0.02745);
			glUniform3f(prog->getUniform("MatDif"), 0.7804, 0.5686, 0.11373);
			glUniform3f(prog->getUniform("MatSpec"), 0.9922, 0.941176, 0.80784);
			glUniform1f(prog->getUniform("MatShine"), 27.9);
			break;
		case 3: // green
			glUniform3f(prog->getUniform("MatAmb"), 0.0, 0.2, 0.0);
			glUniform3f(prog->getUniform("MatDif"), 0.0, 0.8, 0.0);
			glUniform3f(prog->getUniform("MatSpec"), 0.0, 0.5, 0.0);
			glUniform1f(prog->getUniform("MatShine"), 30.0);
			break;
		}
	}

	void drawCat(shared_ptr<MatrixStack> Model)
	{
		Model->pushMatrix();
		Model->translate(vec3(4.5f, -3.0f, 4.0f));
		Model->scale(2);
		Model->rotate(radians(180.0f), vec3(0, 1, 1));
		Model->rotate(radians(120.0f), vec3(0, 0, 1));

		//cat_texture.bind(catProg->getUniform("rockwallTexture"));
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));

		cat->draw(catProg);
		//cat_texture.unbind();

		Model->popMatrix();
	}

	void drawFloor(shared_ptr<MatrixStack> Model)
	{
		Model->pushMatrix();
		Model->translate(vec3(0, -6, 0));
		Model->scale(0.1);

		floor_texture.bind(floorProg->getUniform("rockwallTexture"));
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));

		floor->draw(floorProg);
		floor_texture.unbind();

		Model->popMatrix();
	}

	void drawFlower(shared_ptr<MatrixStack> Model)
	{
		Model->pushMatrix();
		Model->translate(vec3(-0.4, -1, 0));
		Model->rotate(deg90, vec3(0, 0, 1));
		Model->rotate(deg45, vec3(0, 1, 0));
		Model->scale(7);
		SetMaterial(1);
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));
		// draw each shape in flower
		for (int i = 0; i < flowerMesh.size(); i++) {
			flowerMesh[i]->draw(prog);
		}
		Model->popMatrix();
	}

	void drawRobot(shared_ptr<MatrixStack> Model)
	{
		Model->pushMatrix();
		//global rotate (the whole scene)
		//Model->rotate(gRot, vec3(0, 1, 0));

		// draw body
		Model->pushMatrix(); // body push
		//Model->translate(vec3(0, 0.5, -9 + gTrans));
		Model->rotate(deg45, vec3(0, 1, 0));
		SetMaterial(1);
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));
		body->draw(prog);

		// draw head
		Model->pushMatrix(); // head push
		Model->translate(vec3(0, 1.5, 0));
		Model->rotate(rotAmt, vec3(0, 1, 0));
		SetMaterial(0);
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));
		head->draw(prog);
		Model->popMatrix(); // head pop

		// draw right arm
		// upper
		Model->pushMatrix(); // right upper arm push
		Model->translate(vec3(0, 1.5, -1.5));
		Model->rotate(rotAmt, vec3(0, 0, -1));
		SetMaterial(3);
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));
		arm_upper->draw(prog);

		// lower
		Model->pushMatrix(); // right lower arm push
		Model->translate(vec3(0, -1.5, 0));
		Model->rotate(deg45, vec3(0, 0, -1));
		SetMaterial(3);
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));
		arm_lower->draw(prog);

		drawFlower(Model);

		Model->popMatrix(); // right lower arm pop
		Model->popMatrix(); // right upper arm pop

		// draw left arm
		// upper
		Model->pushMatrix(); // left upper arm push
		Model->translate(vec3(0, 1.5, 1.5));
		Model->rotate(rotAmt, vec3(0, 0, 1));
		SetMaterial(3);
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));
		arm_upper->draw(prog);

		// lower
		Model->pushMatrix(); // left lower arm push
		Model->translate(vec3(0, -1.5, 0));
		SetMaterial(3);
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));
		arm_lower->draw(prog);
		Model->popMatrix(); // left lower arm pop
		Model->popMatrix(); // left upper arm pop


		// draw right leg
		// upper
		Model->pushMatrix(); // right upper leg push
		Model->translate(vec3(0, -1.5, -0.5));
		if (animate) Model->rotate(rotAmt, vec3(0, 0, 1));
		else Model->rotate(0, vec3(0, 0, 1));
		SetMaterial(2);
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));
		leg_upper->draw(prog);

		Model->pushMatrix(); // right lower leg push
		Model->translate(vec3(0, -1.5, 0));
		if (animate) Model->rotate(deg45, vec3(0, 0, 1));
		else Model->rotate(0, vec3(0, 0, 1));
		SetMaterial(2);
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));
		leg_upper->draw(prog);
		Model->popMatrix(); // right lower leg pop
		Model->popMatrix(); // right upper leg pop

		// draw left leg
		// upper
		Model->pushMatrix(); // left upper leg push
		Model->translate(vec3(0, -1.5, 0.5));
		if (animate) Model->rotate(rotAmt, vec3(0, 0, -1));
		else Model->rotate(0, vec3(0, 0, 1));
		SetMaterial(2);
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));
		leg_upper->draw(prog);

		Model->pushMatrix(); // left lower leg push
		Model->translate(vec3(0, -1.5, 0));
		if (animate) Model->rotate(deg45, vec3(0, 0, 1));
		else Model->rotate(0, vec3(0, 0, 1));
		SetMaterial(2);
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));
		leg_upper->draw(prog);
		Model->popMatrix(); // left lower leg pop
		Model->popMatrix(); // left upper leg pop
		Model->popMatrix(); // body pop		
		Model->popMatrix();

		if (animate)
		{
			rotAmt += rotInc;

			if (rotAmt >= deg45)
			{
				rotAmt = deg45;
				rotInc *= -1;
			}

			else if (rotAmt <= -deg45)
			{
				rotAmt = -deg45;
				rotInc *= -1;
			}
		}
	}

	void render()
	{
		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		glViewport(0, 0, width, height);

		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Use the matrix stack for Lab 6
		float aspect = width / (float)height;

		// Create the matrix stacks - please leave these alone for now
		auto Projection = make_shared<MatrixStack>();
		auto View = make_shared<MatrixStack>();
		auto Model = make_shared<MatrixStack>();

		// Apply perspective projection.
		Projection->pushMatrix();
		Projection->perspective(45.0f, width / (float)height, 0.01f, 100.0f);

		
		// View is identity - for now
		View->pushMatrix();

		camera.setViewPtr(View);
		camera.look();

		// cat
		catProg->bind();
		glUniformMatrix4fv(catProg->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		glUniformMatrix4fv(catProg->getUniform("V"), 1, GL_FALSE, value_ptr(View->topMatrix()));
		drawCat(Model);

		catProg->unbind();


		// floor
		floorProg->bind();
		glUniformMatrix4fv(floorProg->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		glUniformMatrix4fv(floorProg->getUniform("V"), 1, GL_FALSE, value_ptr(View->topMatrix()));
		drawFloor(Model);

		floorProg->unbind();
		

		// Draw a stack of cubes with indiviudal transforms
		prog->bind();
		
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, value_ptr(View->topMatrix()));
		glUniform3f(prog->getUniform("lightP"), lightTrans + 2, 3, 5);
		
		drawRobot(Model);

		prog->unbind();

		// Pop matrix stacks.
		Projection->popMatrix();
		View->popMatrix();

	}
};

int main(int argc, char* argv[])
{
	// Where the resources are loaded from
	std::string resourceDir = "../resources";
	std::string objectDir = "../objects";

	if (argc >= 2)
	{
		resourceDir = argv[1];
	}

	Application* application = new Application();

	// Your main will always include a similar set up to establish your window
	// and GL context, etc.

	WindowManager* windowManager = new WindowManager();
	windowManager->init(640, 480);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	// GLFW cursor mode
	glfwSetInputMode(windowManager->getHandle(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// This is the code that will likely change program to program as you
	// may need to initialize or set up different data and state

	application->init(resourceDir);
	application->initGeom(objectDir);

	// Loop until the user closes the window.
	while (!glfwWindowShouldClose(windowManager->getHandle()))
	{
		// Render scene.
		application->render();

		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();
	}

	// Quit program.
	windowManager->shutdown();
	delete application;
	return 0;
}
