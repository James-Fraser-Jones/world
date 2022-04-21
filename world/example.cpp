/*
made using following resources:
https://learnopengl.com/
https://lazyfoo.net/tutorials/SDL/index.php
https://www.youtube.com/watch?v=QM4WW8hcsPU&list=PLvv0ScY6vfd-p1gSnbQhY7vMe2rng0IL0&index=1
*/

#include <iostream>

#include <glad/glad.h>
#include <SDL.h>
#include <SDL_opengl.h>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

//vertex shader code
const char* vertexShaderSource = 
    "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\0";

const char* fragmentShaderSource =
    "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "    FragColor = vec4(0.035f, 0.603f, 0.117f, 0.0f);\n"
    "}\n";

//vertices for a triangle in 3D space (in normalized device coordinates i.e. +/- 1)
const GLfloat vertices[] = {
    -0.5f, -0.5f, 0.0f,
     0.5f, -0.5f, 0.0f,
     0.0f,  0.5f, 0.0f
};

const GLfloat rect_vertices[] = {
     0.5f,  0.5f, 0.0f,  // top right
     0.5f, -0.5f, 0.0f,  // bottom right
    -0.5f, -0.5f, 0.0f,  // bottom left
    -0.5f,  0.5f, 0.0f   // top left 
};
const GLuint rect_indices[] = {  // note that we start from 0!
    0, 1, 3,   // first triangle
    1, 2, 3    // second triangle
};

int main(int argc, char* argv[]) {

    /******************************************************
    * setup SDL and OpenGL
    ******************************************************/

    if (SDL_Init(SDL_INIT_VIDEO) < 0){
        std::cout << "SDL could not initialize! SDL_Error: " << SDL_GetError() << "\n";
        SDL_Quit();
        return -1;
    }

    //specify OpenGL version and profile
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_Window* window = SDL_CreateWindow("World Engine", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
    if (window == NULL){
        std::cout << "Window could not be created! SDL_Error: " << SDL_GetError() << "\n";
        SDL_Quit();
        return -1;
    }

    SDL_GLContext context = SDL_GL_CreateContext(window);
    if (context == NULL) {
        std::cout << "Context could not be created! SDL_Error: " << SDL_GetError() << "\n";
        SDL_Quit();
        return -1;
    }

    if (gladLoadGLLoader(SDL_GL_GetProcAddress) < 0) {
        std::cout << "Failed to initialize GLAD";
        SDL_Quit();
        return -1;
    }

    /******************************************************
    * setup shaders and shader program
    ******************************************************/

    //create vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER); //store unique shader ID
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL); //set shader source code
    glCompileShader(vertexShader); //compile shader
    int  success; //shader compilation error handling
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    //create fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    //create shader program (linking vertex and fragment shaders)
    GLuint shaderProgram = glCreateProgram(); //store unique program ID
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success); //shader program linking error handling
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    //delete shaders now they have been linked into shader object
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    /******************************************************
    * configure vertex data
    ******************************************************/

    //create vertex array object (VAO) to store all info for a set of vertices
    GLuint VAO;
    glGenVertexArrays(1, &VAO); //get 1 vertex array object ID
    glBindVertexArray(VAO); //bind this vertex array so that VBO configuration gets added to this object

    //send vertex data to the gpu
    GLuint VBO; //variable to store a unique vertex buffer object IDs
    glGenBuffers(1, &VBO); //get 1 buffer object ID
    glBindBuffer(GL_ARRAY_BUFFER, VBO); //bind buffer type to vertex buffer object (VBO)
    glBufferData(GL_ARRAY_BUFFER, sizeof(rect_vertices), rect_vertices, GL_STATIC_DRAW); //copy vertex data into the buffer (GL_STATIC_DRAW means used a lot but unchanging)

    //make element buffer object for storing vertex indices
    GLuint EBO; 
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(rect_indices), rect_indices, GL_STATIC_DRAW);

    //specify how to attach vertex attributes to vertex shader (using data currently bound to GL_ARRAY_BUFFER)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    /*
    arguments:
    at location "0" (i.e. aPos attribute in vertex shader),
    aPos is a vec3 so contains "3" values,
    values are "GL_FLOAT"s,
    normalize values ? "GL_FALSE",
    size of aPos? "3 * sizeof(float)"
    offset to read from? "0" (but has to be cast to void* for some reason
    */

    //enable vertex attribute aPos at location "0"
    glEnableVertexAttribArray(0);

    /******************************************************
    * misc settings
    ******************************************************/

    glClearColor(0.99f, 0.76f, 0.0f, 1.0f); //set clear color to gold
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); //set drawing mode
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    /******************************************************
    * enter rendering loop
    ******************************************************/

    bool running = true;
    while (running) {

        //input handling
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            if (event.type == SDL_MOUSEMOTION) {
                //std::cout << "mouse has been moved\n";
            }
            if (event.type == SDL_KEYDOWN) {
                //std::cout << "a key has been pressed\n";
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    running = false;
                }
            }
            //const Uint8* state = SDL_GetKeyboardState(NULL);
            //if (state[SDL_SCANCODE_RIGHT]) {
            //    std::cout << "right arrow key is pressed\n";
            //}
        }

        //rendering commands
        glClear(GL_COLOR_BUFFER_BIT); //clear screen
        glUseProgram(shaderProgram); //use our set of shaders
        glBindVertexArray(VAO); //use our set of configured vertices
        glDrawElements(GL_TRIANGLES, std::size(rect_indices), GL_UNSIGNED_INT, 0); //draw triangles (start at vertex 0, draw 3 vertices)

        //update window using swapchain
        SDL_GL_SwapWindow(window);
    }

    SDL_Quit();

    return 0;
}