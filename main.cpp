#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"

#include <vector>

#include <iostream>
#include <Windows.h>
#include <ctime>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void render();
float getRandomX();
float getRandomY();
void foodIsEaten();
void gameOver();
float offset(float x, float x_old);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

const float DIM = 0.02; //dimension of figures, height==width
const float BEGIN = -(1 - DIM), END = 1 - DIM;//-0.95, 0.95
float x_head =0.0, y_head = 0.0;
float dx = 0.02, dy = 0.02;
int length = 100;
bool moving_up = true, moving_down = false;
bool moving_left = false, moving_right = false;

bool food_appeared = false;
float x_food, y_food;

class Window {
public:
	Window() {
		// glfw: initialize and configure
	// ------------------------------
		glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

	// glfw window creation
	// --------------------
		window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Snake", NULL, NULL);
		if (window == NULL)
		{
			std::cout << "Failed to create GLFW window" << std::endl;
			glfwTerminate();
		}
		glfwMakeContextCurrent(window);
		glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

		// glad: load all OpenGL function pointers
		// ---------------------------------------
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
			std::cout << "Failed to initialize GLAD" << std::endl;
		}
	}
	int shouldNotClose() {
		return glfwWindowShouldClose(window);
	}
	void update() {
		processInput(window);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	~Window() {
		glfwDestroyWindow(window);
	}
private:
	GLFWwindow* window = nullptr;
};

class Mesh {
public:
	Mesh() {
		// set up vertex data (and buffer(s)) and configure vertex attributes
	// ------------------------------------------------------------------
		float vertices[] = {
			// positions          // texture coords
			 0.01f,  0.01f, 0.0f,   1.0f, 1.0f, // top right
			 0.01f, -0.01f, 0.0f,   1.0f, 0.0f, // bottom right
			-0.01f, -0.01f, 0.0f,   0.0f, 0.0f, // bottom left
			-0.01f,  0.01f, 0.0f,   0.0f, 1.0f  // top left 
		};
		unsigned int indices[] = {
			0, 1, 3, // first triangle
			1, 2, 3  // second triangle
		};
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

		// position attribute
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		// texture coord attribute
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);

		// load and create a texture 
		// -------------------------
		// texture 1
		// ---------
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// load image, create texture and generate mipmaps
		stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
		data = stbi_load("blueBall.jpg", &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else
		{
			std::cout << "Failed to load texture" << std::endl;
		}
		stbi_image_free(data);
	}
	void bindTexture() {
		// bind textures on corresponding texture units
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
	}void draw() {
		// with the uniform matrix set, draw the first container
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	}
	~Mesh() {
		// optional: de-allocate all resources once they've outlived their purpose:
		// ------------------------------------------------------------------------
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &EBO);
	}
private:
	unsigned int VBO, VAO, EBO;
	unsigned int texture;
	int width, height, nrChannels;
	unsigned char* data;
};

class Snake {
public:
	Snake() {
		Window window;
		Shader ourShader("2d.vs", "2d.fs");
		Mesh mesh;

		srand(time(NULL));

		std::vector<float> x(length + 1);
		std::vector<float> y(length + 1);
		std::vector<float> x_old(length + 1);
		std::vector<float> y_old(length + 1);
	
		while (!window.shouldNotClose())
		{
			render();
			mesh.bindTexture();
			ourShader.use();

			//HEAD
			// create transformations
			glm::mat4 transform = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
			transform = glm::translate(transform, glm::vec3(x_head, y_head, 0.0f));
			// get their uniform location and set matrix (using glm::value_ptr)
			transformLoc = glGetUniformLocation(ourShader.ID, "transform");
			glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));
			mesh.draw();

			x[0] = x_head;
			y[0] = y_head;

			for (int i = 1; i < length; ++i) {
				x[i] = x_old[i - 1];
				y[i] = y_old[i - 1];
			}

