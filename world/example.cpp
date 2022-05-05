/*
made using following resources:
https://learnopengl.com/
https://lazyfoo.net/tutorials/SDL/index.php
https://www.youtube.com/watch?v=QM4WW8hcsPU&list=PLvv0ScY6vfd-p1gSnbQhY7vMe2rng0IL0&index=1
*/

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <glad/glad.h>

#include <SDL.h>
#include <SDL_opengl.h>

#include <stb_image.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

namespace Utils {
    float clamp(float val, float low, float high) {
        if (val < low) {
            return low;
        }
        else if (val > high) {
            return high;
        }
        return val;
    }

    float getRandFloat() {
        return static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    }

    std::string readFile(std::string filename) { //read entire file (https://stackoverflow.com/questions/2912520/read-file-contents-into-a-string-in-c)
        std::ifstream my_file(filename);
        std::string my_string((std::istreambuf_iterator<char>(my_file)), (std::istreambuf_iterator<char>()));
        return my_string;
    }

    std::vector<std::string> splitOn(std::string input, std::string delimiter) {
        std::vector<std::string> chunks = {};
        size_t len = delimiter.length();
        size_t offset = 0;
        size_t loc = 0;
        while ((loc = input.find(delimiter, offset)) != std::string::npos) {
            chunks.push_back(input.substr(offset, loc - offset));
            offset = loc + len;
        }
        chunks.push_back(input.substr(offset, input.length() - offset));
        return chunks;
    }

    template<typename T>
    void print(T x) {
        std::cout << x << std::endl;
    }
}

class Program {
public:
    int ScreenWidth;
    int ScreenHeight;
    std::string Name;

    Program(int screen_width, int screen_height, std::string name) {
        ScreenWidth = screen_width;
        ScreenHeight = screen_height;
        Name = name;
    }

    float getAspectRatio() {
        return (float)ScreenWidth / (float)ScreenHeight;
    }
};

class MeshInstance {
public:
    std::vector<GLfloat> VertexData;
    std::vector<GLint> IndexData;

    MeshInstance(std::string filename) {
        VertexData = {};
        IndexData = {};

        std::string file_string = Utils::readFile(filename);
        std::vector<std::string> split_strings = Utils::splitOn(file_string, "\n\n");

        std::vector<std::string> vertex_lines = Utils::splitOn(split_strings[0], "\n");
        std::vector<std::string> index_lines = Utils::splitOn(split_strings[1], "\n");

        for (int i = 0; i < vertex_lines.size(); i++) {
            std::vector<std::string> vertex_strings = Utils::splitOn(vertex_lines[i], ",");
            for (int j = 0; j < vertex_strings.size(); j++) {
                VertexData.push_back(std::stof(vertex_strings[j]));
            }
        }
        for (int i = 0; i < index_lines.size(); i++) {
            std::vector<std::string> index_strings = Utils::splitOn(index_lines[i], ",");
            for (int j = 0; j < index_strings.size(); j++) {
                IndexData.push_back(std::stoi(index_strings[j]));
            }
        }
    }

    GLsizeiptr getVertexDataSize() {
        return sizeof(VertexData[0]) * VertexData.size();
    }

    GLsizeiptr getIndexDataSize() {
        return sizeof(IndexData[0]) * IndexData.size();
    }
};

class Transform {
public:
    glm::vec3 Translation;
    glm::vec3 Rotation;
    glm::vec3 Scale;

    Transform() {
        Translation = glm::vec3();
        Rotation = glm::vec3();
        Scale = glm::vec3(1.0f);
    }

