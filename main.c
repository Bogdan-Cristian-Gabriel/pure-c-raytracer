#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include "vectors.h"
#include <pthread.h>

#define WIDTH  800
#define HEIGHT 600
#define NUM_THREADS 8

typedef struct ray {
    Vector orig, dir;
} Ray;

typedef struct sphere {
    Vector center;
    double radius;
    Vector color;
    int reflective;
} Sphere;

typedef struct coord {
    double u[WIDTH], v[HEIGHT];
} Coord;

typedef struct thread_data {
    int start_y;
    int end_y;
    Sphere *scene;
    int sphereCount;
    Coord *map;
    uint32_t *img;
    Vector cameraPos;
    double pitch, yaw;
} ThreadData;

// Sphere intersection test - returns -1 if no hit
double hitSphere(Vector center, double radius, Ray r) {
    Vector oc = subVector(r.orig, center);
    double a = dotProduct(r.dir, r.dir);
    double b = 2.0 * dotProduct(oc, r.dir);
    double c = dotProduct(oc, oc) - radius * radius;
    double delta = b * b - 4 * a *c;
    return delta < 0 ? -1.0 : ((-b - sqrt(delta)) / (2 * a)); 
}

Vector traceRay(Ray ray, Sphere scene[], int sphereCount, int depth) {
    // Recursion limit
    if (depth <= 0) {
        return initVector(0.0, 0.0, 0.0);
    }

    int hitIndex = -1;
    double closest_t = __INT_MAX__;

    // Find closest sphere intersection
    for (int i = 0; i < sphereCount; i++) {
        double t = hitSphere(scene[i].center, scene[i].radius, ray);
        if (t > 0.001 && t < closest_t) {  // 0.001 to avoid shadow acne
            closest_t = t;
            hitIndex = i;
        }
    }

    // Sky gradient if no hit
    if (hitIndex == -1) {
        double t = 0.5 * (ray.dir.y + 1.0);
        return addVector(scalarMultipl(initVector(1.0, 1.0, 1.0), 1.0 - t), 
                         scalarMultipl(initVector(0.5, 0.7, 1.0), t));
    }

    // Compute hit point and normal
    Vector lightPos = initVector(0.0, 5.0, 0.0);
    Vector P = addVector(ray.orig, scalarMultipl(ray.dir, closest_t));
    Vector N = subVector(P, scene[hitIndex].center);
    N = normaliseVector(N);

    // Light direction
    Vector L = subVector(lightPos, P);
    L = normaliseVector(L);

    // Shadow check (hard shadows)
    Ray shadow = {addVector(P, scalarMultipl(N, 0.001)), L};
    int shadowHit = -1;
    double intensity;

    for (int i = 0; i < sphereCount; i++) {
        double t = hitSphere(scene[i].center, scene[i].radius, shadow);
        if (t > 0.001) {
            shadowHit = 1;
            intensity = 0.2;  // Ambient only
            break;
        }
    }

    // Lambertian lighting if not in shadow
    if (shadowHit == -1) {
        intensity = dotProduct(N, L);
        intensity = intensity < 0.0 ? 0.0 : intensity;
        intensity += 0.2;  // Ambient light
        intensity = intensity > 1.0 ? 1.0 : intensity;
    }

    Vector localColor = scalarMultipl(scene[hitIndex].color, intensity);

    // Reflection for reflective spheres
    if (scene[hitIndex].reflective) {
        Vector reflectionDir = subVector(ray.dir, scalarMultipl(N, 2.0 * dotProduct(ray.dir, N)));
        Ray mirror = {addVector(P, scalarMultipl(N, 0.001)), normaliseVector(reflectionDir)};
        
        Vector reflectedColor = traceRay(mirror, scene, sphereCount, depth - 1);
        
        return addVector(scalarMultipl(localColor, 0.4), scalarMultipl(reflectedColor, 0.6));
    }

    return localColor;
}

void *renderTile(void *arg) {
    ThreadData *data = (ThreadData*)arg;
    for (int y = data->start_y; y < data->end_y; y++) {
        for (int x = 0; x < WIDTH; x++) {
            // Generate camera ray for each tile
            double u = data->map->u[x];
            double v = data->map->v[y];
            Vector d = initVector(u, v, -1.0);
            d = rotateX(d, data->pitch);
            d = rotateY(d, data->yaw);
            d = normaliseVector(d);
            Ray ray = {data->cameraPos, d};
            
            // Trace ray with max depth 3
            Vector color = traceRay(ray, data->scene, data->sphereCount, 3);
            
            // Convert [0,1] to [0,255] with clamping
            double R = color.x * 255.0;
            double G = color.y * 255.0;
            double B = color.z * 255.0;
            
            R = R > 255.0 ? 255.0 : R;
            G = G > 255.0 ? 255.0 : G;
            B = B > 255.0 ? 255.0 : B;

            // Pack into framebuffer (ARGB format)
            data->img[y * WIDTH + x] = (255 << 24) | ((int)R << 16) | ((int)G << 8) | (int)B;
        }
    }
    return NULL;
}