			//TAIL
			for (int i = 1; i < length; ++i) {
				// create transformations
				glm::mat4 transform = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
				transform = glm::translate(transform, glm::vec3(x[i], y[i], 0.0f));
				// get their uniform location and set matrix (using glm::value_ptr)
				transformLoc = glGetUniformLocation(ourShader.ID, "transform");
				glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));
				mesh.draw();
			}

			for (int i = 0; i < length; ++i) {
				y_old[i] = y[i];
				x_old[i] = x[i];
			}

			//FOOD
			//FOOD sometimes appears on the screen
			if (!food_appeared) {
				x_food = getRandomX();
				for (int i = 1; i < length; ++i) {
					do x_food = getRandomX();
					while((x[i] > (x_food - DIM * 2)) && (x[i] < (x_food + DIM * 2)));//if tail near food
				}
				y_food = getRandomY();
				for (int i = 1; i < length; ++i) {
					do y_food = getRandomY();
					while ((y[i] > (y_food - DIM * 2)) && (y[i] < (y_food + DIM * 2)));//if tail near food
				}
				std::cout << x_food << '\t' << y_food << '\n';
				food_appeared = true;
			}
			transform = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
			transform = glm::translate(transform, glm::vec3(x_food, y_food, 0.0f));
			// get their uniform location and set matrix (using glm::value_ptr)
			transformLoc = glGetUniformLocation(ourShader.ID, "transform");
			glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));
			mesh.draw();

			//food is eaten
			if (((x_head > (x_food - DIM/4)) && (x_head < (x_food + DIM/4))) && ((y_head > (y_food - DIM/4)) && (y_head < (y_food + DIM/4))))
				foodIsEaten();

			//vector resize
			if (length == (x.size() - 1)) {
				x.reserve(length + 10);
				x_old.reserve(length + 10);
				y.reserve(length + 10);
				y_old.reserve(length + 10);
				for (int i = length - 1; i < length + 10; ++i) {
					x.push_back(0.0f);
					x_old.push_back(0.0f);
					y.push_back(0.0f);
					y_old.push_back(0.0f);
				}
			}

			//game over if collision with tail or screen borders
			for (int i = 1; i < length; ++i) {
				if ((x_head == x[i]) && (y_head == y[i]) && init )
					gameOver();
			}
			if (moving_up) { 
				if (y_head < END)
					y_head += dy;
				else
					gameOver();
			}
			if (moving_down) { 
				if (y_head > BEGIN)
					y_head -= dy;
				else
					gameOver();
			}
			if (moving_right) {
				if (x_head < END)
					x_head += dx;
				else
					gameOver();
			}
			if (moving_left) { 
				if (x_head > BEGIN)
					x_head -= dx;
				else
					gameOver();
			}

			Sleep(50);

			// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
			// -------------------------------------------------------------------------------
			window.update();

			if (!init) 
				init = true;
		}
	}
	~Snake() {
		// glfw: terminate, clearing all previously allocated GLFW resources.
		// ------------------------------------------------------------------
		glfwTerminate();
	}
private:
	unsigned int transformLoc;
	bool init = false;
};

void main()
{
	Snake game;
}

float offset(float x, float x_old) {
	if (x == (x_old + dx))
		x = x_old + DIM;
	if (x == (x_old - dx))
		x = x_old - DIM;
	return x;
}

void foodIsEaten() {
	++length;
	food_appeared = false;
}

void gameOver() {
	system("pause");
}

// get random coordinates X and Y for food and bonuses
float getRandomX() {
	float x_temp;

	do
	{
		x_temp = rand() % 150 + 30;
		if ((int)x_temp % 2 != 0) ++x_temp;
		x_temp = (x_temp / 100) - 1;
	} while ((x_head > (x_temp - DIM * 2)) && (x_head < (x_temp + DIM * 2)));//if snake near food
	return x_temp;
}
float getRandomY() {
	float y_temp;
	do
	{
		y_temp = rand() % 150 + 30;
		if ((int)y_temp % 2 != 0) ++y_temp;
		y_temp = (y_temp / 100) - 1;
	} while ((y_head > (y_temp - DIM * 2)) && (y_head < (y_temp + DIM * 2)));//if snake near food
	return y_temp;
}

// render
void render() {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
		//if (x_head < END)
			moving_up = false;
			moving_down = false;
			moving_left = false;
			moving_right = true;
	}

	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
		//if (x_head > BEGIN)
			moving_up = false;
			moving_down = false;
			moving_left = true;
			moving_right = false;
	}

	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
		//if (y_head < END)
			moving_up = true;
			moving_down = false;
			moving_left = false;
			moving_right = false;
	}

	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
		//if (y_head > BEGIN)
			moving_up = false;
			moving_down = true;
			moving_left = false;
			moving_right = false;
	}
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}
