/*
made using following resources:
https://learnopengl.com/
https://lazyfoo.net/tutorials/SDL/index.php
https://www.youtube.com/watch?v=QM4WW8hcsPU&list=PLvv0ScY6vfd-p1gSnbQhY7vMe2rng0IL0&index=1
*/

#include <iostream>
#include <fstream>
#include <string>

#include <glad/glad.h>

#include <SDL.h>
#include <SDL_opengl.h>

#include <stb_image.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

using namespace std;
using namespace glm;

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 640;

const GLfloat rect_vertices[] = { //automatic size assigned to array because square brackets are empty []
    //positions           //colors     //texture
     0.75f,  0.75f, 0.0f, 1.0f, 1.0f,  1.5f,  1.5f, // top right
     0.75f, -0.75f, 0.0f, 1.0f, 0.0f,  1.5f, -0.5f, // bottom right
    -0.75f, -0.75f, 0.0f, 0.0f, 0.0f, -0.5f, -0.5f, // bottom left
    -0.75f,  0.75f, 0.0f, 0.0f, 1.0f, -0.5f,  1.5f, // top left
};
const GLuint rect_indices[] = { // note that we start from 0!
    0, 1, 3, // first triangle
    1, 2, 3, // second triangle
};

float clamp(float val, float low, float high) {
    if (val < low) {
        return low;
    }
    else if (val > high) {
        return high;
    }
    return val;
}