    glm::mat4 getTransformMatrix() { //apply scale, then rotation (Y -> X -> Z), then translation
        glm::mat4 trans = glm::mat4(1.0f);
        trans = glm::rotate(trans, Rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
        trans = glm::rotate(trans, Rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
        trans = glm::rotate(trans, Rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
        trans = glm::translate(trans, Translation);
        trans = glm::scale(trans, Scale);
        return trans;
    }
};

class FPSCamera {
    Transform Trans;

public:
    float Fov;
    float Near;
    float Far;
    float Aspect;

    FPSCamera(float fov, float aspect, float near, float far) {
        Trans = Transform();

        Fov = fov;
        Aspect = aspect;
        Near = near;
        Far = far;
    }

    void setOrientation(glm::vec2 orientation) {
        Trans.Rotation.x = Utils::clamp(orientation.y, -(float)M_PI / 2.0f, (float)M_PI / 2.0f);
        Trans.Rotation.y = fmod(orientation.x, 2.0f * (float)M_PI);
    }
    glm::vec2 getOrientation() {
        return glm::vec2(Trans.Rotation.y, Trans.Rotation.x);
    }

    void setTranslation(glm::vec3 translation) {
        Trans.Translation = translation;
    }
    glm::vec3 getTranslation() {
        return Trans.Translation;
    }

    void relativeMove(glm::vec3 move) {
        //move rotated about y-axis based on Trans.Rotation.y
        glm::vec3 rotated_move = glm::vec3(glm::rotate(glm::mat4(1.0f), -Trans.Rotation.y, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(-move, 1.0f));
        Trans.Translation += rotated_move;
    }

    glm::mat4 getViewMatrix() {
        return Trans.getTransformMatrix();
    }

    glm::mat4 getProjectionMatrix() {
        return glm::perspective(Fov, Aspect, Near, Far);
    }
};

int main(int argc, char* argv[]) {

    Program main_program = Program(1280, 720, "World Engine");

    /******************************************************
    * setup SDL and OpenGL
    ******************************************************/

    if (SDL_Init(SDL_INIT_VIDEO) < 0){
        std::cout << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }

    //specify OpenGL version and profile
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_Window* window = SDL_CreateWindow(
        main_program.Name.c_str(), 
        SDL_WINDOWPOS_UNDEFINED, 
        SDL_WINDOWPOS_UNDEFINED, 
        main_program.ScreenWidth, 
        main_program.ScreenHeight, 
        SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
    if (window == NULL){
        std::cout << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }

    SDL_GLContext context = SDL_GL_CreateContext(window);
    if (context == NULL) {
        std::cout << "Context could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }

    if (gladLoadGLLoader(SDL_GL_GetProcAddress) < 0) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        SDL_Quit();
        return -1;
    }

    /******************************************************
    * setting misc SDL and OpenGL settings
    ******************************************************/

    SDL_SetRelativeMouseMode(SDL_TRUE); //keep mouse in screen

    glClearColor(0.5f, 0.5f, 0.5f, 1.0f); //set clear color to grey 
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); //set drawing mode
    glEnable(GL_DEPTH_TEST); //enable z-buffer depth testing

    /******************************************************
    * set up shaders and shader program
    ******************************************************/
    std::string vert_string = Utils::readFile("example.vert");
    const char* vert_c_string = vert_string.c_str();

    std::string frag_string = Utils::readFile("example.frag");
    const char* frag_c_string = frag_string.c_str();

    //create vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER); //store unique shader ID
    glShaderSource(vertexShader, 1, &vert_c_string, NULL); //set shader source code
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
    glShaderSource(fragmentShader, 1, &frag_c_string, NULL);
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

    MeshInstance cube = MeshInstance("cube.csv");

    //create vertex array object (VAO) to store all info for a set of vertices
    GLuint VAO;
    glGenVertexArrays(1, &VAO); //get 1 vertex array object ID
    glBindVertexArray(VAO); //bind this vertex array so that VBO configuration gets added to this object

    //send vertex data to the gpu
    GLuint VBO; //variable to store a unique vertex buffer object IDs
    glGenBuffers(1, &VBO); //get 1 buffer object ID
    glBindBuffer(GL_ARRAY_BUFFER, VBO); //bind buffer type to vertex buffer object (VBO)
    glBufferData(GL_ARRAY_BUFFER, cube.getVertexDataSize(), cube.VertexData.data(), GL_STATIC_DRAW);

    //make element buffer object for storing vertex indices
    GLuint EBO; 
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, cube.getIndexDataSize(), cube.IndexData.data(), GL_STATIC_DRAW);

    //specify how to attach vertex attributes to vertex shader (using data currently bound to GL_ARRAY_BUFFER)
    GLint pos_index = glGetAttribLocation(shaderProgram, "in_position"); //query index of "in_position" input variable in the shader program
    glVertexAttribPointer(pos_index, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
    glEnableVertexAttribArray(pos_index); //enable vertex attribute aPos at location "0"

    GLint tex_index = glGetAttribLocation(shaderProgram, "in_tex_coord");
    glVertexAttribPointer(tex_index, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
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
        std::cout << "Failed to load texture" << std::endl;
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
        std::cout << "Failed to load texture 2" << std::endl;
    }

    ///////////////////////////////////

    glUseProgram(shaderProgram); //choose shader program to use (before setting texture uniforms)
    glUniform1i(glGetUniformLocation(shaderProgram, "sea_texture"), 0); //set uniform (sea_texture is intended to be GL_TEXTURE0 so we bind a 0)
    glUniform1i(glGetUniformLocation(shaderProgram, "payday_texture"), 1);

    /******************************************************
    * enter rendering loop
    ******************************************************/
    bool running = true;

    float last_time = 0.0f;
    float time = 0.0f;

    float move_speed = 5.0f;
    float fast_mult = 2.0f;
    float look_sensitivity = 0.2f;
    float zoom_sensitivity = 2.0f;

    FPSCamera cam = FPSCamera(glm::radians(45.0f), main_program.getAspectRatio(), 0.1f, 100.0f);

    float mix_val = 1.0f;
    
    glm::vec3 cube_rotations[10];
    for (int i = 0; i < 10; i++) {
        cube_rotations[i] = glm::vec3(Utils::getRandFloat(), Utils::getRandFloat(), Utils::getRandFloat());
    }
    glm::vec3 cube_positions[10];
    for (int i = 0; i < 10; i++) {
        cube_positions[i] = glm::vec3((Utils::getRandFloat() - 0.5) * 8, (Utils::getRandFloat() - 0.5) * 8, (Utils::getRandFloat() - 0.5) * 8);
    }
    
    while (running) {
        //time calculation:
        last_time = time;
        time = (float)SDL_GetTicks() / 1000.0f;
        float delta = time - last_time;

        //input handling:

        SDL_Event event;
        while (SDL_PollEvent(&event)) { //SDL_PollEvent() implicitly calls SDL_PumpEvents(), necessary for below to work (I think)
            if (event.type == SDL_KEYDOWN) {
                //quit
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    running = false;
                }
                if (event.key.keysym.sym == SDLK_r) {
                    cam = FPSCamera(glm::radians(45.0f), main_program.getAspectRatio(), 0.1f, 100.0f);
                }
            }
            if (event.type == SDL_MOUSEWHEEL) { //camera zoom (fov)
                cam.Fov -= event.wheel.y * zoom_sensitivity * delta;
            }
        }

        //this way of doing input events doesn't lead to juttering for smooth transitions, unlike above
        const Uint8* state = SDL_GetKeyboardState(NULL);

        //control texture mix
        if (state[SDL_SCANCODE_UP]) {
            mix_val = Utils::clamp(mix_val + 0.005f, 0.0f, 1.0f);
        }
        if (state[SDL_SCANCODE_DOWN]) {
            mix_val = Utils::clamp(mix_val - 0.005f, 0.0f, 1.0f);
        }

        //camera rotation
        int mouse_x;
        int mouse_y;
        Uint32 mouse_buttons = SDL_GetRelativeMouseState(&mouse_x, &mouse_y);
        glm::vec2 mouse_move = glm::vec2((float)mouse_x, (float)mouse_y) * look_sensitivity * delta;
        cam.setOrientation(cam.getOrientation() + mouse_move);

        //camera movement
        glm::vec3 move = glm::vec3();
        if (state[SDL_SCANCODE_W]) {
            move.z -= 1.0f;
        }
        if (state[SDL_SCANCODE_S]) {
            move.z += 1.0f;
        }
        if (state[SDL_SCANCODE_A]) {
            move.x -= 1.0f;
        }
        if (state[SDL_SCANCODE_D]) {
            move.x += 1.0f;
        }
        if (state[SDL_SCANCODE_SPACE]) {
            move.y += 1.0f;
        }
        if (state[SDL_SCANCODE_LCTRL]) {
            move.y -= 1.0f;
        }

        if (move != glm::vec3()) {
            if (state[SDL_SCANCODE_LSHIFT]) {
                move = normalize(move) * move_speed * fast_mult * delta;
            }
            else {
                move = normalize(move) * move_speed * delta;
            }
        }

        cam.relativeMove(move);

        //rendering commands:

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //clear screen

        //choose textures to use
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textures[0]);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, textures[1]);

        glUseProgram(shaderProgram); //choose shader program to use
        glBindVertexArray(VAO); //choose vertices to use

        //calculate and set shader program's "uniform" variables
        glUniform1f(glGetUniformLocation(shaderProgram, "mix_val"), mix_val); //sets uniform value (has to be called *after* using shader program)

        //view: world space -> view space (adjust to camera)
        glm::mat4 view = cam.getViewMatrix();
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, value_ptr(view));

        //proj: view space -> clip space (add perspective projection and normalize to NDCs)
        glm::mat4 proj = cam.getProjectionMatrix();
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "proj"), 1, GL_FALSE, value_ptr(proj));

        for (int i = 0; i < std::size(cube_positions); i++) {
            //model: local space -> world space (adjust to world)
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cube_positions[i]);
            model = glm::rotate(model, -time * (float)M_PI / 2.0f, cube_rotations[i]); // <- rotation happens before translation above
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, value_ptr(model)); //set transformation matrices
            glDrawElements(GL_TRIANGLES, cube.IndexData.size(), GL_UNSIGNED_INT, 0);
        }

        SDL_GL_SwapWindow(window); //update window using swapchain
    }

    SDL_Quit();

    return 0;
}