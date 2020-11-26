/*
 * Lab 5 base code (could also be used for Program 2)
 * includes modifications to shape and initGeom in preparation to load
 * multi shape objects 
 * CPE 471 Cal Poly Z. Wood + S. Sueda + I. Dunn
 */

#include <iostream>
#include <glad/glad.h>

#include "GLSL.h"
#include "Program.h"
#include "Shape.h"
#include "Texture.h"
#include "MatrixStack.h"
#include "WindowManager.h"
#include "stb_image.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader/tiny_obj_loader.h>

// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace glm;

#define NUM_TEXTURES 21

class Application : public EventCallbacks {

public:

	WindowManager * windowManager = nullptr;

	// Our shader program
	std::shared_ptr<Program> prog;
    std::shared_ptr<Program> textProg;
	std::shared_ptr<Program> cubeProg;
    
    // Shape to be used (from  file) - modify to support multiple
    shared_ptr<Shape> mesh;

    // all the shapes
    vector<shared_ptr<Shape>> plane;
    vector<shared_ptr<Shape>> coin;
    vector<shared_ptr<Shape>> skybox;
    
    // Textures to be used
    shared_ptr<Texture> texture0;
    shared_ptr<Texture> texture1;
    shared_ptr<Texture> texture2;
	vector<shared_ptr<Texture>> plane_texture;

	// keep track of which coins have been retrieved
	uint8_t coin_flags = 0;

	// Contains vertex information for OpenGL
	GLuint VertexArrayID;

	// Data necessary to give our triangle to OpenGL
	GLuint VertexBufferID;

	//example data that might be useful when trying to compute bounds on multi-shape
	vec3 gMin;

	//animation data
	float sTheta = 2.0;
    float sPhi = 0;
    float sAnimation = 0;
    float camSensitivity = 0.001;
	float walkSensitivity = 0.1;

	// camera stuff
	vec3 eye = vec3(20, 2, -45);
	vec3 up = vec3(0, 1, 0);
	vec3 direction = vec3(cos(sPhi) * cos(sTheta), sin(sPhi), cos(sPhi) * cos((3.141593 / 2.0) - sTheta));
	vec3 center = eye + direction;

    double posX, posY = 0;

	// walking flags
	bool w = false;
	bool a = false;
	bool s = false;
	bool d = false;
	bool space = false;
	bool shift = false;

	int x_pos, z_pos = 0;

    vector<string> plane_filenames{ "574B138E_c.png", "41A41EE3_c.png", "1FAAE88D_c.png", "359289F2_c.png", 
		"6E3A21B_c.png", "6B1A233B_c.png", "6B2D96F_c.png", "12436720_c.png", 
		"1FAAE88D_c.png", "1FAAE88D_c.png", "275F399C_c.png", "4020CDFE_c.png", 
		"1B46C8C_c.png", "6C631877_c.png", "3D49A9D5_c.png", "C1DF883_c.png", 
		"3F485258_c.png", "10E99677_c.png", "359289F2_c.png", "6B2D96F_c.png", "12436720_c.png" };

	vector<vec3> coin_loc{ vec3(-20, 17, -2), vec3(-3, 5, -11), vec3(23, 6, -27), vec3(14, 10, 28),
	                       vec3(21, 0, 8),    vec3(-1, 23, 22), vec3(-22, 7, 1),  vec3(38, 6, 11)  };
    
    void scrollCallback(GLFWwindow* window, double deltaX, double deltaY) {
        cout << "use two finger mouse scroll" << endl;
    }

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		if (key == GLFW_KEY_I && action == GLFW_PRESS) {
			x_pos += 1;
			cout << x_pos << " " << z_pos << endl;
		}
		if (key == GLFW_KEY_K && action == GLFW_PRESS) {
			x_pos -= 1;
			cout << x_pos << " " << z_pos << endl;
		}
		if (key == GLFW_KEY_L && action == GLFW_PRESS) {
			z_pos += 1;
			cout << x_pos << " " << z_pos << endl;
		}
		if (key == GLFW_KEY_J && action == GLFW_PRESS) {
			z_pos -= 1;
			cout << x_pos << " " << z_pos << endl;
		}
        if (key == GLFW_KEY_W && action == GLFW_PRESS) {
			w = true;
		}
		if (key == GLFW_KEY_W && action == GLFW_RELEASE) {
			w = false;
		}