void drawScene(Sphere scene[], int sphereCount, Coord *map, uint32_t img[],
               Vector cameraPos, double yaw, double pitch) {
    pthread_t threads[NUM_THREADS];
    ThreadData t_data[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; i++) {
        t_data[i].start_y = HEIGHT / NUM_THREADS * i;
        t_data[i].end_y = HEIGHT / NUM_THREADS * (i + 1);
        t_data[i].img = img;
        t_data[i].map = map;
        t_data[i].scene = scene;
        t_data[i].sphereCount = sphereCount;
        t_data[i].cameraPos = cameraPos;
        t_data[i].yaw = yaw;
        t_data[i].pitch = pitch;
        pthread_create(&threads[i], NULL, renderTile, &t_data[i]);
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], 0);
    }
}

int main(void) {
    if (SDL_Init(SDL_INIT_VIDEO)) {
        return 1;
    }
    SDL_Window *w = SDL_CreateWindow("Raytracer", SDL_WINDOWPOS_CENTERED, 
                                     SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
    if (!w) {
        return 1;
    }
    SDL_Renderer *r = SDL_CreateRenderer(w, -1, 0);
    if (!r) {
        return 1;
    }
    SDL_Texture *t = SDL_CreateTexture(r, SDL_PIXELFORMAT_ARGB8888, 
                                       SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);
    if (!t) {
        return 1;
    }

    uint32_t *img = malloc(WIDTH * HEIGHT * sizeof(uint32_t));
    Coord map;
    int running = 1;
    SDL_Event event;
    Vector cameraPos = initVector(0.0, 0.0, 0.0);
    double yaw = 0.0, pitch = 0.0;
    SDL_SetRelativeMouseMode(SDL_TRUE);

    // Scene setup: 3 spheres
    Sphere scene[3];
    scene[0] = (Sphere){initVector(0, 0, -2.0), 0.5, initVector(1.0, 0.0, 0.0), 0};
    scene[1] = (Sphere){initVector(1.0, 0, -3.0), 0.5, initVector(0.0, 1.0, 0.0), 1};
    scene[2] = (Sphere){initVector(0, -100.5, -2.0), 100.0, initVector(0.5, 0.5, 0.5), 0};
    for (int i = 0; i < WIDTH; i++) {
        map.u[i] = ((2.0 * i / WIDTH) - 1.0) * ((double)WIDTH / HEIGHT);
    }
    for (int i = 0; i < HEIGHT; i++) {
        map.v[i] = 1.0 - 2.0 * i / HEIGHT;
    }

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
                break;
            }
            if (event.type == SDL_MOUSEMOTION) {
                yaw += event.motion.xrel * 0.005;
                pitch -= event.motion.yrel * 0.005;
                if (pitch > 1.5) {
                    pitch = 1.5;
                }
                if (pitch < -1.5) {
                    pitch = -1.5;
                }
            }
            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        running = 0;
                        break;
                    case SDLK_w:
                        cameraPos.z -= 0.2;
                        break;
                    case SDLK_s:
                        cameraPos.z += 0.2;
                        break;
                    case SDLK_a:
                        cameraPos.x -= 0.2;
                        break;
                    case SDLK_d:
                        cameraPos.x += 0.2;
                        break;
                    case SDLK_q:
                        cameraPos.y -=0.2;
                        break;
                    case SDLK_e:
                        cameraPos.y += 0.2;
                        break;                    
                }
            }
        }

        drawScene(scene, 3, &map, img, cameraPos, yaw, pitch);

        SDL_UpdateTexture(t, NULL, img, WIDTH * sizeof(uint32_t));
        SDL_RenderClear(r);
        SDL_RenderCopy(r, t, NULL, NULL);
        SDL_RenderPresent(r);
    }
    free(img);
    SDL_DestroyTexture(t);
    SDL_DestroyRenderer(r);
    SDL_DestroyWindow(w);
    SDL_Quit();
}
