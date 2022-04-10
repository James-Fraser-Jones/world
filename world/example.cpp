
#include <iostream>

#include <glad/glad.h>
#include <SDL.h>
#include <SDL_opengl.h>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

int main(int argc, char* argv[]) {

    if (SDL_Init(SDL_INIT_VIDEO) < 0){
        std::cout << "SDL could not initialize! SDL_Error: " << SDL_GetError() << "\n";
        SDL_Quit();
        return -1;
    }

    //specify OpenGL version
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

    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

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

        //rendering commands (here we're just clearing the screen to gold)
        glClearColor(0.99f, 0.76f, 0.0f, 1.0f); //set screen color
        glClear(GL_COLOR_BUFFER_BIT);

        //update window using swapchain
        SDL_GL_SwapWindow(window);
    }

    SDL_Quit();

    return 0;
}