int main(int argc, char* argv[]) {

    /******************************************************
    * setup SDL and OpenGL
    ******************************************************/

    if (SDL_Init(SDL_INIT_VIDEO) < 0){
        cout << "SDL could not initialize! SDL_Error: " << SDL_GetError() << "\n";
        SDL_Quit();
        return -1;
    }

    //specify OpenGL version and profile
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_Window* window = SDL_CreateWindow("World Engine", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
    if (window == NULL){
        cout << "Window could not be created! SDL_Error: " << SDL_GetError() << "\n";
        SDL_Quit();
        return -1;
    }

    SDL_GLContext context = SDL_GL_CreateContext(window);
    if (context == NULL) {
        cout << "Context could not be created! SDL_Error: " << SDL_GetError() << "\n";
        SDL_Quit();
        return -1;
    }

    if (gladLoadGLLoader(SDL_GL_GetProcAddress) < 0) {
        cout << "Failed to initialize GLAD";
        SDL_Quit();
        return -1;
    }

    /******************************************************
    * set up shaders and shader program
    ******************************************************/

    //read shaders from files (https://stackoverflow.com/questions/2912520/read-file-contents-into-a-string-in-c)
    ifstream vertFile("example.vert");
    string vertString((istreambuf_iterator<char>(vertFile)), (istreambuf_iterator<char>()));
    const char* vertCString = vertString.c_str();

    ifstream fragFile("example.frag");
    string fragString((istreambuf_iterator<char>(fragFile)), (istreambuf_iterator<char>()));
    const char* fragCString = fragString.c_str();

    //create vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER); //store unique shader ID
    glShaderSource(vertexShader, 1, &vertCString, NULL); //set shader source code
    glCompileShader(vertexShader); //compile shader
    int  success; //shader compilation error handling
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << endl;
    }

    //create fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragCString, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << endl;
    }

    //create shader program (linking vertex and fragment shaders)
    GLuint shaderProgram = glCreateProgram(); //store unique program ID
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success); //shader program linking error handling
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        cout << "ERROR::PROGRAM::LINKING_FAILED\n" << infoLog << endl;
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
    GLint pos_index = glGetAttribLocation(shaderProgram, "in_position"); //query index of "in_position" input variable in the shader program
    glVertexAttribPointer(pos_index, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), 0);
    glEnableVertexAttribArray(pos_index); //enable vertex attribute aPos at location "0"

    GLint col_index = glGetAttribLocation(shaderProgram, "in_color");
    glVertexAttribPointer(col_index, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(col_index);

    GLint tex_index = glGetAttribLocation(shaderProgram, "in_tex_coord");
    glVertexAttribPointer(tex_index, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(tex_index);

    /******************************************************
    * configure texture data (using stb image library https://github.com/nothings/stb)
    ******************************************************/

    //image reading setting
    stbi_set_flip_vertically_on_load(true);

    //vars
    int tex_width, tex_height, tex_channel_num;
    stbi_uc* tex_data;

    //allocate memory and get ids
    GLuint textures[2];
    glGenTextures(2, textures);

    ///////////////////////////////////

    glBindTexture(GL_TEXTURE_2D, textures[0]); //bind first ID as current texture

    //set texture settings
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT); //set texture wrapping setting, for 2D textures, in X(S) and Y(T) axes, to mirrored repeating
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //bilinear interpolation when magnifying textures to produce pixel color
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); //trilinear interpolation (using mipmaps) when minifying textures to produce pixel color

    tex_data = stbi_load("sea_texture.jpg", &tex_width, &tex_height, &tex_channel_num, 0); //read texture data from file
    if (tex_data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex_width, tex_height, 0, GL_RGB, GL_UNSIGNED_BYTE, tex_data); //read texture data into texture
        glGenerateMipmap(GL_TEXTURE_2D); //automatically generate mipmaps
        stbi_image_free(tex_data); //free image data
    }
    else {
        cout << "Failed to load texture" << endl;
    }

    ///////////////////////////////////

    glBindTexture(GL_TEXTURE_2D, textures[1]); //bind second ID as current texture

    //set texture settings
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    tex_data = stbi_load("payday.jpg", &tex_width, &tex_height, &tex_channel_num, 0);
    if (tex_data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex_width, tex_height, 0, GL_RGB, GL_UNSIGNED_BYTE, tex_data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(tex_data);
    }
    else {
        cout << "Failed to load texture 2" << endl;
    }

    ///////////////////////////////////

    glUseProgram(shaderProgram); //choose shader program to use (before setting texture uniforms)
    glUniform1i(glGetUniformLocation(shaderProgram, "sea_texture"), 0); //set uniform (sea_texture is intended to be GL_TEXTURE0 so we bind a 0)
    glUniform1i(glGetUniformLocation(shaderProgram, "payday_texture"), 1);

    /******************************************************
    * setting misc settings
    ******************************************************/

    glClearColor(0.5f, 0.5f, 0.5f, 1.0f); //set clear color to grey 
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); //set drawing mode

    /******************************************************
    * enter rendering loop
    ******************************************************/

    float mix_val = 0.5f;

    bool running = true;
    while (running) {

        //input handling:

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    running = false;
                }
                else if (event.key.keysym.sym == SDLK_UP) {
                    mix_val = clamp(mix_val + 0.03f, 0.0f, 1.0f);
                }
                else if (event.key.keysym.sym == SDLK_DOWN) {
                    mix_val = clamp(mix_val - 0.03f, 0.0f, 1.0f);
                }
            }
        }

        //matrix transform calculations:

        float time = (float)SDL_GetTicks() / 1000; //time in seconds

        glm::mat4 transform = glm::mat4(1.0f); //start with identity matrix
        transform = translate(transform, vec3(0.5f, -0.5f, 0.0f)); //translate
        transform = glm::rotate(transform, time, glm::vec3(0.0f, 0.0f, 1.0f)); //rotate

        glm::mat4 transform2 = glm::mat4(1.0f);
        transform2 = translate(transform2, vec3(-0.5f, 0.5f, 0.0f));
        transform2 = glm::scale(transform2, vec3(1.0f*sin(time), 1.0f*sin(time), 1.0f));

        //rendering commands:

        //choose textures to use
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textures[0]);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, textures[1]);

        glUseProgram(shaderProgram); //choose shader program to use
        glBindVertexArray(VAO); //choose vertices to use

        //calculate and set shader program's "uniform" variables
        glUniform1f(glGetUniformLocation(shaderProgram, "mix_val"), mix_val); //sets uniform value (has to be called *after* using shader program)

        glClear(GL_COLOR_BUFFER_BIT); //clear screen

        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "transform"), 1, GL_FALSE, glm::value_ptr(transform)); //set transformation matrix
        glDrawElements(GL_TRIANGLES, size(rect_indices), GL_UNSIGNED_INT, 0); //render triangles to buffer (using bound EBO)

        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "transform"), 1, GL_FALSE, glm::value_ptr(transform2)); //reset transformation matrix
        glDrawElements(GL_TRIANGLES, size(rect_indices), GL_UNSIGNED_INT, 0); //render triangles to buffer (using bound EBO)

        SDL_GL_SwapWindow(window); //update window using swapchain
    }

    SDL_Quit();

    return 0;
}