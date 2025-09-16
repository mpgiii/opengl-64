/*
 * CPE 471 Final Project -- Mario 64 Coin Collector
 * Dr. Zoe Wood
 * Michael Georgariou
 */

#include <iostream>
#include <glad/glad.h>
#include <string>
#include <iostream>
#include <sstream>


#include "GLSL.h"
#include "Program.h"
#include "Shape.h"
#include "Texture.h"
#include "MatrixStack.h"
#include "WindowManager.h"
#include "stb_image.h"
#include "particleSys.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader/tiny_obj_loader.h>

// stuff for on screen text
#include "ft2build.h"
#include "freetype/freetype.h"

// stuff for audio
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

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
    std::shared_ptr<Program> texProg;
	std::shared_ptr<Program> textProg;
	std::shared_ptr<Program> cubeProg;
	std::shared_ptr<Program> partProg;

    // Shape to be used (from file) - modify to support multiple
    shared_ptr<Shape> mesh;

    // all the shapes
    vector<shared_ptr<Shape>> plane;
    vector<shared_ptr<Shape>> coin;
    vector<shared_ptr<Shape>> skybox;

    // Textures to be used
	vector<shared_ptr<Texture>> plane_texture;
	shared_ptr<Texture> part_texture;

	// keep track of which coins have been retrieved
	uint8_t coin_flags = 0;

	// Contains vertex information for OpenGL
	GLuint VertexArrayID;

	// Data necessary to give our triangle to OpenGL
	GLuint VertexBufferID;

	//animation data
	float sTheta = 2.0;
    float sPhi = 0;
    float sAnimation = 0;
    float camSensitivity = 0.001;
	float walkSensitivity = 0.1;

	// camera stuff
	vec3 eye = vec3(20, 2, -45); // starting position
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

    vector<string> plane_filenames{ "574B138E_c.png", "41A41EE3_c.png", "1FAAE88D_c.png", "359289F2_c.png",
		"6E3A21B_c.png", "6B1A233B_c.png", "6B2D96F_c.png", "12436720_c.png",
		"1FAAE88D_c.png", "1FAAE88D_c.png", "275F399C_c.png", "4020CDFE_c.png",
		"1B46C8C_c.png", "6C631877_c.png", "3D49A9D5_c.png", "C1DF883_c.png",
		"3F485258_c.png", "10E99677_c.png", "359289F2_c.png", "6B2D96F_c.png", "12436720_c.png" };

	vector<vec3> coin_loc{ vec3(-20, 17, -2), vec3(-3, 5, -11), vec3(23, 6, -27), vec3(14, 10, 28),
	                       vec3(21, 0, 8), vec3(-1, 23, 22), vec3(-22, 7, 1), vec3(38, 6, 11)  };

	// the partricle system itself
	vector<particleSys*> partSystems;

	// audio engine
	bool audioInitialized = false;
	Mix_Music* backgroundMusic = nullptr;
	Mix_Chunk* coinSound = nullptr;
	float volume = 0.5;

	// text stuff now
	struct Character {
		unsigned int TextureID;  // ID handle of the glyph texture
		glm::ivec2   Size;       // Size of glyph
		glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
		unsigned int Advance;    // Offset to advance to next glyph
	};

	std::map<char, Character> Characters;

	unsigned int VAO, VBO;

	vec3 text_color = vec3(1.0, 0.0, 0.0);

    void scrollCallback(GLFWwindow* window, double deltaX, double deltaY) {
		return;
    }

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}

		if (key == GLFW_KEY_R && action == GLFW_PRESS) {
			// when R is pressed, reset everything
				//animation data
			sTheta = 2.0;
			sPhi = 0;

			// camera stuff
			eye = vec3(20, 2, -45); // starting position
			center = eye + direction;

			for (int i = 0; i < partSystems.size(); i++) {
				// if this coin has already been retrieved, reset its particles
				if (coin_flags & 1 << i) {
					partSystems[i]->reSet();
					partSystems[i]->update();
				}
			}

			coin_flags = 0;
		}

		if (key == GLFW_KEY_UP && action == GLFW_PRESS) {
			// turn up the volume by 10 %
			volume = volume + 0.1;
			if (volume > 1) volume = 1.0;

			if (audioInitialized) {
				Mix_VolumeMusic((int)(volume * MIX_MAX_VOLUME));
			}
		}
		if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) {
			// turn down the volume by 10 %
			volume = volume - 0.1;
			if (volume < 0) volume = 0;

			if (audioInitialized) {
				Mix_VolumeMusic((int)(volume * MIX_MAX_VOLUME));
			}
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

	void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
		return;
	}

	void init(const std::string& resourceDirectory)
	{
		GLSL::checkVersion();

		// Set background color.
		glClearColor(.12f, .34f, .56f, 1.0f);
		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);
		CHECKED_GL_CALL(glEnable(GL_BLEND));
		CHECKED_GL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
		CHECKED_GL_CALL(glPointSize(24.0f));

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
		texProg = make_shared<Program>();
		texProg->setVerbose(true);
		texProg->setShaderNames(resourceDirectory + "/tex_vert.glsl", resourceDirectory + "/tex_frag.glsl");
		texProg->init();
		texProg->addUniform("P");
		texProg->addUniform("V");
		texProg->addUniform("M");
        texProg->addUniform("lightP");
        texProg->addUniform("shine");
        texProg->addUniform("Texture0");
		texProg->addAttribute("vertPos");
		texProg->addAttribute("vertNor");
        texProg->addAttribute("vertTex");

		// Initialize the GLSL program -- this one is for textured stuff.
		textProg = make_shared<Program>();
		textProg->setVerbose(true);
		textProg->setShaderNames(resourceDirectory + "/text_vert.glsl", resourceDirectory + "/text_frag.glsl");
		textProg->init();
		textProg->addAttribute("vertex");
		textProg->addUniform("projection");
		textProg->addUniform("text");
		textProg->addUniform("textColor");

		// Initialize the GLSL program.
		partProg = make_shared<Program>();
		partProg->setVerbose(true);
		partProg->setShaderNames(resourceDirectory + "/lab10_vert.glsl", resourceDirectory + "/lab10_frag.glsl");
		partProg->init();
		partProg->addUniform("P");
		partProg->addUniform("M");
		partProg->addUniform("V");
		partProg->addAttribute("pColor");
		partProg->addUniform("alphaTexture");
		partProg->addAttribute("vertPos");

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

	void initPart() {
		for (int i = 0; i < coin_loc.size(); i++) {
			partSystems.push_back(new particleSys(coin_loc[i]));
			partSystems[i]->gpuSetup();
		}
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
		// for particles
		part_texture = make_shared<Texture>();
		part_texture->setFilename(resourceDirectory + "/alpha.bmp");
		part_texture->init();
		part_texture->setUnit(0);
		part_texture->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

		// for world
		for (int i = 0; i < NUM_TEXTURES; i++) {
			plane_texture.push_back(make_shared<Texture>());
			plane_texture[i]->setFilename(resourceDirectory + "/textures/" + plane_filenames[i]);
			plane_texture[i]->init();
			plane_texture[i]->setUnit(i+1);
			plane_texture[i]->setWrapModes(GL_REPEAT, GL_REPEAT);
		}
    }

	int initText() {
		FT_Library ft;
		if (FT_Init_FreeType(&ft)) {
			std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
			return -1;
		}

		FT_Face face;
		if (FT_New_Face(ft, "../fonts/Mario64.ttf", 0, &face)) {
			std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
			return -1;
		}

		// set width to 0 and height to 48
		FT_Set_Pixel_Sizes(face, 0, 48);

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction

		for (unsigned char c = 0; c < 128; c++) {
			// load character glyph
			if (FT_Load_Char(face, c, FT_LOAD_RENDER))
			{
				std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
				continue;
			}
			// generate texture
			unsigned int texture;
			glGenTextures(1, &texture);
			glBindTexture(GL_TEXTURE_2D, texture);
			glTexImage2D(
				GL_TEXTURE_2D,
				0,
				GL_RED,
				face->glyph->bitmap.width,
				face->glyph->bitmap.rows,
				0,
				GL_RED,
				GL_UNSIGNED_BYTE,
				face->glyph->bitmap.buffer
			);
			// set texture options
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			// now store character for later use
			Character character = {
				texture,
				glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
				glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
				static_cast<unsigned int>(face->glyph->advance.x)
			};
			Characters.insert(std::pair<char, Character>(c, character));
		}

		// enable blending
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// create a VBO and VAO for rendering the quads
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		FT_Done_Face(face);
		FT_Done_FreeType(ft);

		return 0;
	}

	int initAudio() {
		// Initialize SDL audio subsystem
		if (SDL_Init(SDL_INIT_AUDIO) < 0) {
			std::cout << "SDL could not initialize! SDL Error: " << SDL_GetError() << std::endl;
			return -1;
		}

		// Initialize SDL_mixer
		if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
			std::cout << "SDL_mixer could not initialize! SDL_mixer Error: " << Mix_GetError() << std::endl;
			SDL_Quit();
			return -1;
		}

		audioInitialized = true;

		// Load background music
		// note: this file is not included in the repo because
		// I don't wanna host copyrighted stuff lol
		backgroundMusic = Mix_LoadMUS("../audio/theme.mp3");
		if (backgroundMusic == nullptr) {
			std::cout << "Failed to load background music! SDL_mixer Error: " << Mix_GetError() << std::endl;
		}

		// Load coin sound effect
		coinSound = Mix_LoadWAV("../audio/coin.wav");
		if (coinSound == nullptr) {
			std::cout << "Failed to load coin sound! SDL_mixer Error: " << Mix_GetError() << std::endl;
		}

		// Set initial volume
		Mix_VolumeMusic((int)(volume * MIX_MAX_VOLUME));

		return 0;
	}

	void cleanupAudio() {
		if (audioInitialized) {
			// Stop and free music
			Mix_HaltMusic();
			if (backgroundMusic) {
				Mix_FreeMusic(backgroundMusic);
				backgroundMusic = nullptr;
			}

			// Free sound effects
			if (coinSound) {
				Mix_FreeChunk(coinSound);
				coinSound = nullptr;
			}

			// Close SDL_mixer and SDL
			Mix_CloseAudio();
			SDL_Quit();
			audioInitialized = false;
		}
	}

	void renderText(string text, float x, float y, float scale, vec3 color) {
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);

		mat4 projection = ortho(0.0f, float(width), 0.0f, float(height));

		textProg->bind();
		glUniform3f(textProg->getUniform("textColor"), color.x, color.y, color.z);
		glUniformMatrix4fv(textProg->getUniform("projection"), 1, GL_FALSE, value_ptr(projection));
		glActiveTexture(GL_TEXTURE0);
		glBindVertexArray(VAO);

		// iterate through all characters
		std::string::const_iterator c;
		for (c = text.begin(); c != text.end(); c++)
		{
			Character ch = Characters[*c];

			float xpos = x + ch.Bearing.x * scale;
			float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

			float w = ch.Size.x * scale;
			float h = ch.Size.y * scale;
			// update VBO for each character
			float vertices[6][4] = {
				{ xpos,     ypos + h,   0.0f, 0.0f },
				{ xpos,     ypos,       0.0f, 1.0f },
				{ xpos + w, ypos,       1.0f, 1.0f },

				{ xpos,     ypos + h,   0.0f, 0.0f },
				{ xpos + w, ypos,       1.0f, 1.0f },
				{ xpos + w, ypos + h,   1.0f, 0.0f }
			};
			// render glyph texture over quad
			glBindTexture(GL_TEXTURE_2D, ch.TextureID);
			// update content of VBO memory
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			// render quad
			glDrawArrays(GL_TRIANGLES, 0, 6);
			// now advance cursors for next glyph (note that advance is number of 1/64 pixels)
			x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)
		}
		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, 0);
		textProg->unbind();
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

	int countSetBits(uint8_t n)
	{
		unsigned int count = 0;
		while (n) {
			count += n & 1;
			n >>= 1;
		}
		return count;
	}


	void checkCoins() {
		for (int i = 0; i < coin_loc.size(); i++) {
			// if you are within 2 units of any coin, collect it
			if (distance(eye, coin_loc[i]) < 2) {
				// if we are just now collecting the coin, play the jingle
				// and print out the number of coins collected
				if (!(coin_flags & 1 << i)) {
					if (audioInitialized && coinSound) {
						Mix_PlayChannel(-1, coinSound, 0);
					}
				}
				coin_flags |= 1 << i;
			}
		}
	}

	void drawCoins(shared_ptr<MatrixStack> Model) {
		// print out only the coins that have not yet been retrieved
		Model->pushMatrix();
		Model->loadIdentity();

		// loop through all the coin locations
		for (int i = 0; i < coin_loc.size(); i++) {
			if (!(coin_flags & 1 << i)) {
				Model->pushMatrix();
					// make em gold
					SetMaterial(2);
					Model->translate(coin_loc[i]);
					// make it pretty by rotating it
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

	void drawParticles(shared_ptr<MatrixStack> Projection, shared_ptr<MatrixStack> View, shared_ptr<MatrixStack> Model) {
		partProg->bind();
		part_texture->bind(partProg->getUniform("alphaTexture"));
		CHECKED_GL_CALL(glUniformMatrix4fv(partProg->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix())));
		CHECKED_GL_CALL(glUniformMatrix4fv(partProg->getUniform("V"), 1, GL_FALSE, value_ptr(View->topMatrix())));
		CHECKED_GL_CALL(glUniformMatrix4fv(partProg->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix())));

		for (int i = 0; i < partSystems.size(); i++) {
			// if this coin has already been retrieved, draw particles there
			if (coin_flags & 1 << i) {
				partSystems[i]->setCamera(View->topMatrix());
				partSystems[i]->drawMe(partProg);
				partSystems[i]->update();
			}
		}
		part_texture->unbind();

		partProg->unbind();
	}

	void colorFade() {
		if (text_color.r > 0 && text_color.b == 0) {
			text_color.r = text_color.r - 0.01;
			if (text_color.r < 0) text_color.r = 0;
			text_color.g = text_color.g + 0.01;
			if (text_color.g > 1) text_color.g = 1;
		}
		if (text_color.g > 0 && text_color.r == 0) {
			text_color.g = text_color.g - 0.01;
			if (text_color.g < 0) text_color.g = 0;
			text_color.b = text_color.b + 0.01;
			if (text_color.b > 1) text_color.b = 1;
		}
		if (text_color.b > 0 && text_color.g == 0) {
			text_color.b = text_color.b - 0.01;
			if (text_color.b < 0) text_color.b = 0;
			text_color.r = text_color.r + 0.01;
			if (text_color.r > 1) text_color.r = 1;
		}
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

		Model->pushMatrix();
		Model->loadIdentity();


		// Draw all the non-textured stuff first
		prog->bind();
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, value_ptr(View->topMatrix()));
        glUniform3f(prog->getUniform("lightP"), eye.x, eye.y, eye.z);

		// draw coins
		drawCoins(Model);

		prog->unbind();


        // Now draw all the textured stuff
		texProg->bind();
		glUniformMatrix4fv(texProg->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		glUniformMatrix4fv(texProg->getUniform("V"), 1, GL_FALSE, value_ptr(View->topMatrix()));
        glUniform3f(texProg->getUniform("lightP"), 0, 100, 0);
        glUniform1f(texProg->getUniform("shine"), 120.0);

		/* draw the plane */
		Model->pushMatrix();
			Model->translate(vec3(0, -0.8, 0));
			Model->rotate(0.4, vec3(0, 1, 0));
			Model->scale(vec3(5, 5, 5));
			setModel(texProg, Model);
			for (int i = 0; i < plane.size(); i++) {
				plane_texture[i]->bind(texProg->getUniform("Texture0"));
				plane[i]->draw(texProg);
				plane_texture[i]->unbind();
			}
		Model->popMatrix();

		texProg->unbind();

		setSkybox(Projection, View, Model);

        sAnimation = sin(glfwGetTime());

		// now do the particle stuff
		drawParticles(Projection, View, Model);

		// Pop matrix stacks.
		Projection->popMatrix();
		View->popMatrix();
		Model->popMatrix();

		// draw the text
		colorFade();

		stringstream onscreen_text;
		onscreen_text << "Number of coins collected: " << countSetBits(coin_flags) << "/8";
		renderText(onscreen_text.str(), 20.0f, 20.0f, 1.0f, text_color);

		// and do a congratulations and play a jingle if you've collected all the coins
		if (coin_flags == 0xFF) {
			renderText("Congratulations, you found all the coins!", 20.0f, height - 60,
				1.0f, text_color);
		}

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
	// Where the resources are loTeaded from
	std::string resourceDir = "../resources";

	vector<std::string> faces{
		"/skybox/front.bmp",
		"/skybox/back.bmp",
		"/skybox/top.bmp",
		"/skybox/bottom.bmp",
		"/skybox/left.bmp",
		"/skybox/right.bmp"
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
	windowManager->init(1366, 768);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	// This is the code that will likely change program to program as you
	// may need to initialize or set up different data and state

	application->init(resourceDir);
	application->initGeom(resourceDir);
    application->initTex(resourceDir);
	application->initPart();
    res = application->createSky(resourceDir, faces);
	// ShowCursor(false); // Windows-specific function, commented out for cross-platform compatibility

	// start up the music
	if (application->initAudio() == 0 && application->backgroundMusic) {
		Mix_PlayMusic(application->backgroundMusic, -1); // -1 means loop indefinitely
	}

	// init text to write on screen
	if (application->initText() == -1) {
		return -1;
	}

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

	// Cleanup audio and quit program.
	application->cleanupAudio();
	windowManager->shutdown();
	return 0;
}
