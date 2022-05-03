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

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;

//lets us print vectors
std::ostream& operator<< (std::ostream& out, const glm::vec3& vec) {
    out << "{"
        << vec.x << " " << vec.y << " " << vec.z
        << "}";

    return out;
}
std::ostream& operator<< (std::ostream& out, const glm::vec2& vec) {
    out << "{"
        << vec.x << " " << vec.y << " "
        << "}";

    return out;
}

const GLfloat cube_vertices[] = {
    -0.5f, -0.5f, -0.5f, -0.5, -0.5,
     0.5f, -0.5f, -0.5f,  1.5, -0.5,
    -0.5f,  0.5f, -0.5f, -0.5,  1.5,
     0.5f,  0.5f, -0.5f,  1.5,  1.5,
    -0.5f, -0.5f,  0.5f,  1.5, -0.5,
     0.5f, -0.5f,  0.5f, -0.5, -0.5,
    -0.5f,  0.5f,  0.5f,  1.5,  1.5,
     0.5f,  0.5f,  0.5f, -0.5,  1.5
};

const GLuint cube_indices[] = {
    0, 1, 2,  3, 2, 1, //front
    5, 4, 7,  6, 7, 4, //back
    4, 0, 6,  2, 6, 0, //left
    1, 5, 3,  7, 3, 5, //right
    2, 3, 6,  7, 6, 3, //top
    4, 5, 0,  1, 0, 5  //bottom
};

vec3 cube_positions[] = {
    vec3(0.0f,  0.0f,  0.0f),
    vec3(2.0f,  5.0f, -15.0f),
    vec3(-1.5f, -2.2f, -2.5f),
    vec3(-3.8f, -2.0f, -12.3f),
    vec3(2.4f, -0.4f, -3.5f),
    vec3(-1.7f,  3.0f, -7.5f),
    vec3(1.3f, -2.0f, -2.5f),
    vec3(1.5f,  2.0f, -2.5f),
    vec3(1.5f,  0.2f, -1.5f),
    vec3(-1.3f,  1.0f, -1.5f)
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

float rand_float() {
    return static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
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
    * set SDL settings
    ******************************************************/

    SDL_SetRelativeMouseMode(SDL_TRUE); //keep mouse in screen

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
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW); //copy vertex data into the buffer (GL_STATIC_DRAW means used a lot but unchanging)

    //make element buffer object for storing vertex indices
    GLuint EBO; 
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_indices), cube_indices, GL_STATIC_DRAW);

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
    glEnable(GL_DEPTH_TEST); //enable depth testing

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

    //rendering state

    vec2 cam_rot = vec2();
    vec3 cam_trans = vec3(0.0f, 0.0f, 5.0f);
    float cam_fov = radians(45.0f);

    float mix_val = 1.0f;
    
    vec3 cube_rotations[] = {
        vec3(rand_float(), rand_float(), rand_float()),
        vec3(rand_float(), rand_float(), rand_float()),
        vec3(rand_float(), rand_float(), rand_float()),
        vec3(rand_float(), rand_float(), rand_float()),
        vec3(rand_float(), rand_float(), rand_float()),
        vec3(rand_float(), rand_float(), rand_float()),
        vec3(rand_float(), rand_float(), rand_float()),
        vec3(rand_float(), rand_float(), rand_float()),
        vec3(rand_float(), rand_float(), rand_float()),
        vec3(rand_float(), rand_float(), rand_float())
    };
    
    while (running) {

        //time calculation:
        last_time = time;
        time = (float)SDL_GetTicks() / 1000;
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
                    cam_rot = vec2();
                    cam_trans = vec3(0.0f, 0.0f, 5.0f);
                    cam_fov = radians(45.0f);
                }
            }
            if (event.type == SDL_MOUSEWHEEL) { //camera zoom (fov)
                cam_fov -= event.wheel.y * zoom_sensitivity * delta;
            }
        }

        //this way of doing input events doesn't lead to juttering for smooth transitions, unlike above
        const Uint8* state = SDL_GetKeyboardState(NULL);

        //control texture mix
        if (state[SDL_SCANCODE_UP]) {
            mix_val = clamp(mix_val + 0.005f, 0.0f, 1.0f);
        }
        if (state[SDL_SCANCODE_DOWN]) {
            mix_val = clamp(mix_val - 0.005f, 0.0f, 1.0f);
        }

        //camera rotation
        int mouse_x;
        int mouse_y;
        Uint32 mouse_buttons = SDL_GetRelativeMouseState(&mouse_x, &mouse_y);
        vec2 mouse_move = vec2((float)mouse_x, (float)mouse_y) * look_sensitivity * delta;
        cam_rot += mouse_move;
        cam_rot.x = fmod(cam_rot.x, 2 * M_PI);
        cam_rot.y = clamp(cam_rot.y, -M_PI / 2, M_PI / 2);

        //camera movement
        vec3 frame_trans = vec3();
        if (state[SDL_SCANCODE_W]) {
            frame_trans.z -= 1.0f;
        }
        if (state[SDL_SCANCODE_S]) {
            frame_trans.z += 1.0f;
        }
        if (state[SDL_SCANCODE_A]) {
            frame_trans.x -= 1.0f;
        }
        if (state[SDL_SCANCODE_D]) {
            frame_trans.x += 1.0f;
        }
        if (state[SDL_SCANCODE_SPACE]) {
            frame_trans.y += 1.0f;
        }
        if (state[SDL_SCANCODE_LCTRL]) {
            frame_trans.y -= 1.0f;
        }

        if (frame_trans != vec3()) {
            if (state[SDL_SCANCODE_LSHIFT]) {
                frame_trans = normalize(frame_trans) * move_speed * fast_mult * delta;
            }
            else {
                frame_trans = normalize(frame_trans) * move_speed * delta;
            }
        }
        frame_trans = vec3(rotate(mat4(1.0f), -cam_rot.x, vec3(0.0f, 1.0f, 0.0f)) * vec4(frame_trans, 1.0f));
        cam_trans += frame_trans;

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
        mat4 view = mat4(1.0f);
        view = rotate(view, cam_rot.y, vec3(1.0f, 0.0f, 0.0f));
        view = rotate(view, cam_rot.x, vec3(0.0f, 1.0f, 0.0f));
        view = translate(view, -cam_trans);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, value_ptr(view));

        //proj: view space -> clip space (add perspective projection and normalize to NDCs)
        mat4 proj = perspective(cam_fov, (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "proj"), 1, GL_FALSE, value_ptr(proj));

        for (unsigned int i = 0; i < size(cube_positions); i++) {
            //model: local space -> world space (adjust to world)
            mat4 model = mat4(1.0f);
            model = translate(model, cube_positions[i]);
            model = rotate(model, -time * (float)M_PI / 2, cube_rotations[i]); // <- rotation happens before translation above
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, value_ptr(model)); //set transformation matrices
            glDrawElements(GL_TRIANGLES, size(cube_indices), GL_UNSIGNED_INT, 0); //render triangles to buffer (using bound EBO)
        }

        SDL_GL_SwapWindow(window); //update window using swapchain
    }

    SDL_Quit();

    return 0;
}