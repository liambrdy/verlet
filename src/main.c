#include <stdio.h>
#include <stdbool.h>

#include <SDL2/SDL.h>

#include "renderer.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define FPS 60
#define DELTA_TIME (1.0f / FPS)

void GLAPIENTRY MessageCallback(GLenum source,
                                GLenum type,
                                GLuint id,
                                GLenum severity,
                                GLsizei length,
                                const GLchar* message,
                                const void* userParam) {
    (void) source;
    (void) id;
    (void) length;
    (void) userParam;
    fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n", (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type, severity, message);
}

typedef struct {
    Vec2f positionOld;
    Vec2f positionCurrent;
    Vec2f acceleration;
} Object;

#define OBJECT_CAP 10

static Renderer renderer = {0};
static Object objects[OBJECT_CAP] = {0};
static size_t objectCount = 0;

void drawCircle(Renderer *r, Vec2f c, float radius, Vec4f color) {
    rendererRectCentered(r, c, vec2fs(radius*2.0), color);
}

int main() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "ERROR: Failed to initialize SDL: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("Verlet", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (window == NULL) {
        fprintf(stderr, "Failed to create SDL window: %s\n", SDL_GetError());
        return 1;
    }

    {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

        int major, minor;
        SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &major);
        SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &minor);
        printf("GL version %d.%d\n", major, minor);
    }

    if (SDL_GL_CreateContext(window) == NULL) {
        fprintf(stderr, "ERROR: Failed to create opengl context: %s\n", SDL_GetError());
        return 1;
    }

    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "ERROR: Failed to initalize glew\n");
    }

    if (GLEW_ARB_debug_output) {
        glEnable(GL_DEBUG_OUTPUT);
        glDebugMessageCallback(MessageCallback, 0);
    } else {
        fprintf(stderr, "WARNING! GLEW_ARB_debug_output is not available\n");
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    rendererInit(&renderer);

    Object object = {0};
    object.positionCurrent = vec2f(0.0f, 0.0f);
    object.positionOld = vec2f(0.0f, 0.0f);
    objects[objectCount++] = object;

    Uint64 now = SDL_GetPerformanceCounter();
    Uint64 last = 0;
    float dt = DELTA_TIME;

    bool running = true;
    while (running) {
        const Uint32 start = SDL_GetTicks();
        SDL_Event event = {0};
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT: {
                    running = false;
                } break;
            }
        }

        glClearColor(0.0f, 0.15f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        int w, h;
        SDL_GetWindowSize(window, &w, &h);

        rendererUse(&renderer);
        glUniform2f(renderer.uniforms[UNIFORM_SLOT_RESOLUTION], (float) w, (float) h);

        const Vec2f position = vec2fs(0.0f);
        const float radius = 300.0f;
        drawCircle(&renderer, position, radius, vec4f(0.7f, 0.2f, 0.2f, 0.4f));

        for (size_t i = 0; i < objectCount; i++) {
            Object obj = objects[i];

            const Vec2f gravity = vec2f(0.0f, -100.0f);
            objects[i].acceleration = vec2fAdd(objects[i].acceleration, gravity);

            const Vec2f toObj = vec2fSub(obj.positionCurrent, position);
            const float dist = sqrtf(toObj.x*toObj.x + toObj.y*toObj.y);
            if (dist > radius - 50.0f) {
                const Vec2f n = vec2fDiv(toObj, vec2fs(dist));
                objects[i].positionCurrent = vec2fAdd(position, vec2fMul(n, vec2fs(radius - 50.0f)));
            }

            const Vec2f velocity = vec2fSub(objects[i].positionCurrent, objects[i].positionOld);
            objects[i].positionOld = objects[i].positionCurrent;
            Vec2f dtAcceleration = vec2fMul(objects[i].acceleration, vec2fs(dt*dt));
            objects[i].positionCurrent = vec2fAdd(objects[i].positionCurrent, vec2fAdd(velocity, dtAcceleration));
            objects[i].acceleration = vec2fs(0.0f);

            drawCircle(&renderer, objects[i].positionCurrent, 50.0f, vec4fs(1.0f));
        }

        rendererFlush(&renderer);

        SDL_GL_SwapWindow(window);

        last = now;
        now = SDL_GetPerformanceCounter();
        float dt = (float)((now - last)*1000 / (float)SDL_GetPerformanceFrequency());

        const Uint32 duration = SDL_GetTicks() - start;
        const Uint32 deltaTime = 1000 / FPS;
        if (duration < deltaTime) {
            SDL_Delay(deltaTime - duration);
        }
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}