		if (key == GLFW_KEY_S && action == GLFW_PRESS) {
			s = true;
		}
		if (key == GLFW_KEY_S && action == GLFW_RELEASE) {
			s = false;
		}

		if (key == GLFW_KEY_A && action == GLFW_PRESS) {
			a = true;
		}
		if (key == GLFW_KEY_A && action == GLFW_RELEASE) {
			a = false;
		}

		if (key == GLFW_KEY_D && action == GLFW_PRESS) {
			d = true;
		}
		if (key == GLFW_KEY_D && action == GLFW_RELEASE) {
			d = false;
		}

		if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
			space = true;
		}
		if (key == GLFW_KEY_SPACE && action == GLFW_RELEASE) {
			space = false;
		}

		if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_PRESS) {
			shift = true;
		}
		if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_RELEASE) {
			shift = false;
		}
        
		if (key == GLFW_KEY_Z && action == GLFW_PRESS) {
			glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		}
		if (key == GLFW_KEY_Z && action == GLFW_RELEASE) {
			glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		}

	}

	void mouseCallback(GLFWwindow *window, int button, int action, int mods)
	{
        glfwGetCursorPos(window, &posX, &posY);

		if (action == GLFW_PRESS)
		{
			 cout << "Pos X " << posX <<  " Pos Y " << posY << endl;
		}
	}

	void resizeCallback(GLFWwindow *window, int width, int height)
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

		// init the GLSL program for the skybox
		cubeProg = make_shared<Program>();
		cubeProg->setVerbose(true);
		cubeProg->setShaderNames(resourceDirectory + "/cube_vert.glsl", resourceDirectory + "/cube_frag.glsl");
		cubeProg->init();
		cubeProg->addUniform("P");
		cubeProg->addUniform("V");
		cubeProg->addUniform("M");
		cubeProg->addUniform("skybox");
		cubeProg->addAttribute("vertPos");
        cubeProg->addAttribute("vertNor");

		// Initialize the GLSL program -- this one is for textured stuff.
		textProg = make_shared<Program>();
		textProg->setVerbose(true);
		textProg->setShaderNames(resourceDirectory + "/tex_vert.glsl", resourceDirectory + "/tex_frag.glsl");
		textProg->init();
		textProg->addUniform("P");
		textProg->addUniform("V");
		textProg->addUniform("M");
        textProg->addUniform("lightP");
        textProg->addUniform("shine");
        textProg->addUniform("Texture0");
		textProg->addAttribute("vertPos");
		textProg->addAttribute("vertNor");
        textProg->addAttribute("vertTex");
        
        // Initialize the GLSL program -- this one is for NON-textured stuff.
		prog = make_shared<Program>();
		prog->setVerbose(true);
		prog->setShaderNames(resourceDirectory + "/simple_vert.glsl", resourceDirectory + "/simple_frag.glsl");
		prog->init();
		prog->addUniform("P");
		prog->addUniform("V");
		prog->addUniform("M");
        prog->addUniform("lightP");
        prog->addUniform("MatAmb");
		prog->addUniform("MatDif");
		prog->addUniform("MatSpec");
        prog->addUniform("shine");
		prog->addAttribute("vertPos");
		prog->addAttribute("vertNor");
        prog->addAttribute("vertTex");
	}

	void initGeom(const std::string& resourceDirectory)
	{

		//EXAMPLE set up to read one shape from one obj file - convert to read several
		// Initialize mesh
		// Load geometry
 		// Some obj files contain material information.We'll ignore them for this assignment.
 		vector<tinyobj::shape_t> TOshapes;
 		vector<tinyobj::material_t> objMaterials;
 		string errStr;
		//load in the mesh and make the shape(s)
        
        bool rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (resourceDirectory + "/bobomb.obj").c_str());
		if (!rc) {
			cerr << errStr << endl;
		} else {
		    for (int i = 0; i < TOshapes.size(); i++) {
				mesh = make_shared<Shape>();
				mesh->createShape(TOshapes[i]);
				mesh->measure();
				mesh->init();

				plane.push_back(mesh);
			}
		}

		rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (resourceDirectory + "/coin.obj").c_str());
		if (!rc) {
			cerr << errStr << endl;
		}
		else {
			for (int i = 0; i < TOshapes.size(); i++) {
				mesh = make_shared<Shape>();
				mesh->createShape(TOshapes[i]);
				mesh->measure();
				mesh->init();

				coin.push_back(mesh);
			}
		}
        
        // AND the skybox.
        rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (resourceDirectory + "/cube.obj").c_str());
		if (!rc) {
			cerr << errStr << endl;
		} else {
		    for (int i = 0; i < TOshapes.size(); i++) {
				mesh = make_shared<Shape>();
				mesh->createShape(TOshapes[i]);
				mesh->measure();
				mesh->init();

				skybox.push_back(mesh);
			}
		}   
	}
    
    void initTex(const std::string& resourceDirectory) {
        texture0 = make_shared<Texture>();
        texture0->setFilename(resourceDirectory + "/crate.jpg");
        texture0->init();
        texture0->setUnit(0);
        texture0->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
        
        texture1 = make_shared<Texture>();
        texture1->setFilename(resourceDirectory + "/world.jpg");
        texture1->init();
        texture1->setUnit(1);
        texture1->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
        
        texture2 = make_shared<Texture>();
        texture2->setFilename(resourceDirectory + "/grass.jpg");
        texture2->init();
        texture2->setUnit(2);
        texture2->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

		for (int i = 0; i < NUM_TEXTURES; i++) {
			plane_texture.push_back(make_shared<Texture>());
			plane_texture[i]->setFilename(resourceDirectory + "/Textures/" + plane_filenames[i]);
			plane_texture[i]->init();
			plane_texture[i]->setUnit(i+3);
			plane_texture[i]->setWrapModes(GL_REPEAT, GL_REPEAT);
		}
    }
    
    unsigned int createSky(string dir, vector<string> faces) {
        unsigned int textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
        
        int width, height, nrChannels;
        stbi_set_flip_vertically_on_load(false);
        for(GLuint i = 0; i < faces.size(); i++) {
            unsigned char *data =
            stbi_load((dir+faces[i]).c_str(), &width, &height, &nrChannels, 0);
            if (data) {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            } else {
                cout << "failed to load: " << (dir+faces[i]).c_str() << endl;
            }   
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        cout << " creating cube map any errors : " << glGetError() << endl;   return textureID;
    }

	void setModel(std::shared_ptr<Program> prog, std::shared_ptr<MatrixStack>M) {
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix()));
   }

	void updateCamera() {
		direction.x = cos(sPhi) * cos(sTheta);
		direction.y = sin(sPhi);
		direction.z = cos(sPhi) * cos((3.141593 / 2.0) - sTheta);
		center = eye + direction;
	}

	void updateMouse(int width, int height, GLFWwindow* window) {
		glfwGetCursorPos(window, &posX, &posY);
		glfwSetCursorPos(window, width / 2, height / 2);
		sTheta -= (width / 2 - posX) * camSensitivity;
		if (sTheta > 6.28319) sTheta = sTheta - 6.28319;
		if (sTheta < 0) sTheta = sTheta + 6.28319;
		sPhi += (height / 2 - posY) * camSensitivity;
		if (sPhi > (1.39626)) sPhi = 1.39626;
		if (sPhi < -1.39626) sPhi = -1.39626;
	}

	void setSkybox(shared_ptr<MatrixStack> Projection, shared_ptr<MatrixStack> View, shared_ptr<MatrixStack> Model) {
		// skybox stuff
		cubeProg->bind();
		glUniformMatrix4fv(cubeProg->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		glDepthFunc(GL_LEQUAL);
		glUniformMatrix4fv(cubeProg->getUniform("V"), 1, GL_FALSE, value_ptr(View->topMatrix()));
		glUniformMatrix4fv(cubeProg->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));
		Model->pushMatrix();
		Model->scale(vec3(1000, 1000, 1000));
		setModel(cubeProg, Model);
        for (int i = 0; i < skybox.size(); i++) {
            skybox[i]->draw(cubeProg);
        }
		Model->popMatrix();
		glDepthFunc(GL_LESS);
		cubeProg->unbind();
	}

	void move() {
		if (w) {
			eye += walkSensitivity * direction;
		}
		if (a) {
			eye -= walkSensitivity * normalize(cross(direction, up));
		}
		if (s) {
			eye -= walkSensitivity * direction;
		}
		if (d) {
			eye += walkSensitivity * normalize(cross(direction, up));
		}
		if (space) {
			eye += walkSensitivity * up;
		}
		if (shift) {
			eye -= walkSensitivity * up;
		}
	}

	void checkCoins() {
		for (int i = 0; i < coin_loc.size(); i++) {
			if (distance(eye, coin_loc[i]) < 2) {
				coin_flags |= 1 << i;
			}
		}
	}

	void drawCoins(shared_ptr<MatrixStack> Model) {
		// print out only the coins that have not yet been retrieved
		Model->pushMatrix();
		Model->loadIdentity();

		for (int i = 0; i < coin_loc.size(); i++) {
			if (!(coin_flags & 1 << i)) {
				// star on the island
				Model->pushMatrix();
					SetMaterial(2);
					Model->translate(coin_loc[i]);
					Model->rotate(sAnimation, vec3(0, 1, 0));
					Model->scale(vec3(0.05, 0.05, 0.05));
					setModel(prog, Model);
					for (int i = 0; i < coin.size(); i++) {
						coin[i]->draw(prog);
					}
				Model->popMatrix();
			}
		}

		Model->popMatrix();
	}

	void render(GLFWwindow *window) {
		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		glViewport(0, 0, width, height);

		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Use the matrix stack for Lab 6
		float aspect = width/(float)height;

		// Create the matrix stacks - please leave these alone for now
		auto Projection = make_shared<MatrixStack>();
		auto View = make_shared<MatrixStack>();
		auto Model = make_shared<MatrixStack>();

		// Apply perspective projection.
		Projection->pushMatrix();
		Projection->perspective(45.0f, aspect, 0.01f, 1000.0f);

		// mouse stuff
		move();
		updateCamera();
		updateMouse(width, height, window);

		// update the coins based on current position
		checkCoins();

		// update the view
		View->pushMatrix();
			View->loadIdentity();
			View->lookAt(eye, center, up);

		// Draw all the non-textured stuff first
		prog->bind();
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, value_ptr(View->topMatrix()));
        glUniform3f(prog->getUniform("lightP"), eye.x, eye.y, eye.z);

		// draw coins
		drawCoins(Model);

		prog->unbind();
        
        
        // Now draw all the textured stuff
		textProg->bind();
		glUniformMatrix4fv(textProg->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		glUniformMatrix4fv(textProg->getUniform("V"), 1, GL_FALSE, value_ptr(View->topMatrix()));
        glUniform3f(textProg->getUniform("lightP"), 0, 100, 0);
        glUniform1f(textProg->getUniform("shine"), 120.0);

		/* draw the plane */
		Model->pushMatrix();
			Model->translate(vec3(0, -0.8, 0));
			Model->rotate(0.4, vec3(0, 1, 0));
			Model->scale(vec3(5, 5, 5));
			setModel(textProg, Model);
			for (int i = 0; i < plane.size(); i++) {
				plane_texture[i]->bind(textProg->getUniform("Texture0"));
				plane[i]->draw(textProg);
				plane_texture[i]->unbind();
			}
		Model->popMatrix();
       
		textProg->unbind();

		setSkybox(Projection, View, Model);
        
        sAnimation = sin(glfwGetTime());

		// Pop matrix stacks.
		Projection->popMatrix();
		View->popMatrix();

	}
    
    void SetMaterial(int i) {
        switch(i) {
            case 0: // shiny blue plastic
                glUniform3f(prog->getUniform("MatAmb"), 0.02, 0.04, 0.2);
                glUniform3f(prog->getUniform("MatDif"), 0.0, 0.16, 0.9);
                glUniform3f(prog->getUniform("MatSpec"), 0.14, 0.2, 0.8);
                glUniform1f(prog->getUniform("shine"), 120.0);
                break;
            case 1: // flat grey
                glUniform3f(prog->getUniform("MatAmb"), 0.13, 0.13, 0.14);
                glUniform3f(prog->getUniform("MatDif"), 0.3, 0.3, 0.4);
                glUniform3f(prog->getUniform("MatSpec"), 0.3, 0.3, 0.4);
                glUniform1f(prog->getUniform("shine"), 4.0);
                break;
            case 2: // brass
                glUniform3f(prog->getUniform("MatAmb"), 0.3294, 0.2235, 0.02745);
                glUniform3f(prog->getUniform("MatDif"), 0.7804, 0.5686, 0.11373);
                glUniform3f(prog->getUniform("MatSpec"), 0.9922, 0.941176, 0.80784);
                glUniform1f(prog->getUniform("shine"), 27.9);
                break;
            case 3: // grass
                glUniform3f(prog->getUniform("MatAmb"), 0.0, 0.4, 0.0);
                glUniform3f(prog->getUniform("MatDif"), 0.0, 0.4, 0.0);
                glUniform3f(prog->getUniform("MatSpec"), 0.0, 0.4, 0.0);
                glUniform1f(prog->getUniform("shine"), 4.0);
                break;
            case 4: // gold
                glUniform3f(prog->getUniform("MatAmb"), 0.24725, 0.1995, 0.0745);
                glUniform3f(prog->getUniform("MatDif"), 0.75164, 0.60648, 0.22648);
                glUniform3f(prog->getUniform("MatSpec"), 0.628281, 0.555802, 0.366065);
                glUniform1f(prog->getUniform("shine"), 50.0);
                break;
        }
    }
};

int main(int argc, char *argv[])
{
	// Where the resources are loaded from
	std::string resourceDir = "../resources";

	vector<std::string> faces{
		"/Meadow/posx.jpg",
		"/Meadow/negx.jpg",
		"/Meadow/posy.jpg",
		"/Meadow/negy.jpg",
		"/Meadow/posz.jpg",
		"/Meadow/negz.jpg"
	};

	int res;

	if (argc >= 2)
	{
		resourceDir = argv[1];
	}

	Application *application = new Application();

	// Your main will always include a similar set up to establish your window
	// and GL context, etc.

	WindowManager *windowManager = new WindowManager();
	windowManager->init(640, 480);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	// This is the code that will likely change program to program as you
	// may need to initialize or set up different data and state

	application->init(resourceDir);
	application->initGeom(resourceDir);
    application->initTex(resourceDir);
    res = application->createSky(resourceDir, faces);
	ShowCursor(false);

	// Loop until the user closes the window.
	while (! glfwWindowShouldClose(windowManager->getHandle()))
	{
		// Render scene.
		application->render(windowManager->getHandle());

		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();
	}

	// Quit program.
	windowManager->shutdown();
	return 0;
